// -*-C++-*-
// runtime.h
//
// RSL2 runtime control and optimization
//
// $Id: runtime.h,v 1.1 1998/11/18 00:01:26 toddm Exp $
// *******************
// * System Includes *
// *******************
#include <iostream.h>

#include <rw/cstring.h>
#include <rw/tpslist.h>
#include <rw/tphdict.h>
#include <rw/tvhdict.h>

// ******************
// * Local Includes *
// ******************
#include "Resource.h"
#include "res_class.h"
#include "rslMethod.h"
#include "drwcstring.h"
#include "SharedLibrary.h"
#include "lexer_context.h"
#include "DirContents.h"
#include "StringPair.h"

#ifndef _RSL_RUNTIME_H_
#define _RSL_RUNTIME_H_

class rslPackage {
public:
    RWCString name;
    enum load_t { notLoaded=0, Loaded, notFound };
    load_t loaded;

    rslPackage() { loaded = notLoaded; }
    rslPackage(RWCString nm) { name = nm; loaded = notLoaded; }
    
    static unsigned int hash(const rslPackage& p);
    
    inline int operator==(const rslPackage& p)
    {
        return (p.name == name);
    }
    
    inline void operator=(const rslPackage& p)
    {
        name = p.name;
        loaded = p.loaded;
    }
};


// ******************************
// *   RSL Runtime storage
// ******************************
class runtimeRSL {
private:
    // **********************
    // * Session statistics *
    // **********************
	int totalSessions, currentSessions, maxConcurrentSessions;
	int sessionsDoneThisPeriod;

    RWTPtrSlist<param_presetIR> preEvalParams;
    RWTPtrSlist<event> startupECI;

public:
    RWTPtrSlist<rslMethod> RTF;

    ResContext *SysGlobals;
    ResContext *Sessions;
    ResContext *DeadSessions;

    RWTValSlist<rslPackage> Import;
    RWTValHashDictionary<RWCString, SharedLibrary> NameToLibrary;

    elist globalEv, sessionEv;

    int dynamicLibs;

    runtimeRSL(void);
    virtual ~runtimeRSL();
    
    // ***********
    // * Startup *
    // ***********
    int rslStartup(int argc, char **argv);
    void StartWith(DRWCString sw, RWTValSlist<StringPair> ServerParams);
    int ParseFile(const char *fname, lexer_context &lexc);
    int ParseFilesInDirectory(DirContents& dirc);
    
    void CreateUserGlobals(void);
    inline void AddMethod(rslMethod *m) { RTF.insert(m); }
    void AddImport(RWCString imp);
    void AddGlobalEvent(event *e);
    void AddSessionEvent(event *e);
    inline void AddPreEvalParam(param_presetIR *pir) { preEvalParams.insert(pir); }
    void executeStartupECI(void);

    // ***********************
    // * Libraries & Linking *
    // ***********************
    int ImportAndLink(void);
    int LinkMethods(void);
    int EvaluateIRParams(void); 
    
    int ImportPackages(void);
    int LoadPackage(rslPackage& rpack, DirList& classpath);
    int LoadLibrary(rslPackage& rpack, DirContents& dirc);

    // ***********
    // * Runtime *
    // ***********

    // Resource Specific
    ResReference FindResource(RWCString strSession, RWCString resName);
    res_class *FindClass(RWCString strClass);
    ResStructure *CreateResource(RWCString strRType, RWCString strRName="", 
                                 ResList *rlConstructArgs=NULL, ResContext *rcConstructContext=NULL);

    // Session Specific
    ResReference AddNewSession(RWCString strSession, RWCString strSessionObj);
//	ResContext *AddNewSession(RWCString strSession, RWCString strSessionObj);
    ResContext *FindSession(RWCString strSession);
	ResReference FindSessionRef(RWCString strSession);
    int KillSession(RWCString strSession);

	// Session Outgoing Event Queue Specific
	void AddNewDeadSession(RWCString strSession, ResReference refOutQ);
	ResReference GetSessionOutQRef(RWCString strSession);
	elist *GetSessionOutQ(RWCString strSession);
	ResReference GetDeadSessionOutQRef(RWCString strSession);
	elist *GetDeadSessionOutQ(RWCString strSession);
	void KillDeadSession(RWCString strSession);

	// Session Statistics and Counters
    int TotalSessions() { return (totalSessions); }
    int TotalUserSessions() { return (totalSessions-1); }
    int CurrentSessions() { return (currentSessions); }
    int CurrentUserSessions() { return (currentSessions-1); }
    int MaxConcurrentSessions() { return (maxConcurrentSessions); }
    int MaxConcurrentUserSessions() { return (maxConcurrentSessions-1); }
    int SessionsDoneThisPeriod() 
    {   
        int i = sessionsDoneThisPeriod; 
        sessionsDoneThisPeriod = 0;
        return (i); 
    }
    int GetSessionsDoneThisPeriod() {return (sessionsDoneThisPeriod); }

    // Print methods
    void htmlClasses(ostream& out=cout);

    void print(ostream& out=cout);
    void printClasses(ostream& out=cout);
    void printMethods(ostream& out=cout);
    void printGlobals(ostream& out=cout)
        { if (SysGlobals) SysGlobals->print(out); }
    void printContexts(ostream& out=cout)
        { if (Sessions) Sessions->print(out); }
    void printStats(ostream& out=cout);

    void eciStats(ostream& out=cout);
};

extern runtimeRSL runtimeStuff;

#endif





