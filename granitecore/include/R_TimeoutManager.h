// R_TimeoutManager.h
// $Id: R_TimeoutManager.h,v 1.1 1998/11/17 23:48:29 toddm Exp $

#include <iostream.h>
#include <rw/cstring.h>
#include "res_class.h"
#include "Resource.h"
//#include "runtime.h"

#include "timeout_manager.h"

#ifndef _R_TIMEOUTMANAGER_H_
#define _R_TIMEOUTMANAGER_H_

#define R_TimeoutManager_ID 1056995407


// ********************************************
// * rc_TimeoutManager -- the TimeoutManager RSL class
// ********************************************
class rc_TimeoutManager : public res_class {
	Resource *spawn(RWCString aname);
public:
	rc_TimeoutManager(RWCString aname) : res_class(aname)
	{
//		ResClasses.insert((res_class *) this);
	}
};

// *************************************************
// * R_TimeoutManager -- the TimeoutManager Resource
// *************************************************
class R_TimeoutManager : public ResObj {
	
	// private class data
	
	RWTPtrHashSet<timeout_node> nodes;
	RWTValSlist<ResReference> removable;
	
public:
	static rc_TimeoutManager rslType;
	timeout_manager tm;

// Constructors
	
	R_TimeoutManager(RWCString n);
	
// Resource virtuals

	// Info

	unsigned int TypeID() { return R_TimeoutManager_ID; }
	res_class *memberOf(void) { return &rslType; }
	RWCString StrValue(void);
	int LogicalValue();
	
	// Execution
	
	ResStatus execute(int method, ResList& arglist);
	void Clear();
	
	// output
	
//	void print(ostream &out=cout);	// ECI
	void rslprint(ostream &out=cout);

// R_TimeoutManager specific
	
	static R_TimeoutManager *New(RWCString n);
	timeout_node *FindNode(ResReference ref);
	int timeoutReady();
	
// R_TimeoutManager RSL methods
	
	ResStatus rsl_Init(const ResList& arglist);
	ResStatus rsl_Register(const ResList& arglist);
	ResStatus rsl_Touch(const ResList& arglist);
	ResStatus rsl_Remove(const ResList& arglist);
};

#endif

