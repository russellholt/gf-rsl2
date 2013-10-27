// ******************************************************************
// R_RSL_System.h
//
// automatically generated from RSL_System.rsl and the template:
// $Id: R_RSL_System.h,v 1.2 1998/12/15 16:23:44 toddm Exp $
// ******************************************************************
// *******************
// * System Includes *
// *******************
#include <iostream.h>
#include <rw/cstring.h>

// ******************
// * Local Includes *
// ******************
#include "res_class.h"
#include "Resource.h"
#include "runtime.h"

#ifndef _R_RSL_System_H_
#define _R_RSL_System_H_

#define R_RSL_System_ID 1682390827

// ********************************************
// * rc_RSL_System -- the RSL_System RSL class
// ********************************************
class rc_RSL_System : public res_class {
	Resource *spawn(RWCString aname);
public:
	rc_RSL_System(RWCString aname) : res_class(aname)
	{	}
};


// *************************************************
// * R_RSL_System -- the RSL_System Resource
// *************************************************
class R_RSL_System : public Resource {
	
	// private class data
	
public:
	static rc_RSL_System rslType;

// Constructors
	
	R_RSL_System(RWCString n);
	
// Resource virtuals

	// Info

	unsigned int TypeID() { return R_RSL_System_ID; }
	res_class *memberOf(void) { return &rslType; }
	RWCString StrValue(void);
	int LogicalValue();
	int IsEqual(Resource *r);
	
	// Execution
	
	ResStatus execute(int method, ResList& arglist);
	void SetFromInline(RWTPtrSlist<Resource>& inliner);
	void Assign(Resource *r);
	void Clear();
	
	// output
	
	void print(ostream &out=cout);	// ECI
	void rslprint(ostream &out=cout);

// R_RSL_System specific
	
	static R_RSL_System *New(RWCString n);
	void  GetAllMembers(Resource *Destination, Resource *Source);
	
// R_RSL_System RSL methods
	
	ResStatus rsl_readObject(const ResList& arglist);
	ResStatus rsl_writeObject(const ResList& arglist);
	ResStatus rsl_stream(const ResList& arglist);
	ResStatus rsl_deStream(const ResList& arglist);
	ResStatus rsl_className(const ResList& arglist);
	ResStatus rsl_insert(const ResList& arglist);
    ResStatus rsl_insertIntoSession(const ResList& arglist);
	ResStatus rsl_remove(const ResList& arglist);
    ResStatus rsl_removeClass(const ResList& arglist);
    ResStatus rsl_hashCode(const ResList& arglist);
    ResStatus rsl_assign(const ResList& arglist);
    ResStatus rsl_instantiate(const ResList& arglist);
    ResStatus rsl_documentClass(const ResList& arglist);

	ResStatus rsl_get(const ResList& arglist);
	ResStatus rsl_QuitSession(const ResList& arglist);
};

#endif



