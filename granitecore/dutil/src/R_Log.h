// R_Log.h
// $Id: R_Log.h,v 1.1 1998/11/17 23:48:23 toddm Exp $

#ifndef _R_LOG_H_
#define _R_LOG_H_

#include <iostream.h>
#include <rw/cstring.h>
#include "res_class.h"
#include "Resource.h"
//include "runtime.h"


#define R_Log_ID 5009255


// ********************************************
// * rc_Log -- the Log RSL class
// ********************************************
class rc_Log : public res_class {
	Resource *spawn(RWCString aname);
public:
	rc_Log(RWCString aname) : res_class(aname)
	{
	}
};

// *************************************************
// * R_Log -- the Log Resource
// *************************************************
class R_Log : public Resource {
	
	// private class data
	
public:
	static rc_Log rslType;

// Constructors
	
	R_Log(RWCString n);
	
// Resource virtuals

	// Info

	unsigned int TypeID() { return R_Log_ID; }
	res_class *memberOf(void) { return &rslType; }
	RWCString StrValue(void);
	int LogicalValue();
	int IsEqual(Resource *r);
	
	// Execution
	
	ResStatus execute(int method, ResList& arglist);
//	void SetFromInline(RWTPtrSlist<Resource>& inliner);
	void Assign(Resource *r);
	void Clear();
	
	// output
	
	void print(ostream &out=cout);	// ECI
	void rslprint(ostream &out=cout);

// R_Log specific
	
	static R_Log *New(RWCString n);
	void logArgs(const ResList& arglist);

// R_Log RSL methods
	
	ResStatus rsl_debug(const ResList& arglist);
	ResStatus rsl_info(const ResList& arglist);
	ResStatus rsl_notice(const ResList& arglist);
	ResStatus rsl_alert(const ResList& arglist);
	ResStatus rsl_error(const ResList& arglist);
	ResStatus rsl_fatal(const ResList& arglist);
	ResStatus rsl_emergency(const ResList& arglist);
};

#endif

