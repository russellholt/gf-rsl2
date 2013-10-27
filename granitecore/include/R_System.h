// R_System.h
// $Id: R_System.h,v 1.1 1998/11/17 23:55:29 toddm Exp $
// *******************
// * System Includes *
// *******************
#include <iostream.h>
#include <rw/cstring.h>
#include <rw/tvslist.h>

// ******************
// * Local Includes *
// ******************
#include "res_class.h"
#include "Resource.h"

#ifndef _R_System_H_
#define _R_System_H_

#define R_System_ID 907309940

// ************************************
// * rc_System -- the System RSL class
// ************************************
class rc_System : public res_class {
	Resource *spawn(RWCString aname);
public:
	rc_System(RWCString aname) : res_class(aname)
	{
//		ResClasses.insert((res_class *) this);
	}
};


// ************************************
// * R_System -- the System Resource
// ************************************
class R_System : public Resource {
public:
	static rc_System rslType;

	R_System();
	R_System(RWCString n);

	// Resource virtuals

	unsigned int TypeID() { return R_System_ID; }
	res_class *memberOf(void) { return &rslType; }
	Resource *clone(void);
	RWCString StrValue(void) { return RWCString(""); }
	int LogicalValue() { return 1; }
	ResStatus execute(int method, ResList& arglist);
	void Clear() { }
	void print(ostream &out=cout) { out << name; }
	void rslprint(ostream &out=cout) { print(out); }

	// R_System specific

	static R_System *New(RWCString n);
	
	// R_System RSL methods

	ResStatus rsl_date(const ResList& arglist);
	ResStatus rsl_time(const ResList& arglist);
	ResStatus rsl_print(const ResList& arglist);

};

#endif


