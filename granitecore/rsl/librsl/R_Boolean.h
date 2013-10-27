// R_Boolean.h
// $Id: R_Boolean.h,v 1.1 1998/11/17 23:55:02 toddm Exp $
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
#include "R_String.h"

#ifndef _R_Boolean_H_
#define _R_Boolean_H_

#define R_Boolean_ID 655229292

// ************************************
// * rc_Boolean -- the Boolean RSL class
// ************************************
class rc_Boolean : public res_class {
	Resource *spawn(RWCString aname);
public:
	rc_Boolean(RWCString aname) : res_class(aname)
	{
//		ResClasses.insert((res_class *) this);
	}
};


// ************************************
// * R_Boolean -- the Boolean Resource
// ************************************
class R_Boolean : public Resource {
	int val;

public:
	static rc_Boolean rslType;

	R_Boolean(int v=0);
	R_Boolean(RWCString n, int v=0);

	// Resource virtuals

	unsigned int TypeID() { return R_Boolean_ID; }
	res_class *memberOf(void) { return &rslType; }
	Resource *clone(void);
	RWCString StrValue(void) { return val? "true" : "false"; }
	int LogicalValue() { return val; }
	ResStatus execute(int method, ResList& arglist);
	void Clear() { val = 0; }
	void print(ostream &out=cout) { out << (val?"true" : "false"); }
	void rslprint(ostream &out=cout) { print(out); }
	void SetFromInline(RWTPtrSlist<Resource>& inliner);
	void Assign(Resource *r) { val = Bool(r); }
	int IsEqual(Resource *r) { return (val == Bool(r)); }

	// R_Boolean specific
	static R_Boolean *New(RWCString n, int v);
	static int BoolFromResource(Resource *r);
	static int Bool(Resource *r);
	int Bool(void) { return val; }
	inline void Set(int v) { val = v; }
	int FromString(RWCString s) { return (s == "true" ? 1 : 0); }
	inline void operator=(int v) { val = v; }
	friend inline int operator==(R_Boolean &b, int v);

	// R_Boolean RSL methods

	ResStatus OpEQ(const ResList& arglist);
	ResStatus OpAnd(const ResList& arglist);
	ResStatus OpOr(const ResList& arglist);
	ResStatus OpNE(const ResList& arglist);
	ResStatus OpEQ2(const ResList& arglist);
	ResStatus text(const ResList& arglist);
};

inline int operator==(R_Boolean &b, int v)
{
	return (b.val == v);
}
#endif



