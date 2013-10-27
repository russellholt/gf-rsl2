// *******************************************************************************
// *
// * Module Name: HttpSession.cc
// * 
// * Description: 
// *
// * History: Create - CKING 8/07/98
// * 
// * 
// * Copyright 1998 by Destiny Software Corporation.
// * 
// *******************************************************************************
#include "HttpSession.h"



extern CRITICAL crDestiny;
httpSessionHashList HttpSessionList;



unsigned int httpSession::hash (void)
{
        // Uses the class name attribute to calculate a unique hash value.
    return ( name.hash() );
}


httpSession::httpSession (void)
{
    processing = notProcessing;
    fdWebHarness = SYS_NET_INVALID_FD;
    session = NULL;
    pNextSession = NULL;
    pPrevSession = NULL;
    time (&timeout);   
    isn = HttpSessionList.entries();
}


httpSession::httpSession (httpSession_t_ptr sn, SYS_NETFD fdHarness)
{
    Init (sn, fdHarness);
}


httpSession::httpSession (httpSession_t_ptr sn)
{
    Init (sn, SYS_NET_INVALID_FD);
}


void httpSession::Init (httpSession_t_ptr sn, SYS_NETFD fdHarness)
{
    char snName[25];


    isn = HttpSessionList.entries();
    session = sn;
    sprintf (snName, "%p", sn);
    name = snName;
    processing = notProcessing;
    fdWebHarness = fdHarness;
    pNextSession = NULL;
    pPrevSession = NULL;
    time (&timeout);   
}

int httpSession::isAlive (void)
{
    if ( (alive) ||
         (processing == httpSession::Processing) )
       return(TRUE);
       
    return(FALSE);
}


void httpSession::lock (void)
{
    alive = TRUE;
    processing  = httpSession::Processing;
}


void httpSession::unlock (void)
{
    processing  = httpSession::notProcessing;
}


int httpSession::isTimeout (void)
{
    double   timediff = 0;
    time_t curTime;
    
    
    if(isAlive())
    {
           // Reset the alive state of the session, in order to determine if the session
           // is still alive at a later time period. 
       alive = FALSE;
       time(&timeout);
#ifdef DEBUGHASH       
       MyDebugLog(systhread_current(), "reset alive flag\n");
#endif       
    }
    else
    {
#ifdef DEBUGHASH       
        MyDebugLog(systhread_current(), "has session timed-out?\n");
#endif        
            // Session is inactive, check if a timeout has occurred on the session.
        time(&curTime); 
        timediff = difftime(curTime, timeout);
        if (MAX_SESSION_TIMEOUT < timediff)
            return (TRUE); 
    }        
   
    return (FALSE);      
}


httpSessionHashList::httpSessionHashList (void)
{
    sessionTotal = 0;
    time (&timeout);
    memset ((void *)&HttpSessionHashTable[0], 0, sizeof(HttpSessionHashTable));
}


void httpSessionHashList::setTimeoutManagementTime (void)
{
    time (&timeout);
}    


int httpSessionHashList::isTimeoutManagement (void)
{
    double   timediff = 0;
    time_t curTime;
    
    
        // Determine if it is time to perform session inactivity 
        // timeout management.
    time(&curTime); 
    timediff = difftime(curTime, timeout);
    if (MAX_TIMEOUT_MANAGEMENT_CHECK_TIME < timediff)
        return (TRUE); 
   
    return (FALSE);      
}


int httpSessionHashList::hashTableIndex (httpSession *sn)
{
    unsigned int index;
    
    
    
       // Determine which link list of the http session hash table to
       // perform the search upon.   
    index = sn->hash() % MAX_HASH_TABLE_SIZE;
    
    if ( (index < 0) || (index > MAX_HASH_TABLE_SIZE) )
        index = 0;
       
    return (index);   
}
       

void httpSessionHashList::insert (httpSession *sn)
{
   int index;
   

   if (sn == NULL)
       return;

   crit_enter(crDestiny);
   
       // Determine which link list of the http session hash table to
       // perform the insertion upon.   
   index = hashTableIndex (sn);

       // Initialize link list pointer variables of session, and insert
       // the session into the appropriate link list of hash table.
   sn->pPrevSession = NULL;
   sn->pNextSession = NULL;

   if (HttpSessionHashTable[index].pSessionListHead == NULL)
      HttpSessionHashTable[index].pSessionListHead = sn;
   else
   {
      HttpSessionHashTable[index].pSessionListTail->pNextSession = sn;
      sn->pPrevSession = HttpSessionHashTable[index].pSessionListTail;
   }

   HttpSessionHashTable[index].pSessionListTail = sn;
   HttpSessionHashTable[index].sessionListTotal++;	// increment session link list counter
   
   sessionTotal++;					// increment session hash table counter

#ifdef DEBUGHASH       
   sprintf (logbuf, "[TD:%p SN:%p ISN:%d NM:%s HIDX:%d LTOT:%d FD:%p TM:%d] session inserted into table\n",
            systhread_current(), sn->session, sn->getISN(), (sn->getName()).data(), index,
            HttpSessionHashTable[index].sessionListTotal, sn->fdWebHarness, sn->getTimeout());
   MyDebugLog(logbuf);
#endif          

   crit_exit(crDestiny);
}


void httpSessionHashList::removeHashTableEntry (int index, httpSession *sn)
{
   
   httpSession *pNxtSn;
   httpSession *pPrvSn;



   if (sn == NULL)
       return;

       // Disconnect from the Web Harness
   if (sn->fdWebHarness >= 0)
   { 
      net_close (sn->fdWebHarness);
      sn->fdWebHarness = SYS_NET_INVALID_FD;
   }
   
   if ( (sn->pPrevSession == NULL) && (sn->pNextSession == NULL) )
   {
         // only session in the link list
      HttpSessionHashTable[index].pSessionListHead = NULL;
      HttpSessionHashTable[index].pSessionListTail = NULL;
   }
   else if (sn->pPrevSession == NULL)
   {
         // 1st session in the link list 
      HttpSessionHashTable[index].pSessionListHead = sn->pNextSession;
      HttpSessionHashTable[index].pSessionListHead->pPrevSession = NULL;
   }
   else if (sn->pNextSession == NULL)
   {
         // last session in the link list
      HttpSessionHashTable[index].pSessionListTail = sn->pPrevSession;
      HttpSessionHashTable[index].pSessionListTail->pNextSession = NULL;
   }
   else
   {
         // nth session in the link list
      pNxtSn = sn->pNextSession;
      pPrvSn = sn->pPrevSession;
      pNxtSn->pPrevSession = sn->pPrevSession;
      pPrvSn->pNextSession = sn->pNextSession;
   }
         
   HttpSessionHashTable[index].sessionListTotal--;	// decrement session link list counter
   
   sessionTotal--;					// decrement session hash table counter
   
   
}


void httpSessionHashList::remove (httpSession *sn)
{
   
   int index;
   httpSession *pNxtSn;
   httpSession *pPrvSn;



   if (sn == NULL)
       return;

       // Determine which link list of the http session hash table to
       // perform the insertion upon.   
   index = hashTableIndex (sn);

       // Disconnect from the Web Harness
   if (sn->fdWebHarness >= 0)
   { 
      net_close (sn->fdWebHarness);
      sn->fdWebHarness = SYS_NET_INVALID_FD;
   }

   if ( (sn->pPrevSession == NULL) && (sn->pNextSession == NULL) )
   {
         // only session in the link list
      HttpSessionHashTable[index].pSessionListHead = NULL;
      HttpSessionHashTable[index].pSessionListTail = NULL;
   }
   else if (sn->pPrevSession == NULL)
   {
         // 1st session in the link list 
      HttpSessionHashTable[index].pSessionListHead = sn->pNextSession;
      HttpSessionHashTable[index].pSessionListHead->pPrevSession = NULL;
   }
   else if (sn->pNextSession == NULL)
   {
         // last session in the link list
      HttpSessionHashTable[index].pSessionListTail = sn->pPrevSession;
      HttpSessionHashTable[index].pSessionListTail->pNextSession = NULL;
   }
   else
   {
         // nth session in the link list
      pNxtSn = sn->pNextSession;
      pPrvSn = sn->pPrevSession;
      pNxtSn->pPrevSession = sn->pPrevSession;
      pPrvSn->pNextSession = sn->pNextSession;
   }
         
   HttpSessionHashTable[index].sessionListTotal--;	// decrement session link list counter
   
   sessionTotal--;					// decrement session hash table counter
   
   
}


httpSession * httpSessionHashList::find (const httpSession* sn)
{
   int index;
   httpSession *pSession, *pTmpSession;



    MyDebugLog(systhread_current(), "In find\n");
    
    crit_enter(crDestiny);

       // Determine which link list of the http session hash table to
       // perform the insertion upon.   
   index = hashTableIndex ((httpSession *)sn);

   pSession = HttpSessionHashTable[index].pSessionListHead;
   while (pSession != NULL)
   {
      pTmpSession = pSession->pNextSession;
#ifdef DEBUGHASH       
      sprintf (logbuf, "[TD:%p SN:%p ISN:%d NM:%s HIDX:%d LTOT:%d FD:%p TM:%d] SN:%p equal table entry?\n",
               systhread_current(), pSession->session,  pSession->getISN(), (pSession->getName()).data(), index, 
               HttpSessionHashTable[index].sessionListTotal, pSession->fdWebHarness, pSession->getTimeout(),
               sn->session);
      MyDebugLog(logbuf);
#endif
      if (pSession->session == sn->session)
      {
#ifdef DEBUGSESS
          sprintf (logbuf, "[TD:%p SN:%p ISN:%d NM:%s HIDX:%d LTOT:%d FD:%p TM:%d] found session table entry\n",
                   systhread_current(), pSession->session, pSession->getISN(), (pSession->getName()).data(), index,
                   HttpSessionHashTable[index].sessionListTotal, pSession->fdWebHarness, pSession->getTimeout());
          MyDebugLog(logbuf);
#endif          
          crit_exit(crDestiny);
          return (pSession);
      }
      pSession = pTmpSession;
   }
   
   crit_exit(crDestiny);
   return (NULL);
   
}


void httpSessionHashList::clearAndDestroy (void)
{
    int i;
    httpSession *pSession, *pTmpSession;


#ifdef DEBUGSESS
          MyDebugLog(systhread_current(), "In clearAndDestroy\n");
#endif          

    crit_enter(crDestiny);

    for (i=0; i < MAX_HASH_TABLE_SIZE; i++)
    { 
        while ( (pSession = HttpSessionHashTable[i].pSessionListHead) != NULL)
        {
            pTmpSession = pSession->pNextSession;
#ifdef DEBUGSESS
          sprintf (logbuf, "[TD:%p SN:%p ISN:%d NM:%s HIDX:%d LTOT:%d FD:%p] deleting session\n",
                   systhread_current(), pSession->session, pSession->getISN(), (pSession->getName()).data(), i,
                   HttpSessionHashTable[i].sessionListTotal, pSession->fdWebHarness);
          MyDebugLog(logbuf);
#endif          

                // Unlink session from list, and delete the session object.
            removeHashTableEntry (i, pSession);
            delete (pSession);
               // Point to next session in the list
            pSession = pTmpSession;
        }
    }
  
    time (&timeout);
    sessionTotal = 0;
    memset ((void *)&HttpSessionHashTable[0], 0, sizeof(HttpSessionHashTable));

    crit_exit(crDestiny);
}

void httpSessionHashList::dumpHashTableList (int index)
{
    int entryIndex = 0;
    httpSession *pSession, *pTmpSession;



    pSession = HttpSessionHashTable[index].pSessionListHead;
    while (pSession != NULL)
    {
        entryIndex++;
        pTmpSession = pSession->pNextSession;
#ifdef DEBUGHASH       
        sprintf (logbuf, "[TD:%p SN:%p ISN:%d NM:%s HIDX:%d LTOT:%d FD:%p TM:%d] dumping link list entry %d\n",
                 systhread_current(), pSession->session, pSession->getISN(), (pSession->getName()).data(), index,
                 HttpSessionHashTable[index].sessionListTotal, pSession->fdWebHarness, pSession->getTimeout(), entryIndex);
        MyDebugLog(logbuf);
#endif          

        pSession = pTmpSession;
    }
}


void httpSessionHashList::dumpHashTable (void)
{
    crit_enter(crDestiny);
    for (int i=0; i < MAX_HASH_TABLE_SIZE; i++)
        dumpHashTableList (i);
    crit_exit(crDestiny);
}

    
void httpSessionHashList::removeInactiveSessions (void)
{
    int entryIndex = 0;
    httpSession *pSession, *pTmpSession;



    if (sessionTotal <= 0)		// hash table empty
       return;
       
    for (int index=0; index < MAX_HASH_TABLE_SIZE; index++)
    {
        pSession = HttpSessionHashTable[index].pSessionListHead;
        while (pSession != NULL)
        {
            entryIndex++;
            pTmpSession = pSession->pNextSession;
#ifdef DEBUGHASH       
            sprintf (logbuf, "[TD:%p SN:%p ISN:%d NM:%s HIDX:%d LTOT:%d FD:%p TM:%d] checking timeout of link list entry %d\n",
                     systhread_current(), pSession->session, pSession->getISN(), (pSession->getName()).data(), index,
                     HttpSessionHashTable[index].sessionListTotal, pSession->fdWebHarness, pSession->getTimeout(), entryIndex);
            MyDebugLog(logbuf);
#endif
            if (pSession->isTimeout())
            {
#ifdef DEBUGSESS
                sprintf (logbuf, "[TD:%p SN:%p ISN:%d NM:%s HIDX:%d LTOT:%d FD:%p TM:%d] timeout occurred on link list entry %d\n",
                        systhread_current(), pSession->session, pSession->getISN(), (pSession->getName()).data(), index,
                        HttpSessionHashTable[index].sessionListTotal, pSession->fdWebHarness, pSession->getTimeout(), entryIndex);
                MyDebugLog(logbuf);
#endif
                    // Unlink session from list, and delete the session object.
                removeHashTableEntry (index, pSession);
                delete (pSession);
            }          

               // Point to next session in the list
            pSession = pTmpSession;
        }
    }
}
