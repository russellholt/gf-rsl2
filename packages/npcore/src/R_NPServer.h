// *****************************************************************************
// *
// * R_NPServer.h
// *
// * Granite Core Server Resource
// *
// * $Id: R_NPServer.h,v 1.1 1998/11/17 23:08:49 toddm Exp $
// * 
// * $Log: R_NPServer.h,v $
// * Revision 1.1  1998/11/17 23:08:49  toddm
// * Initial revision
// *
// * Revision 2.4  1998/06/29 18:38:28  toddm
// * Move all includes outside of exclusion symbol
// *
// * Revision 2.3  1998/05/11 22:30:19  holtrf
// * making non blocking read on the fifo
// *
// * Revision 2.2  1998/05/01 14:05:16  toddm
// * Add NewConnection to NPServer
// *
// *
// * Copyright (c) 1998 by Destiny Software Corporation
// *
// ******************************************************************
// *******************
// * System Includes *
// *******************
#include <iostream.h>

// ******************
// * Local Includes *
// ******************
#include "res_class.h"
#include "runtime.h"
#include "slog.h"
#include "destiny.h"

#include "R_Server.h"

#ifndef _R_NPServer_H_
#define _R_NPServer_H_

#define R_NPServer_ID 1009137175


// ********************************************
// * rc_NPServer -- the NPServer RSL class
// ********************************************
class rc_NPServer : public res_class {
	Resource *spawn(RWCString aname);
public:
	rc_NPServer(RWCString aname) : res_class(aname)
	{
		ResClasses.insert((res_class *) this);
	}
};

// *************************************************
// * R_NPServer -- the NPServer Resource
// *************************************************
class R_NPServer : public R_Server {
	
	// private class data
	
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
	static rc_NPServer rslType;

    // ****************
    // * Constructors *
    // ****************
	R_NPServer(RWCString n);
	
    // *********************
    // * Resource virtuals *
    // *********************
	unsigned int TypeID() { return R_NPServer_ID; }
	res_class *memberOf(void) { return &rslType; }
	ResStatus execute(int method, ResList& arglist);
	
    // ****************************************
    // * R_GCServer specific member functions *
    // ****************************************
	static R_NPServer *New(RWCString n);

    // **********************
    // * rslServer virtuals *
    // **********************
    void ListenLoop (RWCString fifo);
    void NewConnection(isockstream& in);
	virtual void setupParserContext(istream& in);

    // **************************
    // * R_NPServer RSL methods *
    // **************************
	ResStatus rsl_Internal_StartServer(const ResList& arglist);
};

#endif



