// *****************************************************************************
// *
// * HTTP Session Class
// *
// * History: Create - CKING 7/29/98
// * 
// * 
// * Copyright 1998 by Destiny Software Corporation.
// * 
// *******************************************************************************
#ifndef _HTTP_SESSION_H_
#define _HTTP_SESSION_H_

// *******************
// * System Includes *
// *******************
#include <thread.h>
#include <sys/types.h>
#include <time.h>

// ******************
// * Local Includes *
// ******************
#include <rw/cstring.h>
#include <rw/tphset.h>

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
#include "base/util.h"       /* is_mozilla, getline */
#include "frame/protocol.h"  /* protocol_start_response */
#include "base/file.h"       /* system_fopenRO */
#include "base/buffer.h"     /* filebuf */
#include "frame/log.h"       /* log_error */
}


#define MAX_THREADS				10
#define SYS_NET_INVALID_FD			SYS_NET_ERRORFD
#define MAX_HASH_TABLE_SIZE			1000
#define MAX_SESSION_TIMEOUT			900	// 15 minutes
#define MAX_TIMEOUT_MANAGEMENT_CHECK_TIME	300	// 5 minutes

typedef SYS_THREAD		httpSession_t_ptr;

struct threadControlBlock
{
   SYS_THREAD threadID;
   SYS_NETFD fdWebHarness;
   int processing;
   
   pblock *pb;
   Session *sn;
   Request *rq;
};


class httpSession;
struct httpSessionHashListEntry
{
    long sessionListTotal;
    httpSession * pSessionListHead;
    httpSession * pSessionListTail;
};


class httpSession {
private:
        // types
    enum process_t { notProcessing=0, Processing};

        // variables
    int alive;
    time_t timeout;
    process_t processing;

public:
        // variables
    int isn;
    RWCString name;
    httpSession_t_ptr session;
    httpSession * pNextSession;
    httpSession * pPrevSession;
    SYS_NETFD fdWebHarness;

        // methods
    httpSession ();
    httpSession (httpSession_t_ptr sn, SYS_NETFD fdHarness);
    httpSession (httpSession_t_ptr sn);

    void Init (httpSession_t_ptr sn, SYS_NETFD fdHarness);

    unsigned int hash (void);
    
    inline RWCString getName (void) const { return name; }
    inline int getISN (void) const { return isn; }
    inline time_t getTimeout(void) const {return timeout; }
    int isAlive (void);
    void lock (void);
    void unlock (void);
    int isLocked (void) const { return (processing  == httpSession::Processing); }
    int isTimeout (void);
};


class httpSessionHashList {
private:
        // variables
    char logbuf[256];
    httpSessionHashListEntry HttpSessionHashTable[MAX_HASH_TABLE_SIZE];

    int hashTableIndex (httpSession *sn);
    void removeHashTableEntry (int index, httpSession *sn);
    time_t timeout;
    
public:
        // variables
    int sessionTotal; 

        // methods
    void insert (httpSession *sn);
    httpSession * find (const httpSession* sn);
    void clearAndDestroy (void);
    void remove (httpSession *sn);
    inline int entries(void) const { return sessionTotal; }
    void dumpHashTable (void);
    void dumpHashTableList (int index);
    void removeInactiveSessions (void);
    int isTimeoutManagement (void);
    void setTimeoutManagementTime (void);
    httpSessionHashList();
};


extern void MyDebugLog(char *pstrText);
extern void MyDebugLog (SYS_THREAD threadID, char *pstrMessage);
extern httpSessionHashList HttpSessionList;

#endif
