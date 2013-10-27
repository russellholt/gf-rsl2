// rsl2_server
// Abstract server Resource
// ***************************************************************************
// *
// *  NAME:  rslServer.h
// *
// *  RELATED RESOURCES: 
// *                                                                    
// *  DESCRIPTION:                                                      
// *     Abstract server resource class
// *                                                                    
// * $Id: rslServer.h,v 1.1 1998/11/17 23:56:09 toddm Exp $
// *
// * $Log: rslServer.h,v $
// * Revision 1.1  1998/11/17 23:56:09  toddm
// * Initial revision
// *
// * Revision 2.3  1998/06/29 19:34:25  toddm
// * Move all includes outside of exclusion symbol
// *
// * Revision 2.2  1998/04/30 19:15:15  toddm
// * Remove sessionsDoneThisPeriod
// *
// *
// * Copyright (c) 1996, 1997, 1998 by Destiny Software Corporation
// *
// ***************************************************************************

// *******************
// * System Includes *
// *******************
#include <iostream.h>
#include <Fork.h>
#include <sockinet.h>
#include <rw/cstring.h>

// ******************
// * Local Includes *
// ******************
#include "Resource.h"
#include "b.h"
#include "rslEvents.h"
#include "R_TimeoutManager.h"

#ifndef RSL_SERVER_H_
#define RSL_SERVER_H_

class rslServer : public ResObj {

protected:
	rslServer(RWCString nm, res_class *subclass_class);

	bool quitWhenZeroSessions;
    
	time_t shutdownTime;
	
	R_TimeoutManager *timeoutManager;

public:
	virtual void ListenLoop(int portnum=0, int toFork=1);
	virtual void NewConnection(iostream& in);
	virtual int RoundTripRequest(void);
	virtual void Outbound(event *fromRSL);

	virtual void InitRequest();
	virtual int ValidRequest()=0;
	virtual void FinalizeRequest();
	
	virtual void GetIncomingRequest()=0;
	virtual event *TranslateIncoming(void)=0;


	virtual void TranslateOutLoop(event *evt);
	virtual int TranslateOutEvent(event *e);
	virtual int subTranslateOutEvent(unsigned method, Argument *ar);

	virtual void ShutdownNow();
	virtual void LogPeriodicInfo();

	virtual event *ExecuteIncomingEvents(event *e)=0;
	
	// C++ Reconfiguration interface -- called by ECI interface.
	// to be subclassed where appropriate.
    virtual void SetLogLevel(int loglevel);
    virtual void SetLogWhichSystems(RWCString systems);
    virtual void SetUseFileLog(int bUseLog);
    virtual void SetLogFileName(RWCString strFileName);
	virtual void SetMaxUsers(int m);
	virtual void SetMaxUserMessage(RWCString newmessage);
    virtual void SetTotalSessionsAllowed(int iMaxSessions);
    virtual void SetTotalRequestsAllowed(int iMaxRequests);
    virtual void SetLogStatInterval(int seconds);
	
	// outgoing event methods
	
	virtual void rsl_Display(Argument *ar) { }
	virtual void rsl_Close(Argument *ar) { }
	virtual void rsl_Quit(Argument *ar) { }
	virtual void rsl_Alert(Argument *ar) { }
	virtual void rsl_Error(Argument *ar) { }

	ResStatus execute(int method, ResList& arglist);

// RSL methods ----------------------------

	ResStatus rsl_StartServer(const ResList& arglist);
	ResStatus rsl_NewConnection(const ResList& arglist);
	ResStatus rsl_Internal_StartServer(const ResList& arglist);
	ResStatus rsl_Internal_NewConnection(const ResList& arglist);
	ResStatus rsl_ParseRSLFile(const ResList& arglist);
    ResStatus rsl_ShutdownAtZeroSessions(const ResList& arglist);
    ResStatus rsl_Shutdown(const ResList& arglist);
	ResStatus rsl_CancelShutdown(const ResList& arglist);
	ResStatus rsl_LogPeriodicStatistics(const ResList& arglist);
	
	// ECI reconfig interface (RSL side)
    ResStatus rsl_SetLogLevel(const ResList& arglist);
    ResStatus rsl_SetLogWhichSystems(const ResList& arglist);
	ResStatus rsl_SetUseFileLog(const ResList& arglist);
	ResStatus rsl_SetLogFileName(const ResList& arglist);

    ResStatus rsl_SetMaxUsers(const ResList& arglist);
    ResStatus rsl_SetMaxUserMessage(const ResList& arglist);

	ResStatus rsl_SetTotalSessionsAllowed(const ResList& arglist);
	ResStatus rsl_SetTotalRequestsAllowed(const ResList& arglist);

    ResStatus rsl_SetLogStatInterval(const ResList& arglist);
};

#endif



