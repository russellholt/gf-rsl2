// *******************************************************************************
// *
// * Module Name: CryptoSession.cc
// * 
// * Description: 
// *
// * History: Create - CKING 10/28/98
// * 
// * 
// * Copyright 1998 by Destiny Software Corporation.
// * 
// *******************************************************************************
#include <stdlib.h>
#include <stdio.h>
#include "CryptoSession.h"



//extern CRITICAL crDestiny;
sessionCrtlBlkHashList SessionCrtlBlkList;



unsigned int sessionCrtlBlk::hash (void)
{
        // Uses the class name attribute to calculate a unique hash value.
    return ( name.hash() );
}


sessionCrtlBlk::sessionCrtlBlk (void)
{
    processing = notProcessing;
    session = NULL;
    pNextSession = NULL;
    pPrevSession = NULL;
    time (&timeout);   
    isn = SessionCrtlBlkList.entries();
}


sessionCrtlBlk::sessionCrtlBlk (sessionCrtlBlk_t_ptr sn)
{
    Init (sn);
}


void sessionCrtlBlk::Init (sessionCrtlBlk_t_ptr sn)
{
    char snName[25];


    isn = SessionCrtlBlkList.entries();
    session = sn;
    sprintf (snName, "%p", sn);
    name = snName;
    processing = notProcessing;
    pNextSession = NULL;
    pPrevSession = NULL;
    time (&timeout);   
}

int sessionCrtlBlk::isAlive (void)
{
    if ( (alive) ||
         (processing == sessionCrtlBlk::Processing) )
       return(TRUE);
       
    return(FALSE);
}


void sessionCrtlBlk::lock (void)
{
    alive = TRUE;
    processing  = sessionCrtlBlk::Processing;
}


void sessionCrtlBlk::unlock (void)
{
    processing  = sessionCrtlBlk::notProcessing;
}


int sessionCrtlBlk::isTimeout (void)
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
       CryptoLog("reset alive flag\n");
#endif       
    }
    else
    {
#ifdef DEBUGHASH       
        CryptoLog("has session timed-out?\n");
#endif        
            // Session is inactive, check if a timeout has occurred on the session.
        time(&curTime); 
        timediff = difftime(curTime, timeout);
        if (MAX_SESSION_TIMEOUT < timediff)
            return (TRUE); 
    }        
   
    return (FALSE);      
}


sessionCrtlBlkHashList::sessionCrtlBlkHashList (void)
{
    sessionTotal = 0;
    time (&timeout);
    memset ((void *)&SessionCrtlBlkHashTable[0], 0, sizeof(SessionCrtlBlkHashTable));
}


void sessionCrtlBlkHashList::setTimeoutManagementTime (void)
{
    time (&timeout);
}    


int sessionCrtlBlkHashList::isTimeoutManagement (void)
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


int sessionCrtlBlkHashList::hashTableIndex (sessionCrtlBlk *sn)
{
    unsigned int index;
    
    
    
       // Determine which link list of the  session hash table to
       // perform the search upon.   
    index = sn->hash() % MAX_HASH_TABLE_SIZE;
    
    if ( (index < 0) || (index > MAX_HASH_TABLE_SIZE) )
        index = 0;
       
    return (index);   
}
       

void sessionCrtlBlkHashList::insert (sessionCrtlBlk *sn)
{
   int index;
   

   if (sn == NULL)
       return;

// crit_enter(crDestiny);
   
       // Determine which link list of the  session hash table to
       // perform the insertion upon.   
   index = hashTableIndex (sn);

       // Initialize link list pointer variables of session, and insert
       // the session into the appropriate link list of hash table.
   sn->pPrevSession = NULL;
   sn->pNextSession = NULL;

   if (SessionCrtlBlkHashTable[index].pSessionListHead == NULL)
      SessionCrtlBlkHashTable[index].pSessionListHead = sn;
   else
   {
      SessionCrtlBlkHashTable[index].pSessionListTail->pNextSession = sn;
      sn->pPrevSession = SessionCrtlBlkHashTable[index].pSessionListTail;
   }

   SessionCrtlBlkHashTable[index].pSessionListTail = sn;
   SessionCrtlBlkHashTable[index].sessionListTotal++;	// increment session link list counter
   
   sessionTotal++;					// increment session hash table counter

#ifdef DEBUGHASH       
   sprintf (logbuf, "[SN:%p ISN:%d NM:%s HIDX:%d LTOT:%d TM:%d] session inserted into table\n",
            sn->session, sn->getISN(), (sn->getName()).data(), index,
            SessionCrtlBlkHashTable[index].sessionListTotal, sn->getTimeout());
   CryptoLog(logbuf);
#endif          

// crit_exit(crDestiny);
}


void sessionCrtlBlkHashList::removeHashTableEntry (int index, sessionCrtlBlk *sn)
{
   
   sessionCrtlBlk *pNxtSn;
   sessionCrtlBlk *pPrvSn;



   if (sn == NULL)
       return;

   if ( (sn->pPrevSession == NULL) && (sn->pNextSession == NULL) )
   {
         // only session in the link list
      SessionCrtlBlkHashTable[index].pSessionListHead = NULL;
      SessionCrtlBlkHashTable[index].pSessionListTail = NULL;
   }
   else if (sn->pPrevSession == NULL)
   {
         // 1st session in the link list 
      SessionCrtlBlkHashTable[index].pSessionListHead = sn->pNextSession;
      SessionCrtlBlkHashTable[index].pSessionListHead->pPrevSession = NULL;
   }
   else if (sn->pNextSession == NULL)
   {
         // last session in the link list
      SessionCrtlBlkHashTable[index].pSessionListTail = sn->pPrevSession;
      SessionCrtlBlkHashTable[index].pSessionListTail->pNextSession = NULL;
   }
   else
   {
         // nth session in the link list
      pNxtSn = sn->pNextSession;
      pPrvSn = sn->pPrevSession;
      pNxtSn->pPrevSession = sn->pPrevSession;
      pPrvSn->pNextSession = sn->pNextSession;
   }
         
   SessionCrtlBlkHashTable[index].sessionListTotal--;	// decrement session link list counter
   
   sessionTotal--;					// decrement session hash table counter
   
   
}


void sessionCrtlBlkHashList::remove (sessionCrtlBlk *sn)
{
   
   int index;
   sessionCrtlBlk *pNxtSn;
   sessionCrtlBlk *pPrvSn;



   if (sn == NULL)
       return;

       // Determine which link list of the  session hash table to
       // perform the insertion upon.   
   index = hashTableIndex (sn);

   if ( (sn->pPrevSession == NULL) && (sn->pNextSession == NULL) )
   {
         // only session in the link list
      SessionCrtlBlkHashTable[index].pSessionListHead = NULL;
      SessionCrtlBlkHashTable[index].pSessionListTail = NULL;
   }
   else if (sn->pPrevSession == NULL)
   {
         // 1st session in the link list 
      SessionCrtlBlkHashTable[index].pSessionListHead = sn->pNextSession;
      SessionCrtlBlkHashTable[index].pSessionListHead->pPrevSession = NULL;
   }
   else if (sn->pNextSession == NULL)
   {
         // last session in the link list
      SessionCrtlBlkHashTable[index].pSessionListTail = sn->pPrevSession;
      SessionCrtlBlkHashTable[index].pSessionListTail->pNextSession = NULL;
   }
   else
   {
         // nth session in the link list
      pNxtSn = sn->pNextSession;
      pPrvSn = sn->pPrevSession;
      pNxtSn->pPrevSession = sn->pPrevSession;
      pPrvSn->pNextSession = sn->pNextSession;
   }
         
   SessionCrtlBlkHashTable[index].sessionListTotal--;	// decrement session link list counter
   
   sessionTotal--;					// decrement session hash table counter
   
   
}


sessionCrtlBlk * sessionCrtlBlkHashList::find (const sessionCrtlBlk* sn)
{
   int index;
   sessionCrtlBlk *pSession, *pTmpSession;



//  CryptoLog("In find\n");
    
//  crit_enter(crDestiny);

       // Determine which link list of the  session hash table to
       // perform the insertion upon.   
   index = hashTableIndex ((sessionCrtlBlk *)sn);

   pSession = SessionCrtlBlkHashTable[index].pSessionListHead;
   while (pSession != NULL)
   {
      pTmpSession = pSession->pNextSession;
#ifdef DEBUGHASH       
      sprintf (logbuf, "[SN:%p ISN:%d NM:%s HIDX:%d LTOT:%d TM:%d] SN:%p equal table entry?\n",
               pSession->session,  pSession->getISN(), (pSession->getName()).data(), index, 
               SessionCrtlBlkHashTable[index].sessionListTotal, pSession->getTimeout(),
               sn->session);
      CryptoLog(logbuf);
#endif
      if (pSession->session == sn->session)
      {
#ifdef DEBUGSESS
          sprintf (logbuf, "[SN:%p ISN:%d NM:%s HIDX:%d LTOT:%d TM:%d] found session table entry\n",
                   pSession->session, pSession->getISN(), (pSession->getName()).data(), index,
                   SessionCrtlBlkHashTable[index].sessionListTotal, pSession->getTimeout());
          CryptoLog(logbuf);
#endif          
//        crit_exit(crDestiny);
          return (pSession);
      }
      pSession = pTmpSession;
   }
   
// crit_exit(crDestiny);
   return (NULL);
   
}


void sessionCrtlBlkHashList::clearAndDestroy (void)
{
    int i;
    sessionCrtlBlk *pSession, *pTmpSession;


#ifdef DEBUGSESS
          CryptoLog("In clearAndDestroy\n");
#endif          

//  crit_enter(crDestiny);

    for (i=0; i < MAX_HASH_TABLE_SIZE; i++)
    { 
        while ( (pSession = SessionCrtlBlkHashTable[i].pSessionListHead) != NULL)
        {
            pTmpSession = pSession->pNextSession;
#ifdef DEBUGSESS
          sprintf (logbuf, "[SN:%p ISN:%d NM:%s HIDX:%d LTOT:%d] deleting session\n",
                   pSession->session, pSession->getISN(), (pSession->getName()).data(), i,
                   SessionCrtlBlkHashTable[i].sessionListTotal);
          CryptoLog(logbuf);
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
    memset ((void *)&SessionCrtlBlkHashTable[0], 0, sizeof(SessionCrtlBlkHashTable));

//  crit_exit(crDestiny);
}

void sessionCrtlBlkHashList::dumpHashTableList (int index)
{
    int entryIndex = 0;
    sessionCrtlBlk *pSession, *pTmpSession;



    pSession = SessionCrtlBlkHashTable[index].pSessionListHead;
    while (pSession != NULL)
    {
        entryIndex++;
        pTmpSession = pSession->pNextSession;
#ifdef DEBUGHASH       
        sprintf (logbuf, "[SN:%p ISN:%d NM:%s HIDX:%d LTOT:%d TM:%d] dumping link list entry %d\n",
                 pSession->session, pSession->getISN(), (pSession->getName()).data(), index,
                 SessionCrtlBlkHashTable[index].sessionListTotal, pSession->getTimeout(), entryIndex);
        CryptoLog(logbuf);
#endif          

        pSession = pTmpSession;
    }
}


void sessionCrtlBlkHashList::dumpHashTable (void)
{
//  crit_enter(crDestiny);
    for (int i=0; i < MAX_HASH_TABLE_SIZE; i++)
        dumpHashTableList (i);
//  crit_exit(crDestiny);
}

    
void sessionCrtlBlkHashList::removeInactiveSessions (void)
{
    int entryIndex = 0;
    sessionCrtlBlk *pSession, *pTmpSession;



    if (sessionTotal <= 0)		// hash table empty
       return;
       
    for (int index=0; index < MAX_HASH_TABLE_SIZE; index++)
    {
        pSession = SessionCrtlBlkHashTable[index].pSessionListHead;
        while (pSession != NULL)
        {
            entryIndex++;
            pTmpSession = pSession->pNextSession;
#ifdef DEBUGHASH       
            sprintf (logbuf, "[SN:%p ISN:%d NM:%s HIDX:%d LTOT:%d TM:%d] checking timeout of link list entry %d\n",
                     pSession->session, pSession->getISN(), (pSession->getName()).data(), index,
                     SessionCrtlBlkHashTable[index].sessionListTotal, pSession->getTimeout(), entryIndex);
            CryptoLog(logbuf);
#endif
            if (pSession->isTimeout())
            {
#ifdef DEBUGSESS
                sprintf (logbuf, "[SN:%p ISN:%d NM:%s HIDX:%d LTOT:%d TM:%d] timeout occurred on link list entry %d\n",
                        pSession->session, pSession->getISN(), (pSession->getName()).data(), index,
                        SessionCrtlBlkHashTable[index].sessionListTotal, pSession->getTimeout(), entryIndex);
                CryptoLog(logbuf);
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
