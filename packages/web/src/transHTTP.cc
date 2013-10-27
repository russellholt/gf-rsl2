// ***************************************************************************
// *
// *  NAME:  transHTTP.cc
// *
// *  RESOURCE NAME:    WebServer                                        
// *                                                                    
// *  RELATED RESOURCES: 
// *                                                                    
// *  DESCRIPTION: 
// *                                                                    
// * $Id: transHTTP.cc,v 1.2 1998/12/22 20:04:54 toddm Exp $
// *
// * $Log: transHTTP.cc,v $
// * Revision 1.2  1998/12/22 20:04:54  toddm
// * Fix event cleanup
// *
// * Revision 1.1  1998/11/17 23:30:17  toddm
// * Initial revision
// *
// * Revision 2.11  1998/11/12 21:30:28  toddm
// * Add new subsystems
// *
// * Revision 2.9  1998/11/09 20:58:30  toddm
// * Fix R_LIST processing
// *
// * Revision 2.8  1998/05/13 23:34:04  toddm
// * Benchmark test logging
// *
// * Revision 2.7  1998/05/08 17:17:45  toddm
// * Fix ExecuteIncomingEvents to handle multiple returns
// *
// * Revision 2.5  1998/04/16 20:21:10  toddm
// * Add Request Counting
// *
// * Revision 2.4  1998/04/15 16:07:28  toddm
// * Modify rsl_Quit
// *
// * Revision 2.3  1998/04/06 17:28:38  prehmet
// * Integration with Registrar and Broker.
// *
// * Revision 2.2  1998/04/03 21:35:54  toddm
// * Change WebChannel to handle objects coming back from Core
// *
// * Revision 2.1  1998/02/13 21:34:11  toddm
// * Start work on splitting the Web Channel from the Granite Core
// *
// * Revision 1.10  1998/01/16 16:04:42  toddm
// * Test TotalAppsAllowed for 0
// *
// * Revision 1.9  1998/01/15 18:51:16  toddm
// * Put purify code in for memory testing
// *
// * Revision 1.8  1997/11/05 16:53:57  toddm
// * HTM extension support / Fix multiple cookie problem
// *
// * Revision 1.7  1997/09/24 23:59:49  toddm
// * Fix Memory Leaks
// *
// * Revision 1.6  1997/09/22 15:49:26  toddm
// * Fix session counting
// *
// *
// * Copyright (c) 1995, 1996, 1997 by Destiny Software Corporation
// *
// ***************************************************************************

// *******************
// * System Includes *
// *******************
#include <fstream.h>
#include <stream.h>
#include <sys/times.h>

// ******************
// * Local Includes *
// ******************
#include "rw_utils.h"
#include "R_WebServer.h"
#include "R_String.h"
#include "R_Integer.h"
#include "R_Table.h"
#include "PerfLog.h"

#include "killevents.h"

#ifdef PURIFYTEST
#include "/sw/apps/purify/purify.h"
#endif

#define _hHTML  1213484364  // HTML
#define _hOFX   5195352     // OFX
#define _hQIF   5327174     // QIF

// *********************************************************************
// *                                                                    
// * Function: logEvents                                             
// *                                                                    
// * Description:   This function simply logs the HTTP name/value pairs 
// *                                                  
// * Inputs: evts - A list containing the Name/Value pairs received from the
// *                client.
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: Nothing
// *                                                                    
// *********************************************************************
void R_WebServer::logEvents(RWTValSlist<StringPair> evts)
{   
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In logEvents" << endline;

    if (evts.entries() > 0)
    {
        RWTValSlistIterator<StringPair> iter(evts);
        while(iter())
        {
            logf->info(LOGWEBCHANNEL) << "(R_WebServer) " << iter.key().left()
                                  << ", " << iter.key().right() << endline;
        }
    }
}


// *********************************************************************
// *                                                                    
// * Function: TranslateIncoming                                             
// *                                                                    
// * Description:   This function loops through the list of incoming
// *                HTTP events and converts them to RSL2 events.
// *                                                  
// * Inputs: None                  
// *                                                                    
// * Outputs: None                                                          
// *                b                                                    
// * Returns: Pointer to a list of RSL2 events.
// *                                                                    
// *********************************************************************
event *R_WebServer::TranslateIncoming(void)
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In TranslateIncoming" << endline;

    // ************************************************
    // * If we do not have any Name/Value pairs then  *
    // * we can assume that this is a new session.    *
    // ************************************************
    if (IncomingEvents.isEmpty())
    {
        logf->notice(LOGWEBCHANNEL) << "(R_WebServer) Creating New Session." << endline;
        return (CreateSession());
    }

    logEvents(IncomingEvents);

    RWTValSlistIterator<StringPair> iterIncomingEvents(IncomingEvents);
    StringPair nv;
    DRWCString strName, strValue;

    RWCString strObjID;

    // *****************************************
    // * Used to store the list of input data. *
    // *****************************************
    RWTValSlist<ResReference> DataPairs;
    RWTValSlist<RWCString> Methods;

    strCurrentSession = "";

    // ****************************************************
    // * Loop through the RWTValSlist of Name/Value Pairs *
    // * Creating RSL2 events for each                    *
    // ****************************************************
    while (iterIncomingEvents())
    {
        nv = iterIncomingEvents.key();

        strName = nv.left();
        strValue = nv.right();

        // ***********************************
        // * Check for the session variables *
        // ***********************************
        if (toUpper(strName) == "_APPID")            //  Session id
            strCurrentSession = strValue;
        else
        if (toUpper(strName) == "_OBJID")            //  form id number
            strObjID = strValue;
        else

        // ******************************************************
        // * Does this pair represent an RSL Method? If so we   *
        // * will need to call MethodRequest for the method.    *
        // * BUT we can't call MethodRequest yet because        *
        // * we may not have the ObjID or AppID yet so we'll    *
        // * put each method found into a list then call        *
        // * MethodRequest for each method after we have        *
        // * processed all incoming Events.                     *
        // ******************************************************
        if (strName.contains("_Method:"))
        {
            DRWCString strMethod = strName.after("_Method:");

            // ***************************************
            // * Images give a X & Y coordinate.     *
            // * We only need one of them so we will *
            // * ignore the .x coordinate.           *
            // ***************************************
            if (strMethod.contains(".y"))
            {
                strMethod = strMethod.before(".y");
            }
            else if (strMethod.contains(".x"))
            {
                continue;
            }

            Methods.append( strMethod );
        }
        
        // ******************************************** 
        // * If its not a session variable or a event *
        // * then it is an input data pair            *
        // ******************************************** 
        else
        {
            ResReference refDataPair(R_String::New(strName, strValue));
            DataPairs.append( refDataPair );
        }
    }

    // ********************************************
    // * If we didn't find a Session Id           *
    // * Then we have to create a new session     *
    // *                                          *
    // * This is the case where we need to create *
    // * a session and there is data pairs        *
    // ********************************************
    if (strCurrentSession.length() == 0)
    {
        logf->notice(LOGWEBCHANNEL) << "(R_WebServer) Creating New Session with data." << endline;

        // ********************************************************
        // * When we split the harness we will call CreateSession *
        // * instead of CreateSessionWithData                     *
        // ********************************************************
        return (CreateSessionWithData(DataPairs));
    }

    // **********************************************
    // * Create a list of Audit Requests to execute *
    // **********************************************
    elist *eAuditRequestList = (elist *) Remember (new elist);

    // *****************************************************************
    // * If there are any DataPairs, we need to create an AuditRequest *
    // * to do the assignment of the data to the resource within the   *
    // * form.  We need to do this *before* we send the AuditRequests  *
    // * for the methods so that the data members are already set when *
    // * the method is called (so it can use the new values!)          *
    // *****************************************************************
    event *eDataRequest = DataRequest(strObjID, DataPairs);
    if (eDataRequest)
        eAuditRequestList->add(eDataRequest);

    // *****************************************************
    // * Now we have to loop through the list of methods   *
    // * and call MethodRequest for each method in the     *
    // * list. This will create an Audit Request for each  *
    // * method.                                           *
    // *****************************************************
    RWTValSlistIterator<RWCString> iterMethods(Methods);
    RWCString strThisMethod;

    while (iterMethods())
    {
        strThisMethod = iterMethods.key();

        event *eMethodRequest = MethodRequest(strObjID, strThisMethod);
        if (eMethodRequest)
            eAuditRequestList->add(eMethodRequest);
    }

    return eAuditRequestList;
}


// *********************************************************************
// *                                                                    
// * Function: CreateSessionWithData
// *                                                                    
// * Description:   This function creates a session by calling CreateSession
// *                Then it adds simple parameters to the Web_Startup mehtod
// *                                                  
// * Inputs: DataPairs - A list of Strings containing the Name (ref.Name())
// *                     and Value (ref.StrValue())
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: A valid AuditRequest if Successfull
// *          else NULL
// *                                                                    
// *********************************************************************
event *R_WebServer::CreateSessionWithData(RWTValSlist<ResReference> DataPairs)
{

    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In CreateSessionWithData" << endline;
    
    event *eSessionRequest = CreateSession();
    if (eSessionRequest)
    {
        // **********************************************************
        // * Loop through all the data pairs and send each          *
        // * as a command line argument to the "Web_Startup"        *
        // * method                                                 *
        // *                                                        *
        // * When we split the harness we will have to create       *
        // * an audit request for assignment of datapairs in        *
        // * SetFromDataPairs.  When this happens then we will      *
        // * not have to send the data as command line args to      *
        // * "Web_Startup".  We will just add the audit request     *
        // * to the AuditRequestList after the Web_Startup request. *
        // **********************************************************
        RWTValSlistIterator<ResReference> iterDataPairs(DataPairs);
        ResReference resDataPair;

        R_Table *tblParameters = R_Table::New("tblParameters");
        ResContext &rcParameters = tblParameters->GetLocalContext();

        while(iterDataPairs())
        {
            resDataPair = iterDataPairs.key();
            
            if (!resDataPair.isValid())
            {
                logf->notice(LOGWEBCHANNEL) << "(R_WebServer) Invalid data pair reference!" << endline;
                continue;
            }

            logf->debug(LOGWEBCHANNEL) << "(R_WebServer) Considering Http input (" 
                                   << (resDataPair.Name())
                                   << "," << (resDataPair.StrValue()) << ")" << endline;
            
            rcParameters.AddResource(resDataPair());            
            
        }

        // ********************************************************************
        // * Add the table of params as an argument to the Web_Startup method *
        // ********************************************************************
        ((AuditRequest *) eSessionRequest)->arguments = (elistArg *) Remember (new elistArg);
        ((AuditRequest *) eSessionRequest)->arguments->add((ResArg *) Remember (new ResArg(tblParameters)));

        return(eSessionRequest);
    }
    else
    {
        return(NULL);
    }
}


// *********************************************************************
// *                                                                    
// * Function: CreateSession                                             
// *                                                                    
// * Description:   This function creates a session by executing an 
// *                AuditRequest for the Web_Startup method. 
// *                                                  
// * Inputs: None                  
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: A valid AuditRequest if Successfull
// *          else NULL
// *                                                                    
// *********************************************************************
event *R_WebServer::CreateSession(void)
{

    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In CreateSession" << endline;

    // *********************************
    // * Get a new session number here *
    // *********************************
    strCurrentSession = GetNewSessionId();

    // *********************************************
    // * Create an AuditRequest of the following   *
    // *                                           *
    // * `startup'#`session'.Web_Startup#StartSession(); *
    // *********************************************
    AuditRequest *eSessionRequest = (AuditRequest *) Remember (new AuditRequest(
                        (char *) startupClassName.data(),
                        (char *) strCurrentSession.data(), 
                        "Web_Startup", 
                        "CreateSession"));

    logf->info(LOGWEBCHANNEL) << "(R_WebServer) Call Web_Startup\n" 
                          << eSessionRequest << endline;


    return(eSessionRequest);
}


// *********************************************************************
// *                                                                    
// * Function: GetNewSessionId                                             
// *                                                                    
// * Description:   This function generates a new session id.
// *                                                  
// * Inputs: None                  
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns:
// *        RWCString - New session id.
// *                                                                    
// *********************************************************************
RWCString R_WebServer::GetNewSessionId(void)
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In GetNewSessionId" << endline;

    RWCString strNewSessID;

#ifdef NOBROKER
    
    int procID = getpid();

    struct tms junk;
    clock_t TimeNow = times( &junk );

    strNewSessID = dec(procID);
    strNewSessID += dec(TimeNow);

#else
    if (brkConn != NULL)
    {
        // ***********************
        // * Performance logging *
        // ***********************
        PerfLogger tmCreateSession;
        tmCreateSession.StartTime();

        strNewSessID = brkConn->createSession();

        // ***********************
        // * Performance logging *
        // ***********************
        tmCreateSession.EndTime();
        tmCreateSession.ReportPerf("Create Session", logf);
    }
#endif

    if (strNewSessID.isNull())
    {
        logf->error(LOGWEBCHANNEL)  << "(R_WebServer) Error creating new session id!" << endline;
    }
    else
    {
        logf->notice(LOGWEBCHANNEL) << "(R_WebServer) New session ID is: " 
                                << strNewSessID << endline;
    }

    return strNewSessID;
}


// *********************************************************************
// *                                                                    
// * Function: MethodRequest                                             
// *                                                                    
// * Description:   This function creates an AuditRequest for the mehtod, 
// *                session, and object Id specifed.
// *                                                  
// * Inputs: strObjectName - String containing the Object Id.
// *         strMethod - String containing the Method to execute.
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: Pointer to the new AuditRequest.
// *                                                                    
// *********************************************************************
event *R_WebServer::MethodRequest(RWCString strObjectName, RWCString strMethod)
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In MethodRequest" << endline;

    if (strMethod.length() > 0)
    {
        AuditRequest *eMethodRequest = (AuditRequest *) Remember (new AuditRequest(
                    (char *) strObjectName.data(),      // object: "app" or "form"
                    (char *) strCurrentSession.data(),  // sessionID
                    (char *) strMethod.data(),          // method
                    (char *) "MethodRequest"));                // unused

        logf->info(LOGWEBCHANNEL) << "(R_WebServer) Translate request:\n" 
                              << eMethodRequest << endline;
    
        return eMethodRequest;
    }
    
    return NULL;
}


// *********************************************************************
// *                                                                    
// * Function: DataRequest
// *                                                                    
// * Description:   This function maps elements in the DataPairs list 
// *                to data member objects to the destination Resource 
// *                DataPairs in a list of Resources (ResReference).  
// *
// * Inputs: strObject - String containing the Object Id.
// *         DataPairs - A list of Strings containing the Name (ref.Name())
// *                     and Value (ref.StrValue())
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: Event - returns a pointer to the created audit request.
// *                                                                    
// *********************************************************************
event *R_WebServer::DataRequest(RWCString strObjectName,
                                        RWTValSlist<ResReference> DataPairs)
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In DataRequest" << endline;

    // ******************************************************
    // * Create a form object                               *
    // * first try to create the type of destination object *
    // ******************************************************
    res_class *rc = NULL;

    res_class findrc("_transfer_");
    rc = ResClasses.find(&findrc);

    if (!rc)
    {
        logf->notice(LOGWEBCHANNEL) << "(R_WebServer) Can't find class transfer" 
                                << endline;
        return(NULL);
    }

    // **********************************
    // * Create a new transfer resource *
    // **********************************
    Resource *transferRes = rc->New("transfer");
    if (!transferRes)
    {
        logf->notice(LOGWEBCHANNEL) << "(R_WebServer) Unable to create transfer form object!" 
                                << endline;
        return(NULL);
        
    }

    // ********************************************************
    // * Is the transfer resource a resStructure or resObject *
    // ********************************************************
    if (transferRes->InternalType() != Resource::resStructType &&
        transferRes->InternalType() != Resource::resObjType)
    {
        logf->notice(LOGWEBCHANNEL) << "(R_WebServer) Transfer form object is not streamable!"
                                << endline;
        return(NULL);
    }

    ResContext &transContext = ((ResStructure *) transferRes)->GetLocalContext();

    // *****************************************************************
    // * Iterate through the `DataPairs' list.                         *
    // * Each Resource in the list (ResReference, actually) has        *
    // * a name [Name()] and a value [StrValue()]; these correspond    *
    // * to the resource and the value of the {input ,radio button,    *
    // * check box} *                                                  *
    // *****************************************************************
    RWTValSlistIterator<ResReference> iterDataPairs(DataPairs);
    ResReference resDataPair;

    while(iterDataPairs())
    {
        resDataPair = iterDataPairs.key();
        
        if (!resDataPair.isValid())
        {
            logf->notice(LOGWEBCHANNEL) << "(R_WebServer) Invalid data pair reference!" << endline;
            continue;
        }

        logf->debug(LOGWEBCHANNEL) << "(R_WebServer) Considering Http input (" 
                               << (resDataPair.Name())
                               << "," << (resDataPair.StrValue()) << ")" << endline;

        // *************************************************************
        // * Add the data pair to the Context of the transfer resource *
        // *************************************************************
        transContext.AddResource(resDataPair());
    }
    
    // ***********************************************
    // * Create an audit request to assign the given *
    // * transfer resource to the data member within *
    // * the specified object.                       *
    // ***********************************************
    AuditRequest *arAssign = (AuditRequest *) Remember (new AuditRequest(
                    (char *) strObjectName.data(),      // object: "app" or "form"
                    (char *) strCurrentSession.data(),  // sessionID
                    (char *) "_assign",                 // method.
                    (char *) "DataRequest"));                 // unused

    // ****************************************************************
    // * Add the transfer object as an argument to the _assign method *
    // ****************************************************************
    arAssign->arguments = (elistArg *) Remember (new elistArg);
    arAssign->arguments->add((ResArg *) Remember (new ResArg(transferRes)));

    logf->info(LOGWEBCHANNEL) << "(R_WebServer) Assign\n"
                          << arAssign << endline;

    return(arAssign);
}


// *********************************************************************
// *                                                                    
// * Function: ExecuteIncomingEvents                                             
// *                                                                    
// * Description:   This function simply execute the event or event list. 
// *
// *                This will become a client for an R_ECI_Server when
// *                we split the harness and server into two processes.
// *                                                  
// * Inputs: e - Either a list or a single RSL2 event
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: An event containing the result of the execute.
// *                                                                    
// *********************************************************************
event *R_WebServer::ExecuteIncomingEvents(event *e)
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In ExecuteIncomingEvents" << endline;

    if (!e)
        return NULL;
    
    logf->info(LOGWEBCHANNEL) << "(R_WebServer) Incoming events are:\n" 
                          << e << endline;
    
    if (pGraniteCore == NULL)
    {
        logf->error(LOGWEBCHANNEL) << "(R_WebServer) Error connecting to granite core!" << endline;
        return NULL;
    }

#ifndef NOBROKER
    // *********************************************************
    // * Listen on a specified port for a connection from the  *
    // * granite core, then initiate a dialogue with the core. *
    // *********************************************************

    logf->notice(LOGWEBCHANNEL) << "(R_WebServer) Requesting granite core connection." << endline;

    // ***********************
    // * Performance logging *
    // ***********************
    PerfLogger tmCoreConnect;
    tmCoreConnect.StartTime();

    // *******************************************************
    // * Request connection to Granite Core, then listen for *
    // * the core to connect back.                           *
    // *******************************************************
    if ((brkConn == NULL) ||
        (brkConn->connectToGCP (strCurrentSession) != 0))
    {
        logf->error(LOGWEBCHANNEL) << "(R_WebServer) Error connecting to granite core!" << endline;
        return NULL;
    }

    pGraniteCore->Accept();

    // ***********************
    // * Performance logging *
    // ***********************
    tmCoreConnect.EndTime();
    tmCoreConnect.ReportPerf("Core Connect", logf);

    logf->notice(LOGWEBCHANNEL) << "(R_WebServer) Granite core connected back." << endline;

#endif

    event *eresult = NULL;
//    RWTPtrSlist<event> eresult_list;
    elist *eresult_list = NULL;

    // ***********************
    // * Performance logging *
    // ***********************
    PerfLogger tmCoreRequest;
    tmCoreRequest.StartTime();

    // **********************************************
    // * This elistKind check will go away when the *
    // * harness is process-split.                  *
    // **********************************************
    if (e->isA(event::elistKind))
    {
        elist *eResultsList = (elist *) Remember (new elist);

        RWTPtrSlistIterator<event> iter(((elist *) e)->evtl);
        event *evt=NULL;
        while(iter())
        {
            evt = iter.key();

            logf->info(LOGWEBCHANNEL) << "(R_WebServer) Incoming events (elist entry) EXECUTING `\n"
                                  << evt << endline;

            eresult = pGraniteCore->Execute(evt);   // should be an AuditRequest.

            // ***************************************************
            // * save "intermediate" results by inserting into a *
            // * list. the tail is returned.                     *
            // ***************************************************
            if (eresult)
                eResultsList->add(eresult);
//                eresult_list.append(eresult);
        }

        // ***********************
        // * Performance logging *
        // ***********************
        tmCoreRequest.EndTime();
        tmCoreRequest.ReportPerf("Granite Core Process Time (multi request)", logf);

        return(eResultsList);
    }
    else
    {
        logf->info(LOGWEBCHANNEL) << "(R_WebServer) Incoming events (single event) EXECUTING `\n"
                              << e << endline;

        eresult = pGraniteCore->Execute(e);   // should be an AuditRequest.

        // ***********************
        // * Performance logging *
        // ***********************
        tmCoreRequest.EndTime();
        tmCoreRequest.ReportPerf("Granite Core Process Time (single request)", logf);

        return(eresult);
    }

    // *********************************************************
    // * only the *last* result is returned!                   *
    // * if the last result was not null, then it was inserted *
    // * into the list, so it needs to be removed so the rest  *
    // * of them can be deleted (they're not used).            *
    // *********************************************************
/*
    if (eresult && !eresult_list.isEmpty())
        eresult_list.removeLast();  // last one

    // ****************************
    // * Delete any other events. *
    // ****************************
    while(!eresult_list.isEmpty())
    {
        event *tde = eresult_list.get();
        delete tde;
    }

    return eresult;
*/
}


// *********************************************************************
// *                                                                    
// * Function: subTranslateOutEvent                                             
// *                                                                    
// * Description:   This function is called from rslServer::TranslateOutEvent
// *                for outgoing events that are not in the basic set of 
// *                Display, Close, Quit, Alert, Error.
// *
// * Inputs:    method -    a hash of the event name, exactly like those created
// *                        by rsl2c++, eg, produced by Resource::theIDHash() on the name of
// *                        the argument.
// *            ar - event containing the actual outgoing event.
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: Nothing
// *                                                                    
// *********************************************************************
int R_WebServer::subTranslateOutEvent(unsigned method, Argument *ar)
{
#ifdef RSLERR
    rslerr << "(R_WebServer) In subTranslateOutEvent: ";
    ar->print(rslerr);
    rslerr << ": ";
#endif

    switch(method)
    {
        case _hHTML:    // "HTML"
        case _hOFX:    // "OFX"
        case _hQIF:    // "QIF"
            web_Stuff(ar);
            return 1;

        default:
            break;
    }
    
    logf->error(LOGWEBCHANNEL) << "TranslateOutEvent: method not implemented." << endline;
    
    return 0;
}

// *********************************************************************
// *                                                                    
// * Function: rsl_Display                                             
// *                                                                    
// * Description:   This function is used to display a form or some object.
// *                Either the form Resource has been given to us, or we
// *                have to go and get it via ECI.
// *
// *                For now, I'm assuming that the argument is just a String
// *                which names the object to be displayed, rather than the
// *                object (inline resource) itself.
// *
// * Inputs: ar - Argument containing the name of the object to display.
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: Nothing
// *                                                                    
// *********************************************************************
void R_WebServer::rsl_Display(Argument *ar)
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In rsl_Display" << endline;
    
    // ***********************
    // * Performance logging *
    // ***********************
    PerfLogger tmDisplay;
    tmDisplay.StartTime();

    if (ar == NULL)
    {
        logf->notice(LOGWEBCHANNEL) << "(R_WebServer) rsl_Display for event: (NULL)"
                                << endline;
        return;
    }

    logf->info(LOGWEBCHANNEL) << "(R_WebServer)  rsl_Display for event:\n" << ar << endline;
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) Event Kind is - " << ar->kind << endline;

    ResReference resReturnedObject;

    // *****************************************************
    // * This handles the case where the return object is **
    // * ResArg.  An example of this:                     **
    // * Display: "A_Form";                               **
    // *****************************************************
    if (ar->isA(event::resArgKind))
    {
        ResReference resObjectName = ((ResArg *) ar)->ref;
        if (resObjectName.TypeID() == R_String_ID)    // for now
        {
            RWCString strObjectName = resObjectName.StrValue();

            // *************************************************************
            // * execute displayInit() method to initialize for display    *
            // *************************************************************
            InitObjectDisplay(strObjectName);

            // *****************************
            // * Get the requested object. *
            // *****************************
            resReturnedObject = GetObject(strObjectName);
        }
        else
            logf->alert(LOGWEBCHANNEL) << "(R_WebServer) Object specified in the Display statement "
                                   << " is not a String resource!" << endline;
    }

    // *****************************************************
    // * This handles the case where the return object is **
    // * ListArg.  An example of this:                    **
    // * Display: A_Form;                                 **
    // *****************************************************
    else if (ar->isA(event::argListKind))
    {
        // *****************************
        // * Get the requested object. *
        // *****************************
        resReturnedObject = GetObject(ar);
    }
    else
        logf->alert(LOGWEBCHANNEL) << "(R_WebServer) This event is unimplemented"  << endline;


    MergeTemplate(resReturnedObject);

    // ***********************
    // * Performance logging *
    // ***********************
    tmDisplay.EndTime();
    tmDisplay.ReportPerf("rsl_Display Processing", logf);
}


// *********************************************************************
// *                                                                    
// * Function: rsl_Quit
// *                                                                    
// * Description:   This function is used to quit an RSL Session.
// *
// * Inputs: None
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: Nothing
// *                                                                    
// *********************************************************************
void R_WebServer::rsl_Quit(Argument *ar)
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In rsl_Quit" << endline;
    
    // *********************************************
    // * Create an AuditRequest of the following   *
    // *                                           *
    // * `startup'#`session'.Kill#Quit(); *
    // *********************************************
    AuditRequest *arSessionRequest = new AuditRequest(
                        (char *) startupClassName.data(),
                        (char *) strCurrentSession.data(), 
                        (char *) AR_QuitOnly_Method, 
                        (char *) "Quit");


    logf->info(LOGWEBCHANNEL) << "(R_WebServer) Quit Session: `\n"
                              << arSessionRequest << endline;

    // *********************************************
    // * Send the AuditRequest to the Granite Core *
    // *********************************************
    event *requ_results = pGraniteCore->Execute(arSessionRequest);
    delete arSessionRequest;

    if (requ_results)
    {
        logf->info(LOGWEBCHANNEL) << "(R_WebServer) Results from Quit():\n`"
                              << requ_results << endline;

//        delete requ_results;
    }
    
// #ifdef PURIFYTEST                                                          
//     if (purify_is_running())                                               
//     {                                                                      
//         purify_printf("Session `%s' killed.\n", strCurrentSession.data()); 
//         purify_new_leaks();                                                
//     }                                                                      
// #endif                                                                     
}


// *********************************************************************
// *                                                                    
// * Function: rsl_Alert
// *                                                                    
// * Description:   This function is used to display an alert in response
// *                to a alert cal in rsl.
// *
// * Inputs: ar - Argument containing the message to display.
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: Nothing
// *                                                                    
// *********************************************************************
void R_WebServer::rsl_Alert(Argument *ar)
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In rsl_Alert" << endline;
    
    if (ar == NULL)
    {
        logf->notice(LOGWEBCHANNEL) << "(R_WebServer) No alert message specified."
                                << endline;
        return;
    }

    logf->info(LOGWEBCHANNEL) << "(R_WebServer)  rsl_Alert for event:\n" << ar << endline;

    if (ar->isA(event::resArgKind))
    {
        ResReference resAlertMessage = ((ResArg *) ar)->ref;
        if (resAlertMessage.TypeID() == R_String_ID)    // for now
        {
            RWCString strAlertMessage = resAlertMessage.StrValue();
            ReportMessage("alert", strAlertMessage);
        }
    }
}


// *********************************************************************
// *                                                                    
// * Function: web_Html                                             
// *                                                                    
// * Description:   This function is used to display a form or some object.
// *                Either the form Resource has been given to us, or we
// *                have to go and get it via ECI.
// *
// *                For now, I'm assuming that the argument is just a String
// *                which names the object to be displayed, rather than the
// *                object (inline resource) itself.
// *
// * Inputs: ar - Argument containing the name of the object to display.
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: Nothing
// *                                                                    
// *********************************************************************
void R_WebServer::web_Stuff(Argument *ar)
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In web_Stuff" << endline;
    
    if (ar == NULL)
    {
        logf->notice(LOGWEBCHANNEL) << "(R_WebServer) web_Stuff for event: (NULL)"
                                << endline;
        return;
    }

    logf->info(LOGWEBCHANNEL) << "(R_WebServer)  web_Stuff for event:\n" << ar << endline;

    if (ar->isA(event::resArgKind))
    {
        ResReference resHtmlMessage = ((ResArg *) ar)->ref;
        if (resHtmlMessage.TypeID() == R_String_ID)    // for now
        {
            RWCString strHtmlMessage = resHtmlMessage.StrValue();

            iFileLen = strHtmlMessage.length() + 1;
            OutHtmlFile.append(strHtmlMessage);
        }
    }
}


// *********************************************************************
// *                                                                    
// * Function: InitObject
// *                                                                    
// * Description:   This function is used to call the constructor (Init)
// *                of the object specified.
// *
// * Inputs: strObject - String containing the object to call init on
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: ResReference for the resource created.
// *                                                                    
// *********************************************************************
void R_WebServer::InitObject(RWCString strObject)
{
    if (strObject.length() == 0)
        return;

    AuditRequest *arInit = new AuditRequest(
                    (char *) strObject.data(),
                    (char *) strCurrentSession.data(), 
                    (char *) "Init", 
                    (char *) "InitObject");


    logf->info(LOGWEBCHANNEL) << "(R_WebServer) Initializing Object: `\n"
                              << arInit << endline;

    // *********************************************
    // * Send the AuditRequest to the Granite Core *
    // *********************************************
    event *requ_results = pGraniteCore->Execute(arInit);
    delete arInit;

    if (requ_results)
    {
        logf->info(LOGWEBCHANNEL) << "(R_WebServer) Results from Init():\n`"
                              << requ_results << endline;

//        delete requ_results;
    }
}


// *********************************************************************
// *                                                                    
// * Function: InitObjectDisplay
// *                                                                    
// * Description:   This function is used to call the display init method
// *                of the object specified.
// *
// * Inputs: strObject - String containing the object to call displayinin on
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: ResReference for the resource created.
// *                                                                    
// *********************************************************************
void R_WebServer::InitObjectDisplay(RWCString strObject)
{
    if (strObject.length() == 0)
        return;

    AuditRequest *arInitDisplay = new AuditRequest(
                    (char *) strObject.data(),
                    (char *) strCurrentSession.data(), 
                    (char *) "displayInit", 
                    (char *) "InitObjectDisplay");


    logf->info(LOGWEBCHANNEL) << "(R_WebServer) Initializing Display: `\n"
                              << arInitDisplay << endline;

    // *********************************************
    // * Send the AuditRequest to the Granite Core *
    // *********************************************
    event *requ_results = pGraniteCore->Execute(arInitDisplay);
    delete arInitDisplay;

    if (requ_results)
    {
        logf->info(LOGWEBCHANNEL) << "(R_WebServer) Results from DisplayInit():\n`"
                              << requ_results << endline;

//        delete requ_results;
    }
}


// *********************************************************************
// *                                                                    
// * Function: GetObject                                             
// *                                                                    
// * Description:   This function is used to get
// *                the object specified.
// *
// * Inputs: strObject - String containing the object to create
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: ResReference for the resource created.
// *                                                                    
// *********************************************************************
ResReference R_WebServer::GetObject(RWCString strObject)
{
    ResReference resReturnFail;

    if (strObject.length() == 0)
        return (resReturnFail);

    AuditRequest *arGet = new AuditRequest(
                    (char *) strObject.data(),
                    (char *) strCurrentSession.data(),  // sessionID
                    (char *) "_get",                    // get the object.
                    (char *) "GetObject");              // unused

    logf->info(LOGWEBCHANNEL) << "(R_WebServer) Getting Object: `\n"
                              << arGet << endline;

    // *********************************************
    // * Send the AuditRequest to the Granite Core *
    // *********************************************
    event *requ_results = pGraniteCore->Execute(arGet);
    delete arGet;

    if (requ_results)
    {

        logf->info(LOGWEBCHANNEL) << "(R_WebServer) Results from _get():\n`"
                              << requ_results << endline;


        ResReference resCreatedObject;
        if (requ_results->isA(event::resArgKind))
            resCreatedObject = ((ResArg *) requ_results)->ref;
        else if(requ_results->isA(event::elistKind))
        {
            RWTPtrSlistIterator<event> iter(((elist *) requ_results)->evtl);
            event *e=NULL;

            iter();
            e = iter.key();

            if (e->isA(event::argListKind))
            {
                event *eReturn = e->execute(NULL);
                resCreatedObject = ((ResArg *) eReturn)->ref;
            }
        }

        
        logf->debug(LOGWEBCHANNEL) << "(R_WebServer) Object: " << resCreatedObject.StrValue() << endline;

        if (resCreatedObject.isValid())
        {
            if ((resCreatedObject()->InternalType() == Resource::resStructType) ||
                (resCreatedObject()->InternalType() == Resource::resObjType))
            {
                return(resCreatedObject);
            }
        }
    }

    return (resReturnFail);
}

// *********************************************************************
// *                                                                    
// * Function: GetObject                                             
// *                                                                    
// * Description:   This function is used to get
// *                the object specified.
// *
// * Inputs: ar - The returned Argument 
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: ResReference for the resource created.
// *                                                                    
// *********************************************************************
ResReference R_WebServer::GetObject(Argument *ar)
{
    ResReference resCreatedObject;

    if (ar->isA(event::argListKind))
    {
        event *eReturn = ar->execute(NULL);
        resCreatedObject = ((ResArg *) eReturn)->ref;
    }

    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) Object: " << resCreatedObject.StrValue() << endline;

    if (resCreatedObject.isValid())
    {
        if ((resCreatedObject()->InternalType() == Resource::resStructType) ||
            (resCreatedObject()->InternalType() == Resource::resObjType))
        {
            return(resCreatedObject);
        }
    }

    ResReference resReturnFail;
    return (resReturnFail);
}
