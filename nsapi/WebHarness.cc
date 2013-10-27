// *******************************************************************************
// *
// * Module Name: WebHarness
// * 
// * Description: Netscape Server API functions to handle requests bound for the 
// *              Destiny ECI Server.
// *
// *              The Server Application Functions in this file are Service class
// *              functions.
// * 
// * History: Create - TGM 3/24/97
// * 
// * $Id: WebHarness.cc,v 1.2 1998/12/21 19:34:14 cking Exp $
// * 
// * $Log: WebHarness.cc,v $
// * Revision 1.2  1998/12/21 19:34:14  cking
// * Patch implements a configurable receive timeout value.
// *
// * Revision 1.1  1998/11/17 23:39:21  toddm
// * Initial revision
// *
// * Revision 1.5  1997/10/22 20:16:53  toddm
// * Add Multi-thread support
// *
// * Revision 1.4  1997/09/17 22:47:03  toddm
// * Fix read problem when reading an html file from framework
// *
// * Revision 1.3  1997/09/10 14:43:28  toddm
// * Add Reconnection Support
// *
// * Revision 1.2  1997/08/15 21:27:38  toddm
// * Add Cookie Support
// *
// * Revision 1.1  1997/06/03 18:30:58  toddm
// * Initial revision
// *
// * 
// * Copyright 1997 by Destiny Software Corporation.
// * 
// *******************************************************************************

// *******************
// * System Includes *
// *******************

// ******************
// * Local Includes *
// ******************
#include "Http.h"


extern "C" 
{
// **************************************************************************
// * The following three are standard headers for SAFs.  They're used to    *
// * get the data structures and prototypes needed to declare and use SAFs. *
// **************************************************************************
#include "base/pblock.h"
#include "base/session.h"
#include "frame/req.h"

// ************************
// * Other NSAPI includes *
// ************************
#include "netsite.h"
#include "base/daemon.h"    // daemon_atrestart
#include "base/util.h"      // is_mozilla, getline
#include "frame/protocol.h" // protocol_start_response
#include "base/file.h"      // system_fopenRO
#include "base/net.h"       // Socket 
#include "base/buffer.h"    // filebuf 
#include "frame/log.h"      // log_error
#include "base/crit.h"      // Critical Section

// *****************
// * Socket Stuff  *
// *****************
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
}

// ***********
// * Defines *
// ***********
#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

// ********************************************************
// * File descriptors to be shared between the processes  *
// ********************************************************
static SYS_FILE fdDebugLog = SYS_ERROR_FD;

char *pstrHost;
int iPort;
int iReceiveTimeOut;

CRITICAL crDestiny;
    
#define MAX_LOG_BUFF_LEN	100


// ***************************************************************************
// *
// * FUNCTION: MyDebugLogClose
// *
// * DESCRIPTION: Function called at server restart to close log file
// *
// * PARAMETERS: None
// *
// * RETURNS: Void
// *
// ***************************************************************************
void MyDebugLogClose(void *parameter)
{
    system_fclose(fdDebugLog);
    fdDebugLog = SYS_ERROR_FD;
}


// ***************************************************************************
// *
// * FUNCTION: MyLogInit                            
// *                                                
// * DESCRIPTION: Initialize MyLog                  
// *                                                
// * PARAMETERS: "file" - Filename of the log file. 
// *                                                
// * RETURNS: REQ_ABORTED - If no file name was specifed or cannot open file.
// *          REQ_PROCEED - Success
// *                                                
// ***************************************************************************
extern "C" int MyDebugLogInit(pblock *pb, Session *sn, Request *rq)
{
    // **********************************************
    // * Get the file name from the Parameter Block *
    // **********************************************
    char *pstrFileName = pblock_findval("file", pb);

    // ***************************
    // * Do we have a file name? *
    // ***************************
    if(!pstrFileName) 
    {
        pblock_nvinsert("ERROR", "MyDebugLogInit: please supply a file name", pb);
        return REQ_ABORTED;
    }

    // *********************
    // * Open the file     *
    // *********************
    fdDebugLog = system_fopenWA(pstrFileName);
    if(fdDebugLog == SYS_ERROR_FD)
    {
        pblock_nvinsert("ERROR", "MyDebugLogInit: Unable to open file", pb);
        return REQ_ABORTED;
    }

    // *******************************************
    // * Close log file when server is restarted *
    // *******************************************
    daemon_atrestart(MyDebugLogClose, NULL);
    return REQ_PROCEED;
}


// ***************************************************************************
// *
// * FUNCTION: MyDebugLogClose
// *
// * DESCRIPTION: Function called at server restart to close log file
// *
// * PARAMETERS: None
// *
// * RETURNS: Void
// *
// ***************************************************************************
void CloseWebSocket(void *parameter)
{
    // ***********************************
    // * Disconnect from the Web Harness *
    // ***********************************
    HttpSessionList.clearAndDestroy();

    // ******************************************
    // * Terminate the Critical Section Varible *
    // ******************************************
    crit_terminate(crDestiny);
}


// ***************************************************************************
// *
// * FUNCTION: InitHarness
// *                                                
// * DESCRIPTION: Initialize the Web Harness
// *                                                
// * PARAMETERS: 
// *                                                
// * RETURNS: REQ_ABORTED - If R_WebServer can not be created
// *          REQ_PROCEED - Success
// *                                                
// ***************************************************************************
extern "C" int InitHarness(pblock *pb, Session *sn, Request *rq)
{
    // **************************************
    // * Create a Critical Section Variable *
    // **************************************
    crDestiny = crit_init();

    // **************************************************************
    // * Get the Host and Port that the Web Harness is listening on *
    // **************************************************************
    pstrHost = pblock_findval("Host", pb);
    char *pstrPort = pblock_findval("Port", pb);

    // *****************************
    // * Do we have a port number **
    // *****************************
    if (!pstrPort)
    {
        pblock_nvinsert("ERROR", "InitHarness: please supply a port number", pb);
        return REQ_ABORTED;
    }
    iPort = atoi(pstrPort);

    // ***************************************
    // * If no host name was specified then **
    // * lets try the local host            **
    // ***************************************
    if (!pstrHost) 
    {
        pstrHost = util_hostname();
        if (!pstrPort)
        {
            pblock_nvinsert("ERROR", "InitHarness: A host name was not found", pb);
            return REQ_ABORTED;
        }
    }

    // ****************************************
    // * Do we have a receive timeout number **
    // ****************************************
    iReceiveTimeOut = READ_WAIT;
    char *pstrRcvTmOut = pblock_findval ("RcvTmOut", pb);
    if (pstrRcvTmOut)
        iReceiveTimeOut = atoi (pstrRcvTmOut);

    // *******************************************
    // * Close log file when server is restarted *
    // *******************************************
    daemon_atrestart(CloseWebSocket, NULL);
    return REQ_PROCEED;
}


// ***************************************************************************
// *
// * FUNCTION: MyDebugLog
// *                                                
// * DESCRIPTION: Write some stuff to a debug log file
// *                                                
// * PARAMETERS: None
// *                                                
// * RETURNS: REQ_PROCEED                                      
// *                                                
// ***************************************************************************
void MyDebugLog(char *pstrMessage)
{
    int iLen;

    iLen = strlen(pstrMessage);

    /* The atomic version uses locking to prevent interference */
    system_fwrite_atomic(fdDebugLog, pstrMessage, iLen);

    return;
}


// ***************************************************************************
// *
// * FUNCTION: MyDebugLog
// *                                                
// * DESCRIPTION: Write some stuff to a debug log file
// *                                                
// * PARAMETERS: None
// *                                                
// * RETURNS: REQ_PROCEED                                      
// *                                                
// ***************************************************************************
void MyDebugLog (SYS_THREAD threadID, char *pstrMessage)
{
    int iLen;
    char buffer[256];


     
    sprintf (buffer, "[TD:%p] ", threadID);
    iLen = strlen(buffer);
    strncpy (&buffer[iLen], pstrMessage, (sizeof(buffer) - iLen) - 1);
    iLen = strlen(buffer);

    /* The atomic version uses locking to prevent interference */
    system_fwrite_atomic(fdDebugLog, buffer, iLen);

    return;
}


// ***************************************************************************
// *
// * FUNCTION: ConnectToHarness
// *                                                
// * DESCRIPTION: Open a Socket and connect to the WebHarness.
// *                                                
// * PARAMETERS: None
// *                                                
// * RETURNS: REQ_PROCEED                                      
// *                                                
// ***************************************************************************
SYS_NETFD ConnectToHarness (threadControlBlock *pThread)
{
    struct sockaddr_in server;
    struct hostent *hp;
    unsigned long inaddr;
    SYS_NETFD fdWebHarness;
    char logbuf[MAX_LOG_BUFF_LEN];

#ifdef DEBUG
    sprintf (logbuf, "[TD:%p FD:%p] In ConnectToHarness\n", pThread->threadID, pThread->fdWebHarness);
    MyDebugLog (logbuf);
    sprintf (logbuf, "[TD:%p FD:%p] Web Harness Receive Time-out %d\n", pThread->threadID, fdWebHarness, iReceiveTimeOut);
    MyDebugLog (logbuf);
#endif

    fdWebHarness = pThread->fdWebHarness;

    // ************************************************
    // * if there is a valid file descriptor then we **
    // * are already connect to the specified host   **
    // ************************************************
    if (fdWebHarness == SYS_NET_INVALID_FD) 
    {
        bzero((char *) &server, sizeof(server));

        // ***************************************
        // * Fill in the family and port number **
        // ***************************************
        server.sin_family = AF_INET;
        server.sin_port = htons(iPort);         // Here is the port number

        // ******************************************************************
        // * First try to convert the host name as a dotted-decimal number **
        // * ie) is the host specifed as 123.432.23.23                     **
        // ******************************************************************
        if ((inaddr = inet_addr(pstrHost)) != INADDR_NONE) 
        {
            bcopy((char *) &inaddr, (char *)&server.sin_addr, sizeof(inaddr));
        }
        else
        {
#ifdef DEBUG
            MyDebugLog(pThread->threadID, "Calling gethostbyname\n");
#endif
            if ((hp = gethostbyname(pstrHost)) == 0)
            {
                fdWebHarness = SYS_NET_INVALID_FD;
                return(fdWebHarness);
            }

            bcopy((char *)hp->h_addr, (char *)&server.sin_addr, hp->h_length);
        }


        // *********************************************************
        // * Connect socket to the specified host and port number. *
        // *********************************************************
#ifdef DEBUG
        MyDebugLog(pThread->threadID, "Calling socket\n");
#endif
        // **************************************
        // * Create a Sock Stream of type inet **
        // **************************************
        if ((fdWebHarness=net_socket(AF_INET, SOCK_STREAM, 0)) == SYS_NET_ERRORFD)
        {
            return(fdWebHarness);
        }
    	

#ifdef DEBUG
        MyDebugLog(pThread->threadID, "Calling connect\n");
#endif
        // ********************************************************
        // * connect to the specified host on the specified port **
        // ********************************************************
        if (net_connect(fdWebHarness, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
            fdWebHarness = SYS_NET_INVALID_FD;
            return(fdWebHarness);
        }

#ifdef DEBUG
        MyDebugLog(pThread->threadID, "Successful\n");
#endif

    }

#ifdef DEBUG
     sprintf (logbuf, "[TD:%p FD:%p] Exit ConnectToHarness\n", pThread->threadID, fdWebHarness);
     MyDebugLog (logbuf);
#endif

    return(fdWebHarness);
}


// ***************************************************************************
// *
// * FUNCTION: DisconnectFromHarness
// *                                                
// * DESCRIPTION: Disconnects from the WebHarness and Delete the Socket instance
// *                                                
// * PARAMETERS: None
// *                                                
// * RETURNS: REQ_PROCEED                                      
// *                                                
// ***************************************************************************
int DisconnectFromHarness(threadControlBlock *pThread)
{

#ifdef DEBUG
    MyDebugLog(pThread->threadID, "In DisconnectFromHarness\n");
#endif

    // ***********************************
    // * Disconnect from the Web Harness *
    // ***********************************
    if (pThread->fdWebHarness >= 0) 
    {
        net_close(pThread->fdWebHarness);
        pThread->fdWebHarness = SYS_NET_INVALID_FD;
    }

    return REQ_PROCEED;
}


// ***************************************************************************
// *
// * FUNCTION: GetAck
// *                                                
// * DESCRIPTION:   Wait for an "ACK" to be sent back after data has been
// *                sent over the socket.
// *                                                
// * PARAMETERS: None
// *                                                
// * RETURNS: 1 - Successful
// *          0 - Unsuccessful                                   
// *                                                
// ***************************************************************************
int GetAck (threadControlBlock *pThread)
{
    int iBytesRead;

#ifdef DEBUG
    MyDebugLog(pThread->threadID, "In GetAck\n");
#endif

    // ***************************************
    // * Are we connected to the Web Harness *
    // ***************************************
    if (pThread->fdWebHarness >= 0) 
    {
        char *pstrAck = (char *) MALLOC(sizeof(char) * (strlen(ACK) + 
                                                        LEN_TERMINATOR));
        *pstrAck = '\0';

        // *****************************************
        // * Read the socket looking for an "ACK" **
        // *****************************************
        int iTotalBytesToRead = strlen(ACK);
        if ((iBytesRead = net_read(pThread->fdWebHarness, pstrAck, iTotalBytesToRead, iReceiveTimeOut)) < 0)
        {
            DisconnectFromHarness(pThread);
            FREE(pstrAck);
            return(0);
        }

        // **********************
        // * Did we get an ACK **
        // **********************
        pstrAck[iBytesRead] = '\0';
        if (!strcmp(pstrAck, ACK))
        {
            FREE(pstrAck);
            return(1);
        }

        DisconnectFromHarness(pThread);
        FREE(pstrAck);
    }

    return(0);
}


// ***************************************************************************
// *
// * FUNCTION: SendAck
// *                                                
// * DESCRIPTION:   Send an "ACK" back after data has been
// *                received over the socket.
// *                                                
// * PARAMETERS: None
// *                                                
// * RETURNS: 1 - Successful
// *          0 - Unsuccessful                                   
// *                                                
// ***************************************************************************
int SendAck (threadControlBlock *pThread)
{

#ifdef DEBUG
    MyDebugLog(pThread->threadID, "In SendAck\n");
#endif

    // ***************************************
    // * Are we connected to the Web Harness *
    // ***************************************
    if (pThread->fdWebHarness >= 0) 
    {
        if (net_write(pThread->fdWebHarness, ACK, strlen(ACK)) == IO_ERROR)
        {
            return(0);
        }
        else
        {
            return(1);
        }
    }

    return(0);
}


// ******************************************************************************
// *                                                                            *
// * Function: Process_Request                                                  *
// *                                                                            *
// * Description:   Process Request from the Netscape Server.                   *
// *                                                                            *
// ******************************************************************************
int Process_Request (threadControlBlock *thread)
{
    HTTP http(thread);
    int iStat;

#ifdef DEBUG
    MyDebugLog(thread->threadID, "In Process_Request\n");
    MyDebugLog(thread->threadID, "Calling ProcessRequest\n");
#endif

    // ******************
    // * Handle Request *
    // ******************
    if ((iStat = http.ProcessRequest()) != REQ_PROCEED)
    {
        return(iStat);
    }
        
#ifdef DEBUG
    MyDebugLog(thread->threadID, "Calling ProcessResponse\n");
#endif

    // *****************
    // * Send Response *
    // *****************
    if ((iStat = http.ProcessResponse()) != REQ_PROCEED)
    {
        return(iStat);
    }

    return REQ_PROCEED;
}


void SessionTimeoutManagement (void)
{
    char logbuf[MAX_LOG_BUFF_LEN];


        // Determine if this thread will be responsible for removing sessions that have been
        // inactive over the defined timeout period.
    crit_enter(crDestiny);
    if (HttpSessionList.isTimeoutManagement())
    {
#ifdef DEBUG
        sprintf (logbuf, "[TD:%p STOT:%d] before timeout management\n", systhread_current(), HttpSessionList.entries());
        MyDebugLog(logbuf);
#endif        
        HttpSessionList.removeInactiveSessions ();	// remove inactive sessions from hash table
        HttpSessionList.setTimeoutManagementTime();	// set time for checking for inactive sessions

#ifdef DEBUG
        sprintf (logbuf, "[TD:%p STOT:%d] after timeout management\n", systhread_current(), HttpSessionList.entries());
        MyDebugLog(logbuf);
#endif        
    }
    crit_exit(crDestiny);

}


extern "C" int Handle_Request(pblock *pb, Session *sn, Request *rq)
{
    httpSession httpSn (systhread_current());
    httpSession * newHttpSn = NULL;
    httpSession * curHttpSn = NULL;
    threadControlBlock thread;
    char *p;
    char logbuf[MAX_LOG_BUFF_LEN];
    
    
    
#ifdef DEBUG
    sprintf (logbuf, "[TD:%p STOT:%d] In Handle_Request\n", systhread_current(), HttpSessionList.entries());
    MyDebugLog(logbuf);
#endif    
       
        // Initialize the thread structure.
    thread.pb = pb;
    thread.rq = rq;
    thread.sn = sn;
    thread.fdWebHarness = SYS_NET_INVALID_FD;
    thread.threadID = systhread_current();

#ifdef DEBUGHASH
    HttpSessionList.dumpHashTable();
#endif    
        // Try to find HTTP session in the session hash table.
    curHttpSn = HttpSessionList.find (&httpSn); 
    if (curHttpSn)
    {  
         if (curHttpSn->isLocked())
         {
#ifdef DEBUG
               MyDebugLog(thread.threadID, "FATAL ERROR: request submitted for session already in progress.\n");
#endif
               return (REQ_ABORTED);
         }
             // Retrieve socket file handle associated with this session from previous request(s),
             // and begin processing the request.    
         thread.fdWebHarness = curHttpSn->fdWebHarness;
         curHttpSn->lock();
             // Determine if this thread will be responsible for removing sessions that have been
             // inactive over the defined timeout period.
         SessionTimeoutManagement();
#ifdef DEBUG
         MyDebugLog (thread.threadID, "before processing existing session entry\n");
#endif
         Process_Request(&thread);

         curHttpSn->fdWebHarness = thread.fdWebHarness;		// save socket handle incase it changed
         curHttpSn->unlock();
#ifdef DEBUG
         MyDebugLog (thread.threadID, "after processing existing session entry\n");
#endif    
    }
    else
    {
            // Create new HTTP session entry, add entry to the hash table, and begin processing
            // the request.    
        newHttpSn = new httpSession (systhread_current());
        if (newHttpSn == NULL)
        {
#ifdef DEBUG
            MyDebugLog(thread.threadID, "FATAL ERROR: unable to allocate HTTP session.\n");
#endif
            return (REQ_ABORTED);
        }
        newHttpSn->lock();
        
        HttpSessionList.insert(newHttpSn);

            // Determine if this thread will be responsible for removing sessions that have been
            // inactive over the defined timeout period.
        SessionTimeoutManagement();
#ifdef DEBUG
        MyDebugLog (thread.threadID, "before processing new session entry\n");
#endif
        Process_Request(&thread);

        newHttpSn->fdWebHarness = thread.fdWebHarness;
        newHttpSn->unlock();

#ifdef DEBUG
        MyDebugLog (thread.threadID, "after processing new session entry\n");
#endif
    }
    
#ifdef DEBUGHASH
    HttpSessionList.dumpHashTable();
#endif

#ifdef DEBUG
    sprintf (logbuf, "[TD:%p STOT:%d] Exit Handle_Request\n", systhread_current(), HttpSessionList.entries());
    MyDebugLog(logbuf);
#endif    

    return REQ_PROCEED;
}
