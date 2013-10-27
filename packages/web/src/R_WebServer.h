// *****************************************************************************
// *
// * Web Server Resource
// *
// * History: Create - TGM 3/24/97
// * 
// * $Id: R_WebServer.h,v 1.1 1998/11/17 23:30:32 toddm Exp $
// * 
// * $Log: R_WebServer.h,v $
// * Revision 1.1  1998/11/17 23:30:32  toddm
// * Initial revision
// *
// * Revision 2.11  1998/11/16 21:07:55  toddm
// * Update header for the method MergeTemplate
// *
// * Revision 2.9  1998/11/09 20:58:22  toddm
// * Fix R_LIST processing
// *
// * Revision 2.8  1998/06/29 19:47:08  toddm
// * Move all includes outside of exclusion symbol
// *
// * Revision 2.7  1998/05/29 22:07:09  holtrf
// * fixed bus error for sun.
// *
// * Revision 2.6  1998/05/13 23:33:55  toddm
// * Benchmark test logging
// *
// * Revision 2.5  1998/04/16 20:21:00  toddm
// * Add Request Counting
// *
// * Revision 2.4  1998/04/15 16:06:59  toddm
// * Modify rsl_Quit
// *
// * Revision 2.3  1998/04/06 17:28:35  prehmet
// * Integration with Registrar and Broker.
// *
// * Revision 2.2  1998/04/03 21:35:22  toddm
// * Add GetObject( Argument *ar)
// *
// * Revision 2.1  1998/02/13 21:34:01  toddm
// * Start work on splitting the Web Channel from the Granite Core
// *
// * Revision 1.9  1998/01/15 18:48:52  toddm
// * Add SetRSLConfig method
// *
// * Revision 1.8  1997/11/05 16:53:49  toddm
// * HTM extension support / Fix multiple cookie problem
// *
// * Revision 1.7  1997/09/24 23:59:42  toddm
// * Fix Memory Leaks
// *
// * Revision 1.6  1997/09/19 18:33:42  toddm
// * Fix session counting
// *
// * Revision 1.5  1997/09/17 22:44:38  toddm
// * Fix Garbage Character problem when using Internet Explorer
// *
// * Revision 1.4  1997/08/15 21:25:46  toddm
// * Add cookie support
// *
// * Revision 1.3  1997/07/17 18:17:34  toddm
// * Continue Developement
// *
// * Revision 1.2  1997/07/11 21:21:11  toddm
// * Second
// *
// * Revision 1.1  1997/06/03 18:28:30  toddm
// * Initial revision
// *
// * Copyright 1997 by Destiny Software Corporation.
// * 
// *******************************************************************************
// *******************
// * System Includes *
// *******************
#include <rw/ctoken.h>
#include <rw/tpslist.h>
#include <rw/tvslist.h>
#include <rw/regexp.h>

// ******************
// * Local Includes *
// ******************
#include "rslServer.h"
#include "res_class.h"
#include "drwcstring.h"
#include "StringPair.h"
#include "slog.h"
#include "ECI_Client.h"
#include "BrokerConnection.h"
#include "destiny.h"

#include "R_Server.h"
#include "R_TimeoutManager.h"

#ifndef _R_WebServer_H_
#define _R_WebServer_H_

#define R_WebServer_ID 1075254326

// ********************************************
// * rc_WebServer -- the WebServer RSL class
// ********************************************
class rc_WebServer : public res_class {
    Resource *spawn(RWCString aname);
public:
    rc_WebServer(RWCString aname) : res_class(aname)
    {
    }
};


// *************************************************
// * R_WebServer -- the WebServer Resource
// *************************************************
class R_WebServer : public rslServer {
    
    // ************************
    // * Private Data Members *
    // ************************
    iosockinet *pioClient;               // Client stream socket
    BrokerConnection *brkConn;           // Broker connection object

    inline iosockinet &ioClient(void) {return *pioClient;}

    RWCString strCurrentSession, startupClassName;
    RWCString strDocumentRoot;
    RWCString strDocumentExt;

    RWTValSlist<StringPair> IncomingEvents;     // List of Name/Value pairs of data
    RWCString strQuery;                         // The query string from the Http request

    RWTValSlist<StringPair> IncomingHeaders;    // List of Name/Value pairs of headers
    RWCString strHeaders;                       // The the http headers from the Http request

    int iFileLen;                               // Length in bytes of outgoing HTML
    RWTValSlist<RWCString> OutHtmlFile;         // List containing the outgoing HTML

	int iTotalRequests;							// Number of request handled
	int iTotalRequestsAllowed;					// Total number of requests handled before shutdown

    ECI_Client *pGraniteCore;                   // Pointer to the ECI_Clent object
    int iGCorePort;                             // Granite Core port number

    // ****************************
    // * Private Member Functions *
    // ****************************
    DRWCString UnescapeStr(DRWCString strData);
    int ParseCookies(RWCString strCookies);
    int ParseRequestHeaders(void);
    int ParseRequestData(void);
    void SetRSLConfig( );


protected:
    // **************************
    // * Protected Data Members *
    // **************************

    // **********************
    // * rslServer virtuals *
    // **********************
    virtual void ListenLoop(int portnum=0, int toFork=1);
    void NewConnection(iosockinet& sio);
    
    virtual void InitRequest(void);
    virtual void GetIncomingRequest();
    virtual int ValidRequest(void);
    virtual event *TranslateIncoming(void);
    virtual event *ExecuteIncomingEvents(event *e);
    virtual void FinalizeRequest(void);
    virtual int subTranslateOutEvent(unsigned method, Argument *ar);
    virtual void ShutdownNow();

    void ReadDynamicConfig();

    // *****************************************
    // * rslServer virtuals -- outgoing events *
    // *****************************************
    void rsl_Display(Argument *ar);
    void rsl_Quit(Argument *ar);
    void rsl_Alert(Argument *ar);

    void web_Stuff(Argument *ar);

    // ***************
    // * Misc stuff. *
    // ***************
    void GetHttpHeaders();
    void GetHttpRequest();

    void logEvents(RWTValSlist<StringPair> evts);

    event *CreateSessionWithData(RWTValSlist<ResReference> DataPairs);
    event *CreateSession(void);
    int CheckMaxSessions(void);
    RWCString GetNewSessionId(void);

    event *MethodRequest(RWCString strObjectName, RWCString strMethod);
    event *DataRequest(RWCString strObjectName, RWTValSlist<ResReference> DataPairs);

    void InitObject(RWCString strObject);
    void InitObjectDisplay(RWCString strObject);
    ResReference GetObject(RWCString strObject);
    ResReference GetObject(Argument *ar);

    void ReportMessage(RWCString strHtmlMsgFile, RWCString strMessage);
    void MergeTemplate(ResReference resDisplayObject);
    void RTextSub(DRWCString& strHtmlLine, ResReference resCurrentObj);
    void RListSub(RWTValSlist<DRWCString> &RListBlock, ResReference resCurrentObj,
                            ResReference resList);
    void RCListSub(RWTValSlist<DRWCString> &RListBlock, ResReference resCurrentObj,
                            ResReference resList);
    int VariableSub(DRWCString& strHtmlLine, ResReference refListLine, int doCount);
    int LineCallSub(DRWCString& strHtmlLine, ResReference refListLine);

    // Setup
    void SwitchToFileLog(void);

public:
    // ***********************
    // * Public Data Members *
    // ***********************

	static rc_WebServer *rslType;

    // ****************
    // * Constructors *
    // ****************
    R_WebServer(RWCString n);
    
    // *********************
    // * Resource virtuals *
    // *********************
    unsigned int TypeID() { return R_WebServer_ID; }
    inline res_class *memberOf(void) { return R_WebServer::rslType; }
    ResStatus execute(int method, ResList& arglist);
    
    // *****************************************
    // * R_WebServer specific member functions *
    // *****************************************
    static R_WebServer *New(RWCString n);
};

#endif




