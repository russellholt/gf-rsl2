// ******************************************************************
// R__transfer_.h
//
// automatically generated from _transfer_.rsl and the template:
// $Id: R__transfer_.h,v 1.1 1998/11/17 23:55:32 toddm Exp $
// ******************************************************************
// *******************
// * System Includes *
// *******************
#include "rslEvents.h"

// ******************
// * Local Includes *
// ******************
#include "res_class.h"
#include "Resource.h"
// #include "runtime.h"

#ifndef _R__transfer__H_
#define _R__transfer__H_

#define R__transfer__ID 1129845764

// ********************************************
// * rc__transfer_ -- the _transfer_ RSL class
// ********************************************
class rc__transfer_ : public res_class {
	Resource *spawn(RWCString aname);
public:
	rc__transfer_(RWCString aname) : res_class(aname)
	{	}
};

// *************************************************
// * R__transfer_ -- the _transfer_ Resource
// *************************************************
class R__transfer_ : public ResStructure {
	
	// private class data
	
public:
	static rc__transfer_ rslType;

// Constructors
	
	R__transfer_(RWCString n);
	
// Resource virtuals

	// Info

	unsigned int TypeID() { return R__transfer__ID; }
	res_class *memberOf(void) { return &rslType; }
	
	// Execution
	
	ResStatus execute(int method, ResList& arglist);

// ResStructure virtuals
	void SetFromInline(RWTPtrSlist<Resource>& inliner);

// R__transfer_ specific
	
	static R__transfer_ *New(RWCString n);
	
// R__transfer_ RSL methods
	
};

#endif



