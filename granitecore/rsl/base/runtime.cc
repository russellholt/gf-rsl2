// runtime.cc
// $Id: runtime.cc,v 1.6 1999/01/22 20:56:07 toddm Exp $

#include <fstream.h>
#include <stdlib.h>
#include <unistd.h>

// rogue wave stuff
#include <rw/tpslist.h>
#include <rw/tphdict.h>
#include <rw/tvhset.h>
#include <rw/tvhdict.h>
#include <rw/tvslist.h>

// Destiny stuff
#include "runtime.h"
#include "rslMethod.h"
#include "rsldefaults.h"
#include "DirContents.h"
#include "slog.h"

#include "R_System.h"
#include "R_String.h"
#include "R_Integer.h"
#include "R_Boolean.h"
#include "R_List.h"
#include "R_Queue.h"
#include "R_Status.h"
#include "R_Table.h"

#include "destiny.h"

#ifdef PURIFYTEST
#include "/sw/apps/purify/purify.h"
#endif

#include "killevents.h"

static char rcsid[] = "$Id: runtime.cc,v 1.6 1999/01/22 20:56:07 toddm Exp $";

extern int parse_it(lexer_context &lexc);

extern ofstream rslerr;
//static ofstream rslerr("rslerr");

ofstream *statsFile = NULL, *freelistFile = NULL;

// some debugging statistics
//    they should probably be in a class..
int nResContextAdds=0, nFreelistAdds=0, nFreelistRemoves=0,
    nResourcesCreated=0, nResourcesDestroyed=0, nResObjs=0,
    nProgramResources=0, nResRefsCreated=0, nResRefsDestroyed=0,
    eciCommands=0, actualResCreations=0;

extern int eventsCreated, eventsDestroyed, programEvents, nonProgramEvents; // b.cc
// extern RWTPtrSlist<event> ResultsAccumulator;   // b.cc
extern int argsResolved, argEventsDestroyed;    // Request.cc
extern int totalR_Strings;
extern int ifStatements, ifContexts;    // rslEvents.cc

bool Running = FALSE;   // should be a runtimeRSL class variable
bool useFreelist = TRUE;

unsigned int rslPackage::hash(const rslPackage& p)
{
    return p.name.hash();
}

// runtimeRSL::runtimeRSL()
// Initializes the Sesssions and SysGlobals.
// Being bucketed hash tables, they require an initial bucket capacity.
runtimeRSL::runtimeRSL() :
    NameToLibrary(RWCString::hash, BUCKETS_FOR_IMPORT)
{
    SysGlobals = new ResContext("System Globals", BUCKETS_IN_SYSGLOBALS);
    Sessions = new ResContext("Sessions", BUCKETS_FOR_SESSIONS);
    DeadSessions = new ResContext("DeadSessions", BUCKETS_FOR_SESSIONS);
    
    // ************************************
    // * Load shared libraries by default *
    // ************************************
    dynamicLibs = 1;

    // **********************
    // * Session Statistics *
    // **********************
    totalSessions = 0;
    currentSessions = 0;
    maxConcurrentSessions = 0;

    sessionsDoneThisPeriod = 0;
}

runtimeRSL::~runtimeRSL()
{
}

void runtimeRSL::AddGlobalEvent(event *e)
{
    if (e)
        globalEv.add(e);
}

void runtimeRSL::AddSessionEvent(event *e)
{
    if (!e)
        return;
    
    if (sessionEv.entries() == 0)
        sessionEv.add(e);
    else
        logf->error(LOGRSL) << "There can be only one session{ } block." << endline;
}


// CreateUserGlobals()
void runtimeRSL::CreateUserGlobals(void)
{
    globalEv.execute(SysGlobals);
}

void runtimeRSL::print(ostream& out)
{
    printClasses(out);
    printMethods(out);
}

void runtimeRSL::printClasses(ostream& out)
{
RWTPtrHashTableIterator<res_class> iter(ResClasses);

res_class* rcp = NULL;

    while(iter())
    {
        rcp = iter.key();
        if (rcp)
            rcp->print(out);
    }
    out << flush;
}

// print the classes in html format
void runtimeRSL::htmlClasses(ostream& out)  // remove out
{
RWTPtrHashTableIterator<res_class> iter(ResClasses);

res_class* rcp = NULL;

    ofstream ci("class_index.html");

    ci << "<html><head><title>RSL Classes</title></head><body>\n";

RWCString outname;
    while(iter())
    {
        rcp = iter.key();

        if (rcp)
        {
            outname = rcp->name + ".html";
            ofstream ofs(outname.data());

            rcp->html(ofs);

            ofs.close();
            ci << "<a href=" << outname << ">" << rcp->name << "</a><br>\n";
        }
    }

    ci << "</body></html>\n";
    ci.close();
}


void runtimeRSL::printMethods(ostream& out)
{
RWTPtrSlistIterator<rslMethod> iter(RTF);
rslMethod *rm = NULL;

    while(iter())
    {
        rm = iter.key();
        if (rm)
            rm->print(out);
    }
    out << flush;
}

// LinkMethods
//  For each method (implementation), look up res_class and match rslMethod
// with its declaration.
int runtimeRSL::LinkMethods(void)
{
    //RWTPtrSlistIterator<rslMethod> iter(RTF);
    rslMethod *rm=NULL;
    method_decl *impl_mdecl=NULL;
    res_class *mclass=NULL;
    int nerrors=0;
    res_class lookup;

    while ((rm = RTF.get()) != NULL)    // get() removes the first object
    {
        impl_mdecl = rm->proto();
        if (impl_mdecl)
        {
            lookup.name = impl_mdecl->memberOf;

#ifdef RSLERR
            rslerr << "// Found method `";
            impl_mdecl->print(rslerr);
            rslerr << "'. Looking for class `" << lookup.name << "'\n";
#endif

            mclass = ResClasses.find(&lookup);
            if (!mclass)
            {
                Logf.error(LOGRSL) << "Error: no declaration for class \""
                     << impl_mdecl->memberOf << "\"" << endline;
#ifdef RSLERR
                rslerr << "Error: no declaration for class \""
                     << impl_mdecl->memberOf
                     << "\", from method\n\t";
                cerr << "Error: no declaration for class \""
                     << impl_mdecl->memberOf
                     << "\", from method\n\t";
                impl_mdecl->print(rslerr, 1);
                impl_mdecl->print(cerr, 1);
                rslerr << endl;
                cerr << endl;
#endif
                nerrors++;
                continue;
            }
            if (!mclass->LinkImplementation(rm))
            {
//              Logf.error(LOGRSL) "Error: In class \"" << lookup.name
//                   << "\", no declaration for method
#ifdef RSLERR
                rslerr << "Error: In class \"" << lookup.name
                     << "\", no declaration for method\n\t";
                cerr << "Error: In class \"" << lookup.name
                     << "\", no declaration for method\n\t";
                impl_mdecl->print(rslerr);
                impl_mdecl->print(cerr);
                rslerr << endl;
                cerr << endl;
#endif

                nerrors++;
                continue;
            }
        }
    }
    
#ifdef RSLERR
    if (nerrors > 0)
        rslerr << nerrors << " errors found in LinkMethods()\n";
#endif
    
    return nerrors;
}

// EvaluateIRParams
// Formal parameters which match an inline resource by value
// (param_presetIR) must have their expected value
// evaluated *after* all the resource declarations have been
// parsed! Thus we built a list of them and now churn through
// and evaluate each one.
int runtimeRSL::EvaluateIRParams(void)
{
    if (preEvalParams.entries() <= 0)
        return 0;

#ifdef RSLERR
    rslerr << "Evaluating inline resource expected values "
        << "in formal parameters.\n";
#endif

    Logf.info(LOGRSL) << "Evaluating inline resource parameters.." << endline;
        
    RWTPtrSlistIterator<param_presetIR> iter(preEvalParams);
    param_presetIR *pir=NULL;
    int nerrors = 0;
    event *retev=NULL;

    while(iter())
    {
        pir = iter.key();
        if (!pir)
            continue;
        
        if (pir->toEval)
        {
            Logf.debug(LOGRSL) << "Evaluate parameter: `"
                << pir->toEval << "'" << endline;

            retev = pir->toEval->execute(SysGlobals);
            if (retev && retev->isA(event::resArgKind))
            {
                pir->what = ((ResArg *) retev)->ref();
                if (pir->what)
                {
                    // brace for deletion of retev->ref
                    // (maybe param_preset::what should be a ResReference)
                    pir->what->NewReference();

                    // Done with the inline spec. Really.
                    //delete pir->toEval;
// Commented out TGM - 12/21/98
//					Remember(pir->toEval);
                    pir->toEval = NULL;

                    continue;   // this is the desired path.
                }
            }
        }

#ifdef RSLERR
        rslerr << "Error: unable to evaluate inline parameter `";
        pir->print(rslerr);
#endif

        nerrors++;
        continue;
    }
    
#ifdef RSLERR
    if (nerrors > 0)
        rslerr << nerrors << " errors found in EvaluateIRParams()\n";
#endif
    
    return nerrors;
}

// *********************************************************************
// *                                                                    
// * Method: AddNewSession
// *                                                                    
// * Description:   This method creates a new session resource.
// *                                                  
// * Inputs: strName - RWCString specifying the session name.
// *                                                                    
// * Outputs: None
// *                                                                    
// * Returns: A pointer to the ResStructure created, if Successfull
// *          else NULL
// *                                                                    
// *********************************************************************
//ResContext *runtimeRSL::AddNewSession(RWCString strSession, RWCString strSessionObj)
ResReference runtimeRSL::AddNewSession(RWCString strSession, RWCString strSessionObj)
{
    Logf.debug(LOGRSL) << "(runtimeRSL::AddNewSession) Create new session - '"
                       << strSession << "'" << endline;

    // *********************************
    // * Create a new session resource *
    // *********************************
    ResStructure *prsNewSession = CreateResource(strSessionObj, strSession);
    if (!prsNewSession)
    {
        Logf.error(LOGRSL) << "(runtimeRSL::AddNewSession) Unable to create new session `" 
                           << strSession << "'" << endline;
        ResReference refFail;
        return(refFail);
//        return(NULL);
    }

    // *********************************
    // * Get the session local context *
    // *********************************
    ResContext *prcSessionLocals;
    ResContext &rcSessionLocals = prsNewSession->GetLocalContext();
    prcSessionLocals = &rcSessionLocals;

    // ***************************************************
    // * give this session access to the system globals. *
    // ***************************************************
    prcSessionLocals->AddContext(SysGlobals);
    
    // ************************************************************
    // * Make session ID accessible to the objects in the session *
    // ************************************************************
    prcSessionLocals->AddResource(R_String::New("SessionID", strSession));

    // **************************************************
    // * run session events (declarations, etc) created *
    // * from the session keyword                       *
    // **************************************************
    sessionEv.execute(prcSessionLocals);
    
    // **************************************************
    // * Add the newly created session resource to the  *
    // * list of Sssions                                *
    // **************************************************
    Sessions->AddResource(prsNewSession);

    // **********************************
    // * Update some session statistics *
    // **********************************
	totalSessions++;
	currentSessions++;
	if (currentSessions > maxConcurrentSessions)
		maxConcurrentSessions = currentSessions;

    return(ResReference((Resource *) prsNewSession));
//    return(prcSessionLocals);
}

// *********************************************************************
// *                                                                    
// * Method: FindSession
// *                                                                    
// * Description:   This method tries to find the named session
// *                                                  
// * Inputs: strName - RWCString specifying the name of the session
// *                                                                    
// * Outputs: None
// *                                                                    
// * Returns: A pointer to the ResContext for the specifed session if successful
// *          else NULL
// *                                                                    
// *********************************************************************
ResContext *runtimeRSL::FindSession(RWCString strSession)
{
    Logf.debug(LOGRSL) << "(runtimeRSL::FindSession) Looking for session `" 
                       << strSession << "'" << endline;

    // ********************************
    // * Search the specified session *
    // * in the Sessions Context      *
    // ********************************
    ResStatus stat = Sessions->Find(strSession);
    if (stat.status == ResStatus::rslOk && stat.r)
    {
        // *******************************************
        // * For now, all sessions must be either a **
        // * resStruct or a resObj                  **
        // *******************************************
    	if ((stat.r->InternalType() == Resource::resStructType) ||
    		(stat.r->InternalType() == Resource::resObjType))
    	{
            Logf.debug(LOGRSL) << "(runtimeRSL::FindSession) Found session `" 
                               << strSession << "'" << endline;

            // ********************************************
            // * Get the local context for the specified **
            // * session resource.  This will be the     **
            // * session context.                        **
            // ********************************************
            ResContext *prcSessionLocals;
            ResContext &rcSessionLocals = ((ResStructure *) stat.r)->GetLocalContext();
            prcSessionLocals = &rcSessionLocals;

            return(prcSessionLocals);
        }
    }    

    return(NULL);
}

// *********************************************************************
// *                                                                    
// * Method: FindSessionRef
// *                                                                    
// * Description:   This method tries to find the named session
// *                                                  
// * Inputs: strName - RWCString specifying the name of the session
// *                                                                    
// * Outputs: None
// *                                                                    
// * Returns: A ResReference for the specifed session if successful
// *          else Empty ResReference
// *                                                                    
// *********************************************************************
ResReference runtimeRSL::FindSessionRef(RWCString strSession)
{
    Logf.debug(LOGRSL) << "(runtimeRSL::FindSessionRef) Looking for session ref `" 
                       << strSession << "'" << endline;

    // *************************************
    // * Search for the specified session **
    // * in the Sessions Context          **
    // *************************************
    ResStatus stat = Sessions->Find(strSession);
    if (stat.status == ResStatus::rslOk && stat.r)
    {
        // *******************************************
        // * For now, all sessions must be either a **
        // * resStruct or a resObj                  **
        // *******************************************
    	if ((stat.r->InternalType() == Resource::resStructType) ||
    		(stat.r->InternalType() == Resource::resObjType))
    	{
            Logf.debug(LOGRSL) << "(runtimeRSL::FindSessionRef) Found session `" 
                               << strSession << "'" << endline;

            return(ResReference(stat.r));
        }
    }    

    Logf.info(LOGRSL) << "(runtimeRSL::FindSessionRef) Unable to find session `" 
                       << strSession << "'" << endline;

    ResReference refFail;
    return(refFail);
}

// *********************************************************************
// *                                                                    
// * Method: KillSession
// *                                                                    
// * Description:   This method deletes the named session
// *                                                  
// * Inputs: strName - RWCString specifying the name of the session
// *                                                                    
// * Outputs: None
// *                                                                    
// * Returns: if successful
// *          else NULL
// *                                                                    
// *********************************************************************
int runtimeRSL::KillSession(RWCString strSession)
{
    Logf.debug(LOGRSL) << "(runtimeRSL::KillSession) Destroying session `" 
                       << strSession << "'" << endline;

    // **************************************
    // * First we have to find the session **
    // **************************************
    ResReference refSession = FindSessionRef(strSession);
    if (refSession.isValid())
    {
        // ********************************************
        // * if there is an Outgoing event Q we must **
        // * move it to the DeadSession pool so that **
        // * it can be dealt with later              **
        // ********************************************
        ResReference refOutQ = GetSessionOutQRef(strSession);
        if (refOutQ.isValid())
        {
            AddNewDeadSession(strSession, refOutQ);
        }

        // ********************************************
        // * Clear the content of the session         *
        // ********************************************
        refSession.Clear();

        // ***********************
        // * Remove the Session **
        // ***********************
        Sessions->RemoveResource(strSession);

        // **********************************
        // * Maintain the session counters **
        // **********************************
        currentSessions--;
        sessionsDoneThisPeriod++;

    	// new way
    	KillerEvents.clearAndDestroy();

#ifdef PURIFYTEST
        if (purify_is_running())
        {
            purify_printf("Session %s complete.\n", strSession.data());
            purify_new_leaks();
        }
#endif
    }
    return 0;
}

// *********************************************************************
// *                                                                    
// * Method: AddNewDeadSession
// *                                                                    
// * Description:   This method creates a new session resource.
// *                                                  
// * Inputs: strName - RWCString specifying the session name.
// *         refQutQ - ResReference containing the sessions outgoing event Q.
// *                                                                    
// * Outputs: None
// *                                                                    
// * Returns: Nothing
// *                                                                    
// *********************************************************************
void runtimeRSL::AddNewDeadSession(RWCString strSession, ResReference refOutQ)
{
    Logf.debug(LOGRSL) << "(runtimeRSL::AddNewDeadSession) Create new dead session - '"
                       << strSession << "'" << endline;

    // ******************************************
    // * If there is something in the sessions **
    // * outgoing event queue                  **
    // ******************************************
    if (((R_Queue *) refOutQ())->LogicalValue())
    {
        Logf.debug(LOGRSL) << "(runtimeRSL::AddNewDeadSession) Adding the outgoing event queue for session `" 
                           << strSession << "' to dead session." << endline;

        // **************************************************
        // * Add the Sessions outgoing event queue to the  **
        // * dead sessions context                         **
        // **************************************************
        DeadSessions->AddReferenceTo(strSession, refOutQ());
    }

    return;
}

// *********************************************************************
// *                                                                    
// * Method: GetSessionOutQRef
// *                                                                    
// * Description:   This method is used to get the Reference of the outgoing
// *                event queue for specified session.
// *                                                  
// * Inputs: strName - RWCString specifying the name of the session
// *                                                                    
// * Outputs: None
// *                                                                    
// * Returns: A ResReference for the specifed Outgoing event queue if successful
// *          else Empty ResReference
// *                                                                    
// *********************************************************************
ResReference runtimeRSL::GetSessionOutQRef(RWCString strSession)
{
    Logf.debug(LOGRSL) << "(runtimeRSL::GetSessionQutQRef) Getting session OutQ Reference `" 
                       << strSession << "'" << endline;

    // **************************************
    // * First we have to find the session **
    // **************************************
    ResReference refSession = FindSessionRef(strSession);
    if (refSession.isValid())
    {
        // *****************************************************
        // * Check to see if there is an outgoing event queue. *
        // *****************************************************
        ResReference refOutQueue = ((ResStructure *) refSession())->GetDataMember("OutQ");
        if (refOutQueue.isValid() && refOutQueue.TypeID() == R_Queue_ID) 
        {
            Logf.debug(LOGRSL) << "(runtimeRSL::GetSessionQutQRef) Found session OutQ Reference`" 
                               << strSession << "'" << endline;

            return(refOutQueue);
        }
    }

    Logf.debug(LOGRSL) << "(runtimeRSL::GetSessionQutQRef) Unable to find session OutQ Reference`" 
                       << strSession << "'" << endline;

    ResReference refFail;
    return(refFail);
}

// *********************************************************************
// *                                                                    
// * Method: GetSessionOutQ
// *                                                                    
// * Description:   This method returns the event queue for
// *                the specified session.
// *                                                  
// * Inputs: strName - RWCString specifying the name of the session
// *                                                                    
// * Outputs: None
// *                                                                    
// * Returns: A ResReference for the specifed Outgoing event queue if successful
// *          else Empty ResReference
// *                                                                    
// *********************************************************************
elist *runtimeRSL::GetSessionOutQ(RWCString strSession)
{
    Logf.debug(LOGRSL) << "(runtimeRSL::GetSessionQutQ) Looking for session OutQ `" 
                       << strSession << "'" << endline;

    // **************************************
    // * Get the Reference of the OutQ if  **
    // * if exists.                        **
    // **************************************
    ResReference refOutQ = GetSessionOutQRef(strSession);
    if (refOutQ.isValid())
    {
        elist *pevtQueue = ((R_Queue *) refOutQ())->getQ();
        ((R_Queue *) refOutQ())->getQ() = NULL;
        return(pevtQueue);
    }

    return(NULL);
}

// *********************************************************************
// *                                                                    
// * Method: GetDeadSessionOutQRef
// *                                                                    
// * Description:   This method tries to find the named session Outgoing
// *                event Q in the DeadSessions context.
// *                                                  
// * Inputs: strName - RWCString specifying the name of the session
// *                                                                    
// * Outputs: None
// *                                                                    
// * Returns: A ResReference for the specifed dead session OutQ if successful
// *          else Empty ResReference
// *                                                                    
// *********************************************************************
ResReference runtimeRSL::GetDeadSessionOutQRef(RWCString strSession)
{
    Logf.debug(LOGRSL) << "(runtimeRSL::GetDeadSessionOutQRef) Looking for dead session OutQ `" 
                       << strSession << "'" << endline;

    // ************************************************
    // * Try to find the session in the DeadSessions **
    // * context                                     **
    // ************************************************
    ResStatus stat = DeadSessions->Find(strSession);

    if (stat.status == ResStatus::rslOk && stat.r
         && stat.r->TypeID() == R_Queue_ID)
    {
        Logf.debug(LOGRSL) << "(runtimeRSL::GetDeadSessionOutQRef) Found a dead session OutQ Reference for session `" 
                           << strSession << "'" << endline;

        return(ResReference(stat.r));
    }    

    Logf.debug(LOGRSL) << "(runtimeRSL::GetDeadSessionOutQRef) Unable to find a dead session OutQ Reference for session `" 
                       << strSession << "'" << endline;

    ResReference refFail;
    return(refFail);
}

// *********************************************************************
// *                                                                    
// * Method: GetDeadSessionOutQ
// *                                                                    
// * Description:   This method returns the event queue for
// *                the specified session in the Dead Session context.
// *                                                  
// * Inputs: strName - RWCString specifying the name of the session
// *                                                                    
// * Outputs: None
// *                                                                    
// * Returns: An elist pointer to the event list if successful
// *          else NULL
// *                                                                    
// *********************************************************************
elist *runtimeRSL::GetDeadSessionOutQ(RWCString strSession)
{
    Logf.debug(LOGRSL) << "(runtimeRSL::GetDeadSessionOutQ) Looking for dead session OutQ for session `" 
                       << strSession << "'" << endline;

    // **************************************
    // * Get the Reference of the OutQ if  **
    // * if exists.                        **
    // **************************************
    ResReference refOutQ = GetDeadSessionOutQRef(strSession);
    if (refOutQ.isValid())
    {
        elist *pevtQueue = ((R_Queue *) refOutQ())->getQ();
//        ((R_Queue *) refOutQ())->getQ() = NULL;
        return(pevtQueue);
    }

    return(NULL);
}

// *********************************************************************
// *                                                                    
// * Method: KillDeadSession
// *                                                                    
// * Description:   This method deletes the named session from the
// *                DeadSession context
// *                                                  
// * Inputs: strName - RWCString specifying the name of the session
// *                                                                    
// * Outputs: None
// *                                                                    
// * Returns: if successful
// *          else NULL
// *                                                                    
// *********************************************************************
void runtimeRSL::KillDeadSession(RWCString strSession)
{
    Logf.notice(LOGRSL) << "(runtimeRSL::KillDeadSession) Destroying dead session `" 
                        << strSession << "'" << endline;

    DeadSessions->RemoveResource(strSession);

    return;
}

// *********************************************************************
// *                                                                    
// * Method: FindResource
// *                                                                    
// * Description:   This method tries to find the named Resource
// *                in the named session.
// *                                                  
// * Inputs:    strSession - RWCString specifying the name of the session to search
// *            resName - RWCString specifying the name of the resource to find.
// *                                                                    
// * Outputs: None
// *                                                                    
// * Returns: A ResReference of the found resource if successful
// *          else NULL
// *                                                                    
// *********************************************************************
ResReference runtimeRSL::FindResource(RWCString strSession, RWCString resName)
{
    Logf.debug(LOGRSL) << "(runtimeRSL::FindResource)" << endline;

    // *************************************
    // * Search for the specified session **
    // * in the context of all sessions   **
    // *************************************
    ResContext *rcSession = FindSession(strSession);

    if (rcSession)
    {
        // **************************************
        // * Now look for the resource within  **
        // * the context of the found session  **
        // **************************************
        ResStatus stat = rcSession->Find(resName);

        if (stat.status == ResStatus::rslOk && stat.r)
        {
            Logf.debug(LOGRSL) << "(runtimeRSL::FindResource) Found resource `" << resName
                               << "' in session '" << strSession << "'" << endline;

            return(ResReference(stat.r));
        }    
    }

    Logf.info(LOGRSL) << "(runtimeRSL::FindResource) Unable to find resource `" << resName
                       << "' in session '" << strSession << "'" << endline;

    ResReference refFail;
    return(refFail);
}


// *********************************************************************
// *                                                                    
// * Method: FindClass
// *                                                                    
// * Description:   This method finds the specified class
// *                It then returns a pointer to the found res_class
// *                                                  
// * Inputs: strClass - RWCString specifying the name for the resource.
// *                                                                    
// * Outputs: None
// *                                                                    
// * Returns: A pointer to the res_class found, if Successfull
// *          else NULL
// *                                                                    
// *********************************************************************
res_class *runtimeRSL::FindClass(RWCString strClass)
{
    Logf.debug(LOGRSL) << "(runtimeRSL::FindClass) Looking for class '" 
                       << strClass << "'" << endline;
 
    // ***********************************
    // * Find the specifed resource type *
    // ***********************************
    res_class findrc(strClass);
    res_class *rcFound = ResClasses.find(&findrc);

    if (!rcFound)
    {
        Logf.info(LOGRSL) << "(runtimeRSL::FindClass) Can not find class - '" 
                          << strClass << "'" << endline;
        return(NULL);
    }

    return(rcFound);
}


// *********************************************************************
// *                                                                    
// * Method: CreateResource
// *                                                                    
// * Description:   This method creates a named resource of the specified type.  
// *                It then returns a pointer to the newly create ResStructure.
// *                                                  
// * Inputs: strName - RWCString specifying the name for the resource.
// *         strType - RWCString specifying the type of resource to create.
// *         rlConstructArgs - ResList containing the arguments to the constructor
// *         rcConstructContext - ResContext containing the context for constructor
// *                                                                    
// * Outputs: None
// *                                                                    
// * Returns: A pointer to the ResStructure created, if Successfull
// *          else NULL
// *                                                                    
// *********************************************************************
ResStructure *runtimeRSL::CreateResource(RWCString strRType, RWCString strRName, 
                                        ResList *rlConstructArgs, ResContext *rcConstructContext)
{
    Logf.debug(LOGRSL) << "(runtimeRSL::CreateResource) Create object - '" 
                       << strRName << "' of type - '" << strRType << "'" << endline;

    // ***********************************
    // * Find the specifed resource type *
    // ***********************************
    res_class *rcFound = FindClass(strRType);

    if (!rcFound)
    {
        return(NULL);
    }

    // ********************************************************
    // * Create a new resource instance of the specified type *
    // ********************************************************
    Resource *rNewResource = rcFound->New(strRName, rlConstructArgs, rcConstructContext);
    if (!rNewResource)
    {
        Logf.info(LOGRSL) << "(runtimeRSL::CreateResource) Can not create object - '" << strRName 
                          << "' of type - '" << strRType << "'" <<  endline;
        return(NULL);
    }

    // **************************************************
    // * Make sure that the newly created resource is a *
    // * resStructure or a resObject                    *
    // **************************************************
    if ((rNewResource->InternalType() != Resource::resStructType) &&
        (rNewResource->InternalType() != Resource::resObjType))
    {
        Logf.info(LOGRSL) << "(runtimeRSL::CreateResource) Newly created object - '"
                          << strRName << "' is not streamable" << endline;
        rcFound->Delete(rNewResource);
        return(NULL);
    }

    return((ResStructure *) rNewResource);
}


void runtimeRSL::AddImport(RWCString imp)
{
    rslPackage rpack(imp);

    if (!Import.contains(rpack))
        Import.append(rpack);
}

//  ImportPackages()
//  What to do with "import" statements.
int runtimeRSL::ImportPackages()
{
    int nImports = Import.entries();

#ifdef RSLERR
    rslerr << "ImportPackages()\n";
#endif

    if (nImports <= 0)
    {
#ifdef RSLERR
        rslerr << "\t(none)\n";
#endif
        return 0;
    }

    // Get the package search path from the environment variable.
    // Direct assignment to an RWCString from the result of getenv()
    // caused a core dump if the var isn't found (getenv() returns null)
    RWCString rsl_classpath;
    char *p = getenv(RSL_CLASSPATH); // rsldefaults.h
    if (p)
        rsl_classpath = p;
#ifdef RSLERR
    else
        rslerr << "No environment variable `" << RSL_CLASSPATH
            << "' found. Using current directory.\n";
#endif

    DirList classpath(rsl_classpath);

    // Since AddImport() appends to the END of
    // the list, then for any import statements encountered
    // during LoadLibrary(), the list iterator will simply
    // continue to go through to the end.
    RWTValSlistIterator<rslPackage> iter(Import);
    int nerrors=0;

    nImports = Import.entries();

    while(iter())
    {
        rslPackage rpack = iter.key();

#ifdef RSLERR
        rslerr << "\tpackage `" << rpack.name << "': "
            << (rpack.loaded? "already loaded" : "not loaded") << ".\n";
#endif

        if (rpack.loaded == rslPackage::notLoaded)
        {
            nerrors += LoadPackage(rpack, classpath);
        }
    }

#ifdef RSLERR
    if (nerrors > 0)
        rslerr << nerrors << " errors found in ImportPackages(), returned from LoadLibrary()\n";
#endif
    
    return nerrors;
}

// LoadPackage()
// Find a directory for the package and load the rsl files there.
int runtimeRSL::LoadPackage(rslPackage& rpack, DirList& classpath)
{
    rslPackage::load_t stat = rslPackage::notLoaded;


    // find a pathname for the package directory
    RWCString path = classpath.findPath(rpack.name);

#ifdef RSLERR
    rslerr << "LoadPackage() for `" << rpack.name << "'\n";
    rslerr << "Found package directory `" << path << "'\n";
#endif

    Logf.info(LOGRSL) << "Importing package `" << rpack.name << "' from directory `"
        << path << "'" << endline;

    int nerrors = 0;

    if (path.length() <= 0)
        stat = rslPackage::notFound;
    else
    {
        DirContents dirc(path);

#ifdef RSLERR
        rslerr << "RSL files to load:\n";
        rslerr << dirc << endl;
#endif

        LoadLibrary(rpack, dirc);

        nerrors += ParseFilesInDirectory(dirc);

        stat = rslPackage::Loaded;
    }

    // upadate the actual object
    rslPackage q;
    if (Import.find(rpack, q))
        q.loaded = stat;

    return nerrors;
}

//  LoadLibrary()
//  Load the shared library indicated by rpack, and use the filenames
//  in dirc to build method symbols to load res_class objects for
//  each Resource in the library.
int runtimeRSL::LoadLibrary(rslPackage& rpack, DirContents& dirc)
{
    RWCString libname = "";
    
    if (dynamicLibs)
        libname = RWCString("lib") + rpack.name + ".so";

    Logf.info(LOGRSL) << "Loading dynamic library `" << libname << "'" << endline;
        
#ifdef RSLERR
    rslerr << "LoadLibrary(): find library `" << libname
        << "' opening...\n";
#endif
    
    // if libname is of zero length, then this will allow
    // symbols to be gotten from the main segment
    SharedLibrary shlib(rpack.name, libname, 0);
    //shlib.open();
    shlib.open(RTLD_NOW | RTLD_GLOBAL);

    if (shlib.error())
    {
        // This is not an error in all cases, since a package doesn't
        // require a library implementation! It could all be
        // implemented in rsl...

		Logf.debug(LOGRSL) << "LoadLibrary : "
			<< shlib.lastError() << endline;
		Logf.debug(LOGRSL) << "  (not an error if this package has no C++)"
			<< endline;

#ifdef RSLERR
        if (dynamicLibs)
            rslerr << "Error: " << shlib.lastError() << endl;
#endif

        return 0;
    }

    // Store the library. This used to be ClassToLibrary, but now that
    // we're using the rsl filename as the class name (sans .rsl), we
    // can load the symbols right here without having to go back to
    // this shared lib collection later. It's stored here so we can
    // close it or unload it later.
    NameToLibrary.insertKeyAndValue(rpack.name, shlib);

    Logf.debug(LOGRSL) << "dynamic library `" << libname << "' loaded successfully."
		<< endline;

    // Want to load res_class creator functions for each Resource,
    // so we use the file name as the class name to create the
    // symbol. The symbol name is built from macros in rsldefaults.h,
    // but most probably it is: for rsl class X, the function is
    //      res_class *Create_X_RC();
    // and the C-linkage symbol is just Create_X_RC.

	Logf.debug(LOGRSL) << "Searching for bootstrap C function symbols from class file names..."
		<< endline;

    RWTValSlist<RWCString> &files = dirc.getList();
    RWTValSlistIterator<RWCString> iter(files);
    DRWCString s, symbol;
    while(iter())
    {
        s = iter.key();

		if ( (s.length() < 5) || (s(s.length() - 4, 4) != ".rsl") )
        {
            Logf.debug(LOGRSL) << "Skipping file `" << s << "'" << endline;
            continue;
        }

        s = s.before(".");

        // create the symbol name
        symbol = CREATE_FN_PREFIX + s + CREATE_FN_SUFFIX;

        Logf.debug(LOGRSL) << "load symbol `" << symbol << "'" << endline;

        // Get the symbol
        res_class* (*RC_Creator)() = (res_class *(*)()) shlib.getSymbol(symbol.data());
        
        if (shlib.error())
        {
            // probably symbol not found. Report this error and skip it.
            Logf.info(LOGRSL) << "LoadLibrary: " << shlib.lastError() << endline;
			Logf.debug(LOGRSL) << "\t(skipping file `" << iter.key() << "')" << endline;
            continue;
        }
        
        // Call the function -- get the res_class
        res_class *theRC = (*RC_Creator)();
        
        if (theRC)
        {
            ResClasses.insert(theRC);

            Logf.debug(LOGRSL) << "Found RSL class `" << theRC->Name()
                << "' in library `" << libname << "' implemented in C++" << endline;

        }
		else
			Logf.debug(LOGRSL) << "No C++ implementation found for `" << s << "'" << endline;
    }

    Logf.info(LOGSERVER) << "Library `" << shlib.name() << "' loaded from path `"
        << shlib.realPath() << "'" << endline;

    return 1;
}

// ParseFilesInDirectory()
// Given a directory name, parse all files in it that:
//      a) contain ".rsl"
//      b) do not begin with '.' (eg: old Razor backup files)
int runtimeRSL::ParseFilesInDirectory(DirContents& dirc)
{
    RWTValSlistIterator<RWCString> iter(dirc.getList());
    RWCString s;
    lexer_context lexc;
    int nerrors=0;

    while(iter())
    {
        s = iter.key();

	if ( (s.length() < 4) || (s(s.length() - 4, 4) != ".rsl") )
        {
            logf->debug(LOGSERVER) << "RSL: parser skipping file `"
                << s << "': not `.rsl'" << endline;
            continue;
        }
        
        if (s[0] == '.')
        {
            logf->debug(LOGSERVER) << "RSL: parser skipping file `"
                << s << "': begins with `.'" << endline;
            continue;
        }
        
        s.prepend("/");
        s.prepend(dirc.path());
        nerrors += ParseFile(s.data(), lexc);
#ifdef RSLERR
        rslerr << "\tpath: `" << s << "'\n";
#endif
    }

#ifdef RSLERR
    if (nerrors > 0)
        rslerr << nerrors << " errors in ParseFilesInDirectory()\n";
#endif

    return nerrors;
}

//  ImportAndLink()
//  Import all packages, if there are any
//  evaluate inline resources which are method parameters
//  link methods to res_class objects
int runtimeRSL::ImportAndLink(void)
{
    int nerrors=0;
    

    nerrors += ImportPackages();
    nerrors += EvaluateIRParams();
    nerrors += LinkMethods();

    CreateUserGlobals();

    
    return nerrors;
}


#define _TAB '\t'
void runtimeRSL::printStats(ostream &out)
{
    static bool once = FALSE;

    // write column headers (only once!)
    if (!once)
    {
        out << "process " << (getpid()) << endl
            << "nResContextAdds"
            << "\tnFreelistAdds"
            << "\tFreelistRemoves"
            << "\tResourcesCreated"
            << "\tactualResCreations"
            << "\ttotalR_Strings"
            << "\tProgramResources"
            << "\tResourcesDestroyed"
            << "\tResRefsCreated"
            << "\tResRefsDestroyed"
            << "\tnonResRefResources"
            << "\tResObjs"
            << "\tifStatements"
            << "\tifContexts"
            << "\teventsCreated"
            << "\tprogramEvents"
            << "\tnonProgramEvents"
            << "\teventsDestroyed"
            << "\targEventsDestroyed"
//            << "\teventsSaved"
            << "\targsResolved"
            << endl;

        once = TRUE;    // done.
    }

    out << nResContextAdds
        << _TAB << nFreelistAdds
        << _TAB << nFreelistRemoves
        << _TAB << nResourcesCreated
        << _TAB << actualResCreations
        << _TAB << totalR_Strings
        << _TAB << nProgramResources
        << _TAB << nResourcesDestroyed
        << _TAB << nResRefsCreated
        << _TAB << nResRefsDestroyed
        << _TAB << (nResourcesCreated - nResRefsCreated)
        << _TAB << nResObjs
        << _TAB << ifStatements
        << _TAB << ifContexts
        << _TAB << eventsCreated
        << _TAB << programEvents
        << _TAB << nonProgramEvents
        << _TAB << eventsDestroyed
        << _TAB << argEventsDestroyed
//        << _TAB << (ResultsAccumulator.entries())
        << _TAB << argsResolved
        << endl;

}

// *************************************
// record statistics about the freelists
// *************************************
void freelistStats(ostream &out)
{
    static bool once = FALSE;

    if (!Running)
        return;

    res_class *rc = NULL;

    // **********************************
    // write column headers: class names
    // (but only once!)
    // **********************************
    RWTPtrHashTableIterator<res_class> iter(ResClasses);
    if (!once)
    {
        out << "eciNumber" << _TAB;
        while(iter())
        {
            rc = iter.key();
            if (rc)
                out << rc->Name() << _TAB;
        }
        out << endl;
        once = TRUE;    // done.
    }

    // *********************************
    // write column entries (only once!)
    // *********************************
    RWTPtrHashTableIterator<res_class> iter2(ResClasses);

    // first column: which eci command number this is for..
    out << eciCommands << _TAB;

    while(iter2())
    {
        rc = iter2.key();
        if (rc)
            out << rc->FreeListEntries() << _TAB;
    }
    out << endl;
}

// print stats in eci format
void runtimeRSL::eciStats(ostream& out)
{
    out << "process_" << getpid() << "_stats: List { "
        << "ResContextAdds: " << nResContextAdds
        << ", FreelistAdds: " << nFreelistAdds
        << ", FreelistRemoves: " << nFreelistRemoves
        << ", ResourcesCreated: " << nResourcesCreated
        << ", programResources: " << nProgramResources
        << ", ResourcesDestroyed: " << nResourcesDestroyed
        << ", ResRefsCreated: " << nResRefsCreated
        << ", ResRefsDestroyed: " << nResRefsDestroyed
        << ", nonRefResources: " << (nResourcesCreated - nResRefsCreated)
        << ", ResObjs: " << nResObjs
        << ", eventsCreated: " << eventsCreated
        << ", eventsDestroyed: " << eventsDestroyed
        << " }";
}

// rslStartup()
//  Do the command line thing. Parse given files.
int runtimeRSL::rslStartup(int argc, char **argv)
{
    int nerrors=0, server=0, htmlit=0, printit=0;

#ifdef DEBUG
    printit=1;
#endif

    int portnum=0, forceit=0, okToRun=0;
    bool printStatistics = FALSE;

    RWTValSlist<StringPair> ServerParams;       // List of Name/Value pairs to pass to server
    StringPair CmdLinePair;

    RWCString clo;
    DRWCString startWith="";

    // A lexer context for loading all the command line files
    lexer_context lexc;

    // import rsl basic types by default!
    AddImport("rsl");

    for (int i=1; i<argc; i++)
    {
        clo = argv[i];

        if (clo == "-port")
        {
            CmdLinePair("port", argv[++i]);         // This command line option needs to be
            ServerParams.append(CmdLinePair);       // passed to the server
        }
        else
        if (clo == "-fifo")
        {
            CmdLinePair("fifo", argv[++i]);         // This command line option needs to be
            ServerParams.append(CmdLinePair);       // passed to the server
        }
        else
        if (clo == "-print")
            printit=1;
        else
        if (clo == "-html")
            htmlit=1;
        else
        if (clo == "-groupexpr")
            lexc.bGroupOpExpr = 1;
        else
        if (clo == "-server")
            server = 1;
        else
        if (clo == "-static")
            dynamicLibs = 0;
        else
        if (clo == "-dynamic")  // the default, but acceptable
            dynamicLibs = 1;
        else
        if (clo == "-start")
            startWith = argv[++i];
        else
        if (clo == "-stats")
            printStatistics = TRUE;
        else
        if (clo == "-statsfile")
        {
            statsFile = new ofstream(argv[++i]);
            freelistFile = new ofstream((RWCString(argv[i]) + "_freelist").data());

            if (!statsFile)
                cerr << "-statsfile: unable to open. (ignoring)\n";
            if (!freelistFile)
                cerr << "-statsfile: unable to open for freelist file. (ignoring)\n";
        }
        else
        if (clo == "-nofreelist")
            useFreelist = FALSE;
        else
        if (clo == "-ignore")
            forceit = 1;
        else
        if (clo == "-trace")
            rslMethod::traceMode = 1;
        else
        if (clo == "-traceargs")
            rslMethod::traceMode = 2;
        else
            nerrors = ParseFile(argv[i], lexc);

        if (nerrors > 0)
            break;
    }

    if (nerrors > 0)
        return nerrors;

#ifdef RSLERR
    rslerr << "\n/* Done Parsing. */\n\n";
    rslerr << "// Linking methods..\n// ===================\n";
#endif

    nerrors += ImportAndLink();

#ifdef RSLERR
    rslerr << "\n// ===================\n";

    if (nerrors > 0)
        rslerr << endl << nerrors << " link errors found.\n";
#endif

    if (printit)
    {
        print();
        return 0;
    }

    if (htmlit)
    {
        htmlClasses();
        return 0;
    }
    
    Running = TRUE;

    if (startupECI.entries() > 0)
        executeStartupECI();

    okToRun = (forceit || nerrors==0);
    
    if (okToRun)
    {
        if (startWith.length() > 0)
            StartWith(startWith, ServerParams);
        else
        if (server)
            StartWith(DEFAULT_SERVER, ServerParams);
    }

    if (printStatistics)
        printStats(cout);
    
    return nerrors;
}

// StartWith()
#define STARTUP_CONTEXT_NAME "Start"
void runtimeRSL::StartWith(DRWCString sw, RWTValSlist<StringPair> ServerParams)
{
    DRWCString obj, meth;

#ifdef RSLERR
    rslerr << "====  IN STARTWITH  ===== \n";
#endif

    if (sw.contains("."))
    {
        obj = sw.before(".");
        meth = sw.after(".");
    }
    else
    {
        obj = sw;
        meth = AR_CreateOnly_Method;
    }

// Add Remember TGM - 12/21/98
    AuditRequest *areq = (AuditRequest *) Remember (new AuditRequest(
        (char *) obj.data(),
        (char *) STARTUP_CONTEXT_NAME,
        (char *) meth.data(),
        (char *) "Server_Startup"));
    areq->kind |= event::dynamicRequestKind;

    // ****************************************************
    // * If Server specific parameters were specified on  *
    // * the command line, then we will pass them as a    *
    // * argument of type R_Table to the startup method.  *
    // ****************************************************
    if (ServerParams.entries())
    {
        // *********************************************
        // * Create a new table resource to pass as a  *
        // * parameter to the server                   *
        // *********************************************
        R_Table *tblParameters = R_Table::New("tblServerParams");
        ResContext &rcParameters = tblParameters->GetLocalContext();

        RWTValSlistIterator<StringPair> iterPairs(ServerParams);
        StringPair ParamPair;

        while(iterPairs())
        {
            ParamPair = iterPairs.key();
            ResReference refDataPair(R_String::New(ParamPair.left(), ParamPair.right()));
            rcParameters.AddResource(refDataPair());            
        }

        // ********************************************************************
        // * Add the table of params as an argument to the Web_Startup method *
        // ********************************************************************
        areq->arguments = (elistArg *) Remember(new elistArg);
        areq->arguments->add((ResArg *) Remember(new ResArg(tblParameters)));
    }

    event *outgoing = areq->execute(NULL);
    
    if (outgoing)
        outgoing->print(cout);

    KillSession(STARTUP_CONTEXT_NAME);

#ifdef RSLERR
    rslerr << flush;
#endif
}

void runtimeRSL::executeStartupECI(void)
{
    event *e=NULL, *retev=NULL;

    // traverse & simultaneously clear the list
    while(!startupECI.isEmpty())
    {
        e = startupECI.get();

        // execute the event
        retev = e->execute(NULL);

        // log the ECI responses
        if (retev)
        {
            Logf.notice(LOGRSL) << retev << endline;
            //delete retev;
// Commented out TGM - 12/21/98
//			Remember(retev);
        }

        //delete e;
// Commented out TGM - 12/21/98
//		Remember(e);

        e = NULL;
        retev = NULL;
    }
}

// ***********************
// *  ParseFile
// ***********************
int runtimeRSL::ParseFile(const char *fname, lexer_context &lexc)
{
    logf->notice(LOGSERVER) << "Parsing RSL file `" << fname << "'." << endline;

    ifstream in(fname);

    if (!in)
    {
        Logf << "Unable to read." << endline;
        return 1;
    }

    // Set "current lexer" variables
    lexc.SetSource(lexer_context::File, fname);
    lexc.SetLexerIO(in);

    lexc.done = lexer_context::Ready;
    lexc.type = lexer_context::batch;
    lexc.keystate = lexer_context::Everywhere;

    int nerrors = 0;

    while (!nerrors)
    {

        nerrors = parse_it(lexc);

        // handle eci commands (dynamic requests) in files
        // add them to a list to be executed later
        if (lexc.done == lexer_context::GotECIReq && lexc.dynamic_request)
        {
            Logf.info(LOGRSL) << "Parsed ECI request `" << lexc.dynamic_request
                << "': delayed execution." << endline;

            startupECI.insert(lexc.dynamic_request);

            lexc.done = lexer_context::Ready;
        }
        else
        if (in.eof())
        {
            if (nerrors == 1)
                nerrors = 0;
            break;
        }
        else
            break;
    }

    if (nerrors > 0)
        Logf.error(LOGSERVER) << nerrors << " errors parsing RSL file `" << fname
            << "'." << endline;
    
    return nerrors;
}
