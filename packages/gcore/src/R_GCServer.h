// *****************************************************************************
// *
// * R_GCServer.h
// *
// * Granite Core Server Resource
// *
// * History: Create - TGM 3/25/98
// * 
// * $Id: R_GCServer.h,v 1.1 1998/11/17 23:06:24 toddm Exp $
// * 
// * $Log: R_GCServer.h,v $
// * Revision 1.1  1998/11/17 23:06:24  toddm
// * Initial revision
// *
// * Revision 2.2  1998/05/01 14:03:47  toddm
// * Check Implement TotalSessionAllowed
// *
// * Revision 2.1  1998/03/27 21:19:25  toddm
// * Check out to update version number to 2.x
// *
// * Revision 1.1  1998/03/27 19:03:21  toddm
// * Initial revision
// *
// * Copyright (c) 1998 by Destiny Software Corporation
// *
// ******************************************************************
#ifndef _R_GCServer_H_
#define _R_GCServer_H_

// *******************
// * System Includes *
// *******************

// ******************
// * Local Includes *
// ******************
#include "res_class.h"
#include "runtime.h"
#include "slog.h"
#include "destiny.h"

#include "R_Server.h"

#define R_GCServer_ID 892679703


// ********************************************
// * rc_GCServer -- the GCServer RSL class
// ********************************************
class rc_GCServer : public res_class {
	Resource *spawn(RWCString aname);
public:
	rc_GCServer(RWCString aname) : res_class(aname)
	{	}
};

// *************************************************
// * R_GCServer -- the GCServer Resource
// *************************************************
class R_GCServer : public R_Server {
	
    // ****************************
    // * Private Member Functions *
    // ****************************
	void SetRSLConfig( );
	void SwitchToFileLog(void);
	
protected:
    // **************************
    // * Protected Data Members *
    // **************************

public:
    // ***********************
    // * Public Data Members *
    // ***********************
	static rc_GCServer *rslType;

    // ****************
    // * Constructors *
    // ****************
	R_GCServer(RWCString n);
	
    // *********************
    // * Resource virtuals *
    // *********************
	unsigned int TypeID() { return R_GCServer_ID; }
	res_class *memberOf(void) { return rslType; }
	ResStatus execute(int method, ResList& arglist);
	
    // ****************************************
    // * R_GCServer specific member functions *
    // ****************************************
	static R_GCServer *New(RWCString n);

    // **********************
    // * rslServer virtuals *
    // **********************
    void NewConnection(iosockinet& sio);
};

#endif

