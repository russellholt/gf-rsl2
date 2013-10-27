// ***************************************************************************
// *
// *  NAME:  rslServer.cc
// *
// *  RELATED RESOURCES: 
// *                                                                    
// *  DESCRIPTION:                                                      
// *     Abstract server resource class
// *                                                                    
// * $Id: rslServer.cc,v 1.3 1999/01/22 20:46:12 toddm Exp $
// *
// * $Log: rslServer.cc,v $
// * Revision 1.3  1999/01/22 20:46:12  toddm
// * Clean up events for each request
// *
// * Revision 1.2  1998/12/22 19:19:38  toddm
// * Fix event cleanup
// *
// * Revision 1.1  1998/11/17 23:54:14  toddm
// * Initial revision
// *
// * Revision 2.3  1998/06/29 19:34:13  toddm
// * Replace true with TRUE and false with FALSE
// *
// * Revision 2.2  1998/04/30 18:59:46  toddm
// * Fix logging of PeriodicInfo
// *
// *
// * Copyright (c) 1996, 1997, 1998 by Destiny Software Corporation
// *
// ***************************************************************************
// *******************
// * System Includes *
// *******************
#include <iostream.h>
#include <fstream.h>
#include <strstream.h>
#include <strings.h>
#include <time.h>
#include <stdlib.h>

// ******************
// * Local Includes *
// ******************
#include "rslServer.h"
#include "slog.h"
#include "lexer_context.h"
#include "runtime.h"
#include "R_String.h"
#include "R_Integer.h"
#include "R_Boolean.h"
#include "destiny.h"

#include "killevents.h"

#ifdef RSLERR
extern ofstream rslerr;
#endif

extern int parse_it(lexer_context &lexc);
extern slog *logf;

// TEMPORARY HACK in b.cc
//extern void elist___kill(event *& el);

// Standard outgoing events
#define _hDISPLAY 671615600 // Display
#define _hCLOSE 644640627   // Close
#define _hQUIT 1366649204   // Quit
#define _hALERT 896296306   // Alert
#define _hERROR 930247279   // Error
#define _hEXITSESSION 2138207239    // ExitSession

#define _hNEWCONNECTION 746549321   // NewConnection
#define _hINTERNAL_STARTSERVER 1683052415   // Internal_StartServer
#define _hLOGPERIODICSTATISTICS 962687078   // LogPeriodicStatistics
#define _hINTERNAL_NEWCONNECTION 757885561  // Internal_NewConnection

#define _hSETLOGLEVEL 1248285737    // SetLogLevel
#define _hSETLOGWHICHSYSTEMS 1096892178 // SetLogWhichSystems
#define _hSETUSEFILELOG 728071763	// SetUseFileLog
#define _hSETLOGFILENAME 1023573060	// SetLogFileName
#define _hSETMAXUSERS 1466913342    // SetMaxUsers
#define _hSETMAXUSERMESSAGE 1092357436  // SetMaxUserMessage
#define _hSETTOTALSESSIONSALLOWED 806486103	// SetTotalSessionsAllowed
#define _hSETTOTALREQUESTSALLOWED 1024333905	// SetTotalRequestsAllowed
#define _hSETLOGSTATINTERVAL 1216289824	// SetLogStatInterval

#define _hPARSERSLFILE 1935297882   // ParseRSLFile
#define _hSHUTDOWNATZEROSESSIONS 421290613  // ShutdownAtZeroSessions
#define _hSHUTDOWN 923206170	// Shutdown
#define _hCANCELSHUTDOWN 605509988	// CancelShutdown


rslServer::rslServer(RWCString nm, res_class *subclass_class)
    : ResObj(nm, subclass_class)
{
    quitWhenZeroSessions = FALSE;
    shutdownTime = -1;

    timeoutManager = NULL;

    // find the timeout manager
    ResStatus stat = runtimeStuff.SysGlobals->Find("TimeoutManager");
    if (stat.status == ResStatus::rslOk && stat.r
            && stat.r->TypeID() == R_TimeoutManager_ID)
        timeoutManager = (R_TimeoutManager *) (stat.r);
}

// subclasses should override this
void rslServer::ShutdownNow()
{
    LogPeriodicInfo();
    exit(0);
}


// ***************
// * listen loop *
// ***************
void rslServer::ListenLoop(int portnum,int toFork)
{
    int nerrors=0, bindresult=0;

    // Fork variables
    Fork::suicide_signal (SIGTERM);
    int childpid;
    int childstat, childstatpid;
    int killchild = 1;  // kill child when parent terminates.
    int reason = 1;

    sockinetbuf sin (sockbuf::sock_stream);

    if (portnum > 0)
    {
        bindresult=sin.bind(INADDR_ANY, portnum);
    }
    else
        bindresult=sin.bind();
        
        
    if (bindresult != 0)
    {
        Logf.fatal(LOGSERVER) << "Unable to bind to address." << endline;
        cerr << "Unable to bind to address.\n";
        return;
    }
        
    // Set some socket options
    
    sin.reuseaddr(1);
    sin.keepalive(1);

    cout << "localhost = " << sin.localhost() << endl
        << "localport = " << sin.localport() << endl << flush;

    sin.listen();


    // ***************
    // * listen loop *
    // ***************
    for(;;)
    {
        Logf.notice(LOGSERVER) << "RSL Server: Waiting for connection..." << endline;

        cout << "RSL Server: Waiting for connection...\n" << flush;
        iosockinet s(sin.accept());

        if (toFork)
        {
            Fork *spawning = new Fork(killchild, reason);
    
            if (spawning && spawning->process_id() == -1)
            {
                perror("fork failed");
                exit(-5);
            }
            
            if (spawning->is_child())
            {
                NewConnection(s);
                delete spawning;
                return;
            }

            delete spawning;
        }
        else
            NewConnection(s);

        cout << "RSL Server: Close client socket.\n" << flush;
        s->close();
    }
}


// ******************************************
// ** NewConnection -- the child process   **
// ******************************************
void rslServer::NewConnection(iostream& in)
{
    cout << "RSL Server: Accepted connection, pid " << (getpid()) << endl
         << "Switching lexer streams with client socket.\n" << flush;

    // Set current lexer state
    lexer_context lexc;
    lexc.SetSource(lexer_context::Stream, "<network>");
    lexc.SetLexerIO(in, in);
    lexc.done = lexer_context::Ready;
    lexc.type = lexer_context::interactive;
    lexc.keystate = lexer_context::Everywhere;

	do
    {
        RoundTripRequest();
    } while (lexc.done == lexer_context::Ready);

}


// RoundTripRequest()
// Full "round trip":
//  - get incoming events
//  - execute RSL2 event and get outgoing RSL2 events
//  - translate outgoing RSL2 events to RSL1 Events
//  - send outgoing RSL1 Events to the RMG object.
// Returns 0 on success, -1 on failure
int rslServer::RoundTripRequest(void)
{
    InitRequest();
    
    GetIncomingRequest();
    
    if (!ValidRequest())
    {
#ifdef RSLERR
        rslerr << "RoundTripRequest: got a bad Request. Returning.\n" << flush;
#endif
        return -1;
    }

    event *toRSL = TranslateIncoming();

    event *fromRSL = ExecuteIncomingEvents(toRSL);

    Outbound(fromRSL);

	KillerEvents.clearAndDestroy();
	
    return 0;
}

void rslServer::Outbound(event *fromRSL)
{
    TranslateOutLoop(fromRSL);

    FinalizeRequest();
    
// Commented out TGM - 12/21/98
//    elist___kill(fromRSL);
}

void rslServer::InitRequest()
{
#ifdef RSLERR
    rslerr << "R_Server::InitRequest()\n";
#endif
}

void rslServer::FinalizeRequest()
{
#ifdef RSLERR
    rslerr << "R_Server::FinalizeRequest()\n";
#endif
}


// TranslateOutLoop
// Outgoing events are those returned by RSL. This loops
// through and calls TranslateOutEvent() for each one.
void rslServer::TranslateOutLoop(event *outgoing)
{

    if (!outgoing)
    {
#ifdef RSLERR
        rslerr << "rslServer::TranslateOutLoop for NULL event.\n";
#endif
        return;
    }

#ifdef RSLERR
    outgoing->print(rslerr);
    rslerr <<"'\n" << flush;
#endif

    // log the outgoing event (event to the harness)
    // note that sending an event * to the slog object is limited
    // to 65536 total bytes. this is because slog is not an ostream
    // class and allocates an internal strstream object with a
    // static char* buf of that size. yeah. ugg.
    logf->notice(LOGSERVER) << "Harness<-framework: " << outgoing << endline;


    if (outgoing->isA(event::elistKind))
    {
        RWTPtrSlistIterator<event> iter(((elist *) outgoing)->evtl);
        event *e=NULL;

        while(iter())
        {
            e = iter.key();
            if (e && e->isA(event::elistKind))
            {
#ifdef RSLERR
                rslerr << "===> TranslateOutLoop(): found elist: `";
                e->print(rslerr);
                rslerr << "': recursing...\n" << flush;
#endif

                TranslateOutLoop(e);
            }
            else
            if (e)
                TranslateOutEvent(e);
        }
    }
    else
        TranslateOutEvent(outgoing);
}


// R_RMGServer::TranslateOutEvent
// Interpret one outgoing event from RSL, such as returned by:
//      return Display: "PersInfoForm";
//
// This implementation provides the basic set of supported
// outgoing events, namely
//      Display, Close, Alert, Quit, Error.
//
// Upon receiving an event not in this set, we call the
// virtual function subTranslateOutEvent(), to be implemented
// by subclasses to extend this set of events. For example,
// the RMGServer adds GetUserData.
//
// See rslServer design notes for comments about how this might
// be done better.
int rslServer::TranslateOutEvent(event *e)
{
#ifdef RSLERR
    rslerr << "rslServer::TranslateOutEvent: `";
    e->print(rslerr);
    rslerr << "': ";
    
    if (e->isA(event::requestKind))
        rslerr << "<requestKind>";
    
    if (e->isA(event::elistKind))
        rslerr << "<elistKind>";
        
    if (e->isA(event::resKind))
        rslerr << "<resKind>";
        
    if (e->isA(event::argKind))
        rslerr << "<argKind>";
        
    if (e->isA(event::resArgKind))
        rslerr << "<resArgKind>";
        
    if (e->isA(event::argListKind))
        rslerr << "<argListKind>";
        
    if (e->isA(event::objReqArgKind))
        rslerr << "<objReqArgKind>";
        
    if (e->isA(event::elistArgKind))
        rslerr << "<elistArgKind>";
        
    if (e->isA(event::eventGroupKind))
        rslerr << "<eventGroupKind>";
        
    if (e->isA(event::requestArgKind))
        rslerr << "<requestArgKind>";
        
    if (e->isA(event::controlKind))
        rslerr << "<controlKind>";
#endif

    if (e->isA(event::argKind))
    {
        Argument *ar = (Argument *) e;
//      DRWCString outev = ((ResArg *) ar)->ref.Name();
        DRWCString outev = ar->argName;

#ifdef RSLERR
        rslerr << "\t(ArgKind) `" << outev << "'\n";
#endif

        unsigned method = Resource::theIDHash(outev.data());
        
        switch(method)
        {
            case _hDISPLAY: // "Display"
                rsl_Display(ar);
                return 1;

            case _hCLOSE:   // "Close"
                rsl_Close(ar);
                return 1;
    
            case _hQUIT:    // "Quit"
                rsl_Quit(ar);
                return 1;
    
            case _hALERT:   // "Alert"
                rsl_Alert(ar);
                return 1;
    
            case _hERROR:   // "Error"
                rsl_Error(ar);
                return 1;
    
            case _hEXITSESSION: // "ExitSession"
                rsl_Error(ar);
                return 1;

            default:
                if (subTranslateOutEvent(method, ar))
                    return 1;
#ifdef RSLERR
                else
                    rslerr << "event type `" << outev << "' unimplemented.\n";
#endif
        }
    }

#ifdef RSLERR
    else
        rslerr << "event kind `" << e->kind << "' unsupported here.\n";
#endif

    return 0;
}

// subTranslateOutEvent
// Called by TranslateOutEvent for the purpose of extending the
// standard set of outgoing events... by a subclass, obviously,
// so by default it's unrecognized.
int rslServer::subTranslateOutEvent(unsigned method, Argument *ar)
{
    return 0;
}


void rslServer::SetLogLevel(int loglevel)
{
    ResReference ref = GetDataMember("LogLevel");
    // this should be changed to not have to create a new resource
    if (ref.isValid())
        ref()->Assign(R_Integer::New("", loglevel));
}


void rslServer::SetLogWhichSystems(RWCString systems)
{
    logf->SetSubSysMask(systems);
    ResReference ref = GetDataMember("LogWhichSystems");    // Server.rsl
    // this should be changed to not have to create a new resource
    if (ref.isValid())
        ref()->Assign(R_String::New("", systems));
}


void rslServer::SetUseFileLog(int bUseLog)
{
    ResReference ref = GetDataMember("UseFileLog");
    // this should be changed to not have to create a new resource
    if (ref.isValid())
        ref()->Assign(R_Boolean::New("", bUseLog));
}


void rslServer::SetLogFileName(RWCString strFileName)
{
    ResReference ref = GetDataMember("LogFileName");    // Server.rsl
    // this should be changed to not have to create a new resource
    if (ref.isValid())
        ref()->Assign(R_String::New("", strFileName));
}


void rslServer::SetMaxUsers(int m)
{
    ResReference ref = GetDataMember("MaxUsers");
    // this should be changed to not have to create a new resource
    if (ref.isValid())
        ref()->Assign(R_Integer::New("", m));
}

void rslServer::SetMaxUserMessage(RWCString newmessage)
{
    ResReference ref = GetDataMember("MaxUserMessage");
    // this should be changed to not have to create a new resource
    if (ref.isValid())
        ref()->Assign(R_String::New("", newmessage));
}

void rslServer::SetTotalSessionsAllowed(int iMaxSessions)
{
    ResReference ref = GetDataMember("TotalSessionsAllowed");
    // this should be changed to not have to create a new resource
    if (ref.isValid())
        ref()->Assign(R_Integer::New("", iMaxSessions));
}

void rslServer::SetTotalRequestsAllowed(int iMaxRequests)
{
    ResReference ref = GetDataMember("TotalRequestsAllowed");
    // this should be changed to not have to create a new resource
    if (ref.isValid())
        ref()->Assign(R_Integer::New("", iMaxRequests));
}

void rslServer::SetLogStatInterval(int seconds)
{
    ResReference ref = GetDataMember("LogStatInterval");
    // this should be changed to not have to create a new resource
    if (ref.isValid())
        ref()->Assign(R_Integer::New("", seconds));

    if (timeoutManager)
    {
        ResReference thisref(this);
        timeout_node *realtm = timeoutManager->FindNode(thisref);

        // update the node's max idle time, then Touch it to update it in the list.
        if (realtm)
        {
            realtm->max_idle = seconds;
            timeoutManager->tm.Touch(realtm);
        }
    }
}

// LogPeriodicInfo
// Public method, ok to be called at any time (within a logging period)
// if desired.
void rslServer::LogPeriodicInfo(void)
{
    ResReference ref = GetDataMember("LogStatInterval");    // Server.rsl
    float timeout_interval = (float) R_Integer::Int(ref());
    
    if (timeout_interval > 0)
    {
        // convert the log period from seconds to minutes
        float fMin;
        fMin = timeout_interval / 60;
        char sMin[40];  // "big" buffer. could be a static.
        sprintf(sMin, "%.1f", fMin);
    
        Logf.notice(LOGSERVER) << "*******************************************" << endline;
        Logf.notice(LOGSERVER) << "* Periodic: " << runtimeStuff.SessionsDoneThisPeriod()
            << " Completed in " << sMin << " min; "
            << runtimeStuff.CurrentUserSessions() << " now, "
            << runtimeStuff.MaxConcurrentUserSessions() << " top, "
            << runtimeStuff.TotalUserSessions() << " total." << endline;
        Logf.notice(LOGSERVER) << "*******************************************" << endline;
    }
}

// ***************************
// *** Resource stuff
// ***************************
ResStatus rslServer::execute(int method, ResList& arglist)
{
    Logf.debug(LOGSERVER) << "Invoking method in server base class `rslServer'" << endline;

    switch(method)
    {
        case _hINTERNAL_STARTSERVER:    // "Internal_StartServer"
            return rsl_Internal_StartServer(arglist);

        case _hINTERNAL_NEWCONNECTION:  // "Internal_NewConnection"
            return rsl_Internal_NewConnection(arglist);

        case _hPARSERSLFILE:    // "ParseRSLFile"
            return rsl_ParseRSLFile(arglist);

        case _hLOGPERIODICSTATISTICS:   // "LogPeriodicStatistics"
            return rsl_LogPeriodicStatistics(arglist);
            
        case _hSETLOGLEVEL: // "SetLogLevel"
            return rsl_SetLogLevel(arglist);
 
        case _hSETLOGWHICHSYSTEMS:  // "SetLogWhichSystems"
            return rsl_SetLogWhichSystems(arglist);
            
		case _hSETUSEFILELOG:	// "SetUseFileLog"
			return rsl_SetUseFileLog(arglist);

		case _hSETLOGFILENAME:	// "SetLogFileName"
			return rsl_SetLogFileName(arglist);

        case _hSETMAXUSERS: // "SetMaxUsers"
            return rsl_SetMaxUsers(arglist);
 
        case _hSETMAXUSERMESSAGE:   // "SetMaxUserMessage"
            return rsl_SetMaxUserMessage(arglist);
 
		case _hSETTOTALSESSIONSALLOWED:	// "SetTotalSessionsAllowed"
			return rsl_SetTotalSessionsAllowed(arglist);

		case _hSETTOTALREQUESTSALLOWED:	// "SetTotalRequestsAllowed"
			return rsl_SetTotalRequestsAllowed(arglist);

        case _hSETLOGSTATINTERVAL:  // "SetLogStatInterval"
            return rsl_SetLogStatInterval(arglist);
 
        case _hSHUTDOWNATZEROSESSIONS:  // "ShutdownAtZeroSessions"
            return rsl_ShutdownAtZeroSessions(arglist);
 
        case _hSHUTDOWN:    // "Shutdown"
            return rsl_Shutdown(arglist);

		case _hCANCELSHUTDOWN:	// "CancelShutdown"
			return rsl_CancelShutdown(arglist);

        default: ;
    }

    return ResStatus(ResStatus::rslFail);
}


// RSL method "Internal_StartServer"
ResStatus rslServer::rsl_Internal_StartServer(const ResList& arglist)
{
    ResReference ref = GetDataMember("Port");

    ListenLoop(R_Integer::IntFromResource(ref()));

    return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "Internal_NewConnection"
ResStatus rslServer::rsl_Internal_NewConnection(const ResList& arglist)
{
    return ResStatus(ResStatus::rslOk, NULL);
}


//---------------------------------------------


// RSL method "SetLogLevel"
//  SetLogLevel(Integer level);
ResStatus rslServer::rsl_SetLogLevel(const ResList& arglist)
{
    SetLogLevel(R_Integer::Int(arglist.get(0)));
    return ResStatus(ResStatus::rslOk, NULL);
}
 
// RSL method "SetLogWhichSystems"
//  SetLogWhichSystems(String logsys);
ResStatus rslServer::rsl_SetLogWhichSystems(const ResList& arglist)
{
    SetLogWhichSystems(arglist[0].StrValue());
    return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "SetUseFileLog"
ResStatus rslServer::rsl_SetUseFileLog(const ResList& arglist)
{
    SetUseFileLog(R_Boolean::Bool(arglist.get(0)));
	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "SetLogFileName"
ResStatus rslServer::rsl_SetLogFileName(const ResList& arglist)
{
    SetLogFileName(arglist[0].StrValue());
	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "SetMaxUsers"
//  SetMaxUsers(Integer maxusers);
ResStatus rslServer::rsl_SetMaxUsers(const ResList& arglist)
{
    SetMaxUsers(R_Integer::Int(arglist.get(0)));
    return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "SetMaxUserMessage"
//  SetMaxUserMessage(String message);
ResStatus rslServer::rsl_SetMaxUserMessage(const ResList& arglist)
{
    SetMaxUserMessage(arglist[0].StrValue());
    return ResStatus(ResStatus::rslOk, NULL);
}
 
// RSL method "SetTotalSessionsAllowed"
ResStatus rslServer::rsl_SetTotalSessionsAllowed(const ResList& arglist)
{
    SetTotalSessionsAllowed(R_Integer::Int(arglist.get(0)));
	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "SetTotalRequestsAllowed"
ResStatus rslServer::rsl_SetTotalRequestsAllowed(const ResList& arglist)
{
    SetTotalRequestsAllowed(R_Integer::Int(arglist.get(0)));
	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "SetLogStatInterval"
//  SetLogStatInterval(Integer seconds);
ResStatus rslServer::rsl_SetLogStatInterval(const ResList& arglist)
{
    SetLogStatInterval(R_Integer::Int(arglist.get(0)));
    return ResStatus(ResStatus::rslOk, NULL);
}
 
//---------------------------------------------

// RSL method "ShutdownAtZeroSessions"
// ShutdownAtZeroSessions(Boolean b);
ResStatus rslServer::rsl_ShutdownAtZeroSessions(const ResList& arglist)
{
    quitWhenZeroSessions = (R_Boolean::Bool(arglist.get(0)));
    return ResStatus(ResStatus::rslOk, NULL);
}
 
// RSL method "Shutdown"
// Shutdown(Integer seconds);
ResStatus rslServer::rsl_Shutdown(const ResList& arglist)
{
    shutdownTime = time(0) + R_Integer::Int(arglist.get(0));

    char *ati = asctime(localtime(&shutdownTime));

    if (ati)
        logf->alert(LOGSERVER) << "Server shutdown scheduled for " << ati << endline;
    
    if (shutdownTime == 0)
        quitWhenZeroSessions = TRUE;

    return ResStatus(ResStatus::rslOk, NULL);
}   

 
// RSL method "CancelShutdown"
ResStatus rslServer::rsl_CancelShutdown(const ResList& arglist)
{
    if (shutdownTime > 0)
        logf->alert(LOGSERVER) << "Cancel shutdown." << endline;
    else
        logf->info(LOGSERVER) << "CancelShutdown(): none to cancel." << endline;
    
    shutdownTime = 0;

    return ResStatus(ResStatus::rslOk, NULL);
}

ResStatus rslServer::rsl_LogPeriodicStatistics(const ResList& arglist)
{
    LogPeriodicInfo();

#ifdef RSLERR
	rslerr << "Calling LogPeriodicInfo here!!!\n";
#endif
	
    /*  note: if this method is called by the timeout manager,
        the timer is automatically reset. */

    return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "ParseRSLFile"
ResStatus rslServer::rsl_ParseRSLFile(const ResList& arglist)
{
RWCString fname = arglist[0].StrValue();

#ifdef RSLERR
    rslerr << "rslServer::ParseRSLFile() for `" << fname << "'...\n";
#endif

    ifstream in(fname.data());
    if (in)
    {
        lexer_context lexc; // a lexer

        // Set current lexer state
        lexc.SetSource(lexer_context::Stream, fname);
        lexc.SetLexerIO(in, cout);
        lexc.done = lexer_context::Ready;
        lexc.type = lexer_context::interactive;
        lexc.keystate = lexer_context::Everywhere;
    
        // print a first prompt (if applicable)
        if (lexc.type == lexer_context::prompt)
            lexc.Out() << "RSL> " << flush;
    
        int nerrors = parse_it(lexc);
        
        if (nerrors > 0)
            cerr << nerrors << " parser errors in file `" << fname << "'\n";
        else
        {
#ifdef RSLERR
            rslerr << "\n/* Done Parsing. */\n\n";
            rslerr << "// Linking methods..\n// ===================\n";
#endif

            nerrors += runtimeStuff.EvaluateIRParams();
            nerrors = runtimeStuff.LinkMethods();

#ifdef RSLERR
            rslerr << "\n// ===================\n";
#endif
        
            if (nerrors > 0)
                cerr << endl << nerrors << " link errors found.\n";
        }

    }
    else
        cerr << "ParseRSLFile: unable to find file `" << fname << "'.\n";

    return ResStatus(ResStatus::rslOk, NULL);
}
