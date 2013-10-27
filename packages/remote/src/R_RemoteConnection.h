// R_RemoteConnection.h
// $Id: R_RemoteConnection.h,v 1.1 1998/11/17 23:11:33 toddm Exp $
#include <iostream.h>
#include <rw/cstring.h>
#include "res_class.h"
#include "Resource.h"
#include "runtime.h"

#ifndef _R_RemoteConnection_H_
#define _R_RemoteConnection_H_

#define R_RemoteConnection_ID 1007100941

// ********************************************
// * rc_RemoteConnection -- the RemoteConnection RSL class
// ********************************************
class rc_RemoteConnection : public res_class {
	Resource *spawn(RWCString aname);
public:
	rc_RemoteConnection(RWCString aname) : res_class(aname)
	{
		ResClasses.insert((res_class *) this);
	}
};

// *************************************************
// * R_RemoteConnection -- the RemoteConnection Resource
// *************************************************
class R_RemoteConnection : public Resource {
	
	// private class data
	
public:
	static rc_RemoteConnection rslType;

// Constructors
	
	R_RemoteConnection(RWCString n);
	
// Resource virtuals

	// Info

	unsigned int TypeID() { return R_RemoteConnection_ID; }
	res_class *memberOf(void) { return &rslType; }
	
	// Execution
	
	ResStatus execute(int method, ResList& arglist);
	
	// output
	
// R_RemoteConnection specific
	
	static R_RemoteConnection *New(RWCString n);
	
// R_RemoteConnection RSL methods
	
	ResStatus rsl_connect(const ResList& arglist);
};

#endif



