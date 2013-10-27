// *****************************************************************************
// *
// * Cryptography Management Sub-System Session Class
// *
// * History: Create - CKING 7/29/98
// * 
// * 
// * Copyright 1998 by Destiny Software Corporation.
// * 
// *******************************************************************************
#ifndef _CRYPTO_SESSION_H_
#define _CRYPTO_SESSION_H_

#include <sys/types.h>
#include <time.h>

#include <rw/cstring.h>
#include <rw/tphset.h>

#include "CryptoSubSystem.h"

#define MAX_HASH_TABLE_SIZE			1000
#define MAX_SESSION_TIMEOUT			900	// 15 minutes
#define MAX_TIMEOUT_MANAGEMENT_CHECK_TIME	300	// 5 minutes


typedef CryptoSession *		sessionCrtlBlk_t_ptr;

class sessionCrtlBlk;
struct sessionCrtlBlkHashListEntry
{
    long sessionListTotal;
    sessionCrtlBlk * pSessionListHead;
    sessionCrtlBlk * pSessionListTail;
};


class sessionCrtlBlk {
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
    sessionCrtlBlk_t_ptr session;
    sessionCrtlBlk * pNextSession;
    sessionCrtlBlk * pPrevSession;

        // methods
    sessionCrtlBlk ();
    sessionCrtlBlk (sessionCrtlBlk_t_ptr sn);

    void Init (sessionCrtlBlk_t_ptr sn);

    unsigned int hash (void);
    
    inline RWCString getName (void) const { return name; }
    inline int getISN (void) const { return isn; }
    inline time_t getTimeout(void) const {return timeout; }
    int isAlive (void);
    void lock (void);
    void unlock (void);
    int isLocked (void) const { return (processing  == sessionCrtlBlk::Processing); }
    int isTimeout (void);
};


class sessionCrtlBlkHashList {
private:
        // variables
    time_t timeout;
    char logbuf[256];
    sessionCrtlBlkHashListEntry SessionCrtlBlkHashTable[MAX_HASH_TABLE_SIZE];

        // methods
    int hashTableIndex (sessionCrtlBlk *sn);
    void removeHashTableEntry (int index, sessionCrtlBlk *sn);
    
public:
        // variables
    int sessionTotal; 

        // methods
    void insert (sessionCrtlBlk *sn);
    sessionCrtlBlk * find (const sessionCrtlBlk* sn);
    void clearAndDestroy (void);
    void remove (sessionCrtlBlk *sn);
    inline int entries(void) const { return sessionTotal; }
    void dumpHashTable (void);
    void dumpHashTableList (int index);
    void removeInactiveSessions (void);
    int isTimeoutManagement (void);
    void setTimeoutManagementTime (void);
    sessionCrtlBlkHashList();
};


extern void CryptoLog(char *pstrText);
extern sessionCrtlBlkHashList SessionCrtlBlkList;

#endif
