// ******************************************************************
// R_Queue.h
//
// automatically generated from Queue.rsl and the template:
// $Id: R_Queue.h,v 1.1 1998/11/17 23:55:14 toddm Exp $
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
#include "b.h"

#ifndef _R_Queue_H_
#define _R_Queue_H_

#define R_Queue_ID 880108917

// ********************************************
// * rc_Queue -- the Queue RSL class
// ********************************************
class rc_Queue : public res_class {
	Resource *spawn(RWCString aname);
public:
	rc_Queue(RWCString aname) : res_class(aname)
	{	}
};

// *************************************************
// * R_Queue -- the Queue Resource
// *************************************************
class R_Queue : public Resource {
	
	// private class data

	elist *theQ;
	
public:
	static rc_Queue *rslType;

// Constructors
	
	R_Queue(RWCString n);
	
// Resource virtuals

	// Info

	unsigned int TypeID() { return R_Queue_ID; }
	res_class *memberOf(void) { return rslType; }
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

// R_Queue specific
	
	static R_Queue *New(RWCString n);
	inline elist *& getQ() { return theQ; }
	void add(ResReference *);
	
// R_Queue RSL methods
	
	ResStatus rsl_add(const ResList& arglist);
	ResStatus rsl_remove(const ResList& arglist);
};

#endif



