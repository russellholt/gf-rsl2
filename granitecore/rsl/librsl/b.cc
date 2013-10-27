// b.cc
// Event subclasses. see also rslEvent.(cc|h)
// $Id: b.cc,v 1.4 1999/01/22 20:47:01 toddm Exp $
#include <fstream.h>

#include "b.h"
#include "runtime.h"
#include "rsldefaults.h"
#include "R_String.h"
#include "R_Status.h"
#include "slog.h"
#include "destiny.h"

#include "killevents.h"

extern ofstream rslerr; // rslmain.cc

extern ofstream *statsFile, *freelistFile;  // runtime.cc
extern void freelistStats(ostream &out);    // runtime.cc
extern int eciCommands; // runtime.cc
extern bool Running;    // runtime.cc

int eventsCreated=0, eventsDestroyed=0, programEvents=0, nonProgramEvents=0;

// RWTPtrSlist<event> ResultsAccumulator;

// *************************************************************
// *             +++++      event CLASS       +++++            *
// *************************************************************
event::event(void)
{
    kind=noKind;
    eventsCreated++;

    if (!Running)
        programEvents++;

    if (Running)
        nonProgramEvents++;
}

event::~event()
{
    eventsDestroyed++;
}

eventGroup::eventGroup(event *e)
{
    kind = eventGroupKind;
    ev = e;
}

void eventGroup::print(ostream& out)
{
    out << '(';
    if (ev)
        ev->print(out);
    out << ')';
}

// *************************************************************
// *             +++++      elist CLASS       +++++            *
// *************************************************************
elist::elist(void)
{
    kind = elistKind;
    use_own_context = FALSE;
}

elist::~elist()
{
//  rslerr << "BEGIN elist::~elist() for :";
//  print(rslerr);
//  rslerr << '\n';
//
//  if (!isA(programCodeKind))
//      while (!evtl.isEmpty())
//          delete (evtl.get());

}

// hack. yes. I know. can't change header right now...
// RFH 9/23/1997
void elist___kill(event *& el)
{
    if (!el)
        return;

    elist *el_list = NULL;
    event *ee = NULL;

    if (el->isA(event::elistKind))
    {
        el_list = (elist *) el;

        while (!el_list->evtl.isEmpty())
        {
            // safety mechanism:
            // if the event is part of the program, then
            // just drop it. others are pointing to it.
            // otherwise, delete it.
            ee = el_list->evtl.get();

            if (!(ee->isA(event::programCodeKind)))
                //delete ee;
				Remember(ee);
        }
    }

    //delete el;
	Remember(el);
    el = NULL;
}


void elist::print(ostream& out)
{
    RWTPtrSlistIterator<event> ei(evtl);
    event *e=NULL;
    while(++ei == TRUE)
    {
        e = ei.key();
        if (e)
        {
            e->print(out);
            out << ";\n";
        }
    }
}

void elist::add(event *e)
{
    if (e)
    {
        if (e->isA(localDeclKind))
            use_own_context = TRUE;

        evtl.append(e);
    }
}

void elist::transferContentsFrom(elist *evts)
{
    if (!evts)
        return;

    while (!evts->evtl.isEmpty())
        add(evts->evtl.get());  // remove from evts and add
}

// elist::execute()
// Process a list of events, which could be:
// - incoming requests
// - the statements of a method
event *elist::execute(ResContext *context)
{
RWTPtrSlistIterator<event> iter(evtl);
event *e=NULL, *retev=NULL;
elist *returnList=NULL;

#ifdef RSLERR
    rslerr << "BEGIN elist::execute()\n";
#endif

    while (iter())
    {
        e = iter.key();
        if (!e)
            continue;

#ifdef RSLERR
        rslerr << "elist::execute() for\n\t";
        e->print(rslerr);
        rslerr << endl;
#endif

        
        // Check for control -- just pass on if found.
        if (e->isA(controlReqKind))
        {
            if ( ((controlRequest *) e)->hasFlag(controlRequest::crReturn))
            {
#ifdef RSLERR
                rslerr << "elist::execute--RETURN: pass 1:\n"
                    << "\tCreate new controlRequest..\n";
#endif

                controlRequest *cR = new controlRequest( *((controlRequest *) e) );
                
#ifdef RSLERR
                rslerr << "\texecute controlRequest..\n";
#endif

                event *retE = e->execute(context);


                if (retE && retE->isA(elistKind))
                {
                    cR->arguments = (elist *) retE;

                    // clear e's arguments if we're going to use their
                    // execution results (and if they're not the same!)
//                  if ( cR->arguments != ((Request *) e))
//                      elist___kill((event *) (((Request *) e)->arguments));
                }
                else
                {
//                  elist___kill((event *) (((Request *) e)->arguments));
//                  cR->arguments = NULL;
                }

#ifdef RSLERR
                rslerr << "\tset argsResolved flag...\n";
#endif
                cR->addFlag(controlRequest::argsResolved);
                
#ifdef RSLERR
                rslerr << "\treturn the new controlRequest..\n";
#endif
                return cR;
            }
        }
        else
        if (e->isA(controlKind))
        {
#ifdef RSLERR
            rslerr << "elist::execute -- break/continue found, returning.\n";
            return e;
#endif
        }

        // do it
        retev = e->execute(context);
        
        // Check for control again. If we have a control request,
        // we must again just pass it on up. An example of this particular case
        // is a return inside an if statement, where the above e->execute() runs the
        // `if' statement itself (and thus one branch of the if) and contains a
        // `return', which it propogates here, so we must interrupt and return it up.
        // This passing on will end when it reaches rslMethod::execute, which is a
        // consumer of control requests.

#ifdef RSLERR
        rslerr << "elist: about to check return in pass 2..\n";     
#endif
        if (retev)
        {
            if (retev->isA(controlReqKind))
            {
                if ( ((controlRequest *) retev)->hasFlag(controlRequest::crReturn))
                {
#ifdef RSLERR
                    rslerr << "elist::execute--RETURN: pass 2: just return it.\n";
#endif
                    return retev;
                }
#ifdef RSLERR
                else
                    rslerr << "Unknown control request, flags: "
                        <<  ((controlRequest *) retev)->CRFlags() << endl;
#endif
            }
            else
            if (e->isA(controlKind))
            {
#ifdef RSLERR
                rslerr << "elist::execute -- break/continue (pass 2).\n";
#endif
                return e;
            }
            
#ifdef RSLERR
            else
                rslerr << "Not a controlReqKind -- flags are: " << retev->kind << "\n";
#endif

            // retev could be a return value to be appended
            // OR a result to be discarded.

            CheckReturnEvent(e, retev, returnList, context);
        }

    }
#ifdef RSLERR
    rslerr << "END elist::execute()\n";
#endif

    // NOTE: returnList is always NULL since passive "return" disabled in
    // CheckReturnEvent() below. Will be made more permanent when return-control
    // is worked out. (10 Feb 1997)
    return returnList;
}

// CheckReturnEvent
// Analyze the return value from event execution.
// If it is a control event (controlRequest, that is), add
// the results to the outgoing list (returnList).
// However, if it is a regular Resource (ResArg) or R_Status objects
// whose value is zero (indicating all is ok), it will be deleted.
// R_Status objects with non-zero values, however, will be passed
// to an exception handler.
void elist::CheckReturnEvent(event *e, event *retev, elist *& returnList, ResContext *context)
{
    if (e == retev)
    {
#ifdef RSLERR
        rslerr << "CheckReturnEvent: e == retev (skipping).\n";
#endif
        return;
    }
        
    if (retev)
    {
        // * check ret for ControlEvent ..
        //    * propogate breaks and continues
        // * add return values to outgoing argument list

#ifdef RSLERR
        rslerr << "CheckReturnEvent: got return event: `";
        retev->print(rslerr);
        rslerr << "', kind " << (e->kind) << '\n';
#endif

// ***********************************************************************************
// * NOTE:
// * passive "return" (adding elements to the outgoing argument list instead of
// *    flow control) is disabled for now. Instead, similar functionality is moved
// *    up into the owners of the elist, eg rslMethod, ifRequest, etc.
// ***********************************************************************************
//
//      if ((e->kind) & (event::controlReqKind))
//      {
//          rslerr << "\t(control request)\n";
//          
//          controlRequest *creq = (controlRequest *) e;
//          
//          if (creq->crKind == crOutput || creq->crKind == crReturn)
//          {
//              if (!returnList)
//                  returnList = new elistArg;
//                  
//              // If the result of a controlRequest execution is
//              // an elist, want to add its *contents* to the
//              // list of return values, not the list itself.
//              if (retev && ((retev->kind) & event::elistKind))
//              {
//                  returnList->transferContentsFrom((elist *) retev);
//                  rslerr << "elist::CheckReturnEvent(): DELETE a ControlRequest elist.\n" << flush;
//                  delete retev;
//              }
//              else
//                  returnList->add(retev);
//              
//              // Furthermore, "return" is an action.
//              if (creq->crKind == crReturn)
//          }
//          else
//              cerr << "RSL internal error: unknown control request, type "
//                  << creq->crKind << endl;
//          
//      }
//      else
// ***********************************************************************************
        if ((retev->kind) & (event::elistKind))
        {
#ifdef RSLERR
            rslerr << "\tFound elistKind.. checking for resArg\n";
#endif
            elist *thelist = ((elist *) retev);
            if (thelist->entries() > 0)
            {
                event *contents = (thelist->evtl.first());
                if ( (contents->kind) & event::resArgKind)
                {
#ifdef RSLERR
                    rslerr << "\tfound resArgKind..\n";
                    // Check for a Status!
#endif
                    ResReference& ref = ((ResArg *) contents)->ref;

#ifdef RSLERR
                    rslerr << "\tResArg is : `"; contents->print(rslerr);
                    rslerr << "', resource is: `"; ref.print(rslerr);
                    rslerr << "', class `" << ref.ClassName() << "'\n";
#endif

                    // ****************************************************
                    // "exception handling"
                    // got a status returned. if the severity is > 0,
                    // then execute a request to self with the same
                    // method name, and send this status object as the
                    // argument. sort of cool. this really needs to be
                    // extended to allow the setting of different method
                    // name, and possibly a different object. IE, a decent
                    // way to do that will be when the session is itself
                    // a class instead of just a ResContext. Then we could
                    // have session-level "exception" handling methods.
                    // ****************************************************
                    if (ref.TypeID() == R_Status_ID)
                    {
#ifdef RSLERR
                        rslerr << "\tFound status..\n";
#endif
                        if (  ((R_Status *) ref())->Severity() > 0)
                        {
                            elist eee;
                            eee.add(contents);
                            Request req((char *) "self", context->Name().data(), &eee);
                            CheckReturnEvent(&req, req.RExecute(context, 0), returnList, context);
                        }
                    }
					/* obsoleted by Remember()
                    else    // save the whole list.
                        ResultsAccumulator.insert(retev);
					*/
                }
            }
            else    // empty elist returned
            {
				// obsoleted by Remember()
                //delete retev;

                retev = NULL;
//              ResultsAccumulator.insert(retev);
            }
        }
#ifndef RSLERR
// Commented out TGM - 12/21/98
//        else
            //obsoleted by Remember()
//            ResultsAccumulator.insert(retev);
#else
        else
        {
//          ResultsAccumulator.insert(retev);
            ////rslerr << "Got non-control return event: ";
            ////retev->print(rslerr);
            ////rslerr << "\t(ignoring for now. delete?)\n";
            
			//obsoleted by Remember()
            //delete retev;
            retev = NULL;
        }
#endif
    }

}

// *************************************************************
// *       +++++        ObjRequest CLASS         +++++         *
// *************************************************************
objRequest::objRequest(const char* what)
{
    kind=objRequestKind;
    object=what;

    // Special case! request for "self", that is, the owner of the 
    // context in which this request is executed.
    if (object == "self")
        kind |= event::selfReqKind;
}

// objRequest::Resolve
// Find an object by name in the given context
ResStatus objRequest::Resolve(ResContext *context)
{
    if (context==NULL)
    {
#ifdef RSLERR
        rslerr << "objRequest::Resolve() - null context\n";
#endif
        return ResStatus(ResStatus::rslFail);
    }


    // Request for "self"
    if (kind & event::selfReqKind)
    {
#ifdef RSLERR
        rslerr << "objRequest::Resolve() -- request for SELF found\n";
#endif

        Resource *cown = context->Owner();
        if (cown)
            return ResStatus(ResStatus::rslOk, cown);
#ifdef RSLERR
        else
            rslerr << "\tno context owner..\n";
#endif
    }



    return (context->Find(object));
}

// *************************************************************
// *       +++++      AuditRequest CLASS       +++++           *
// *************************************************************
AuditRequest::AuditRequest(char* o, char* oid, char* m, char* mid)
    : Request(o, m), object_id(oid), method_id(mid)
{ }

AuditRequest::~AuditRequest()
{
}

void AuditRequest::print(ostream& out)
{
    out << object << '#' << object_id << '.';
    out << method << '#' << method_id << "( ";
    if (arguments)
        arguments->print(out);
    out << " );" << flush;
}

// *********************************************************************
// *                                                                    
// * Method: AuditRequest::execute
// *                                                                    
// * Description:   This is the entrance to the system (from ECI).
// *                We find the context that is indicated by the object_id,
// *                and call Request::Resolve inside that context. In this case,
// *                the context argument isn't used because we don't have a context yet.
// *                                                  
// * Inputs: context - ResContext specifying the session context.  In this case
// *                   the contex argument isn't used because we don't have a session yet.
// *                                                                    
// * Outputs: None
// *                                                                    
// * Returns: A pointer to the a event
// *          else NULL
// *                                                                    
// *********************************************************************
event *AuditRequest::execute(ResContext *context)
{
    logf->debug(LOGSERVER) << "(AuditRequest::execute)" << endline;

    // ***************************
    // * statistics (runtime.cc) *
    // ***************************
    eciCommands++;

    // ***********************************************
    // * Look for the specified session (object_id) **
    // ***********************************************
    ResContext *prcSession;
    ResReference refSession = runtimeStuff.FindSessionRef(object_id);

    // ******************************************
    // * if the session dosen't already exist  **
    // * then we must create a new one if it's **
    // * not in the DeadSession list           **
    // ******************************************
    if (!refSession.isValid())
    {
        Logf.info(LOGSERVER) << "(AuditRequest::execute) No session context `" << object_id 
                                << "', checking Dead Sessions." << endline;

        // ***********************************************************
        // * Check the DeadSession list.  If the session exists here *
        // * then send the outgoing events list back and remove      *
        // * the session from the DeadSession list.                  *
        // *                                                         *
        // * This is possible if we got a timeout that killed the    *
        // * session.                                                *
        // ***********************************************************
        elist *pevtQueue = runtimeStuff.GetDeadSessionOutQ(object_id);
        if (pevtQueue) 
        {
            Logf.debug(LOGSERVER) << "(AuditRequest::execute) Found DeadSession '" << object_id 
                                 << "' containing " << pevtQueue << endline;

//            runtimeStuff.KillDeadSession(object_id);
            return(pevtQueue);
        }

        // ****************************************
        // * Ok so the session has never existed  *
        // ****************************************
        Logf.info(LOGSERVER) << "(AuditRequest::execute) No session context `" << object_id 
                                << "', creating new." << endline;

        // ***************************
        // * Create the new session **
        // ***************************
        refSession = runtimeStuff.AddNewSession(object_id, object);
        if (!refSession.isValid())
        {
            return(NULL);
        }

        // ********************************************
        // * Get the local context for the specified **
        // * session resource.  This will be the     **
        // * session context.                        **
        // ********************************************
        ResContext &rcSessionLocals = ((ResStructure *) refSession())->GetLocalContext();
        prcSession = &rcSessionLocals;
    }

    // ******************************************
    // * The session already exists so lets try *
    // * to find the object within this session *
    // ******************************************
    else
    {
        Logf.info(LOGSERVER) << "(AuditRequest::execute) Found session `" 
                             << object_id  << "'" << endline;

        // ********************************************
        // * Get the local context for the specified **
        // * session resource.  This will be the     **
        // * session context.                        **
        // ********************************************
        ResContext &rcSessionLocals = ((ResStructure *) refSession())->GetLocalContext();
        prcSession = &rcSessionLocals;

        // *********************************************************
        // * It is possible that the object we are looking for     *
        // * is the session object itself.  If this is the case    *
        // * then we've found the object.  If not then we have to  *
        // * find the object within the session.                   *
        // *********************************************************
        if (object != refSession.ClassName())
        {
            // **************************************************************
            // * if the session already exists then look for the object in  *
            // * this session.  If the object doesn't exist create it.      *
            // **************************************************************
            Logf.info(LOGSERVER) << "(AuditRequest::execute) Looking for '" << object  
                                 << "' In session '" << object_id << "'" << endline;

            ResStatus stat = prcSession->Find(object);
            
            // *******************************************************************
            // * if it doesn't exist, then create it, so this works the same way *
            // * as when the requested session doesn't exist.                    *
            // *******************************************************************
            if (stat.status == ResStatus::rslFail)
            {
                logf->error(LOGSERVER) << "(AuditRequest::execute) Object '" << object 
                                       << "' does not exist in session '" << object_id << "'" << endline;

                return NULL;
            }
        }
        else
        {
            Logf.debug(LOGSERVER) << "(AuditRequest::execute) - The session with the name '" 
                                  << prcSession->Name() << "' is the object we are looking for" << endline;
        }
    }

    // *******************************************************************
    // * Sometimes we just want to create a new session and new object,  *
    // * and we don't have anything for it to do yet (except whatever is *
    // * in its constructor)                                             *
    // *                                                                 *
    // * AR_CreateOnly_Method is defined as Init.  Since Init is the rsl *
    // * constructor method that is called implicitly when an object is  *
    // * created, We should never execute this mehod explictly.          *
    // *******************************************************************
    if (method == AR_CreateOnly_Method)
    {
        return(NULL);
    }

    // *********************************
    // * This is explicit quit session *
    // *********************************
    else if (method == AR_QuitOnly_Method)
    {
        // ********************
        // * Kill the session *
        // ********************
        runtimeStuff.KillSession(object_id);

        // ****************************************
        // * Check the Dead Sessions to see if   **
        // * there is an events in the out queue **
        // ****************************************
        elist *pevtQueue = runtimeStuff.GetDeadSessionOutQ(object_id);
        if (pevtQueue) 
        {
            Logf.debug(LOGSERVER) << "(AuditRequest::execute) Found DeadSession '" << object_id 
                                 << "' containing " << pevtQueue << endline;

            runtimeStuff.KillDeadSession(object_id);
            return(pevtQueue);
        }

        return(NULL);
    }

    // ******************************************************
    // * If the object we are looking is the session object *
    // * then we will treat this as a special case          *
    // ******************************************************
    event *eReturn=NULL;
    if (object == refSession.ClassName())
    {
        ResList rl(arguments? (arguments->entries()) : 0);
        ResolveArguments(prcSession, rl);

        eReturn = executeResource(refSession(), rl, prcSession, 1); 
    }
    else
    {
        eReturn = Request::execute(prcSession);
    }

    // *******************************************************
    // * Since in the cource of this AuditRequest the       **
    // * session could have been killed, we must first      **
    // * look to see if the session still exists.  If       **
    // * it still exists then try to get the OutQ from      **
    // * the session.  If the session doesn't exist anymore **
    // * then we must try to get the OutQ from the Dead     **
    // * Sessions contexts.                                 **
    // *******************************************************
    elist *pevtQueue = NULL;
        
    // ***********************************************
    // * Look for the specified session (object_id) **
    // ***********************************************
    refSession = runtimeStuff.FindSessionRef(object_id);
    if (refSession.isValid())
        pevtQueue = runtimeStuff.GetSessionOutQ(object_id);
//    else
//        pevtQueue = runtimeStuff.GetDeadSessionOutQ(object_id);

    // ****************************
    // * Collect some statistics **
    // ****************************
    if (statsFile || freelistFile)
    {
        if (statsFile)
            runtimeStuff.printStats(*statsFile);
        if (freelistFile)
            freelistStats(*freelistFile);
    }

    // *****************************************
    // * if there is an Outgoing event list   **
    // * then we have to merge these events   **
    // * with the returned events for this    **
    // * AuditRequest                         **
    // *                                      **
    // * Append the AuditRequest events to    **
    // * the end of the Outgoing event list   **
    // *****************************************
    if (pevtQueue)
    {
        if (eReturn)
        {
            if (eReturn->isA(elistKind))
                ((elist *) pevtQueue)->transferContentsFrom((elist *) eReturn);
            else
                ((elist *) pevtQueue)->add(eReturn);
        }

        return(pevtQueue);
    }

    return(eReturn);
}