// R_String.h
// $Id: R_Integer.h,v 1.1 1998/11/17 23:55:05 toddm Exp $
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

#ifndef _R_Integer_H_
#define _R_Integer_H_

#define R_Integer_ID 772474469

// ************************************
// * rc_Integer -- the Integer RSL class
// ************************************
class rc_Integer : public res_class {
	Resource *spawn(RWCString aname);
public:
	rc_Integer(RWCString aname) : res_class(aname)
	{
//		ResClasses.insert((res_class *) this);
	}
};


// ************************************
// * R_Integer -- the Integer Resource
// ************************************
class R_Integer : public Resource {
	int val;

public:
	static rc_Integer rslType;

	R_Integer(int v=0);
	R_Integer(RWCString n, int v=0);

	// Resource virtuals

	unsigned int TypeID() { return R_Integer_ID; }
	res_class *memberOf(void) { return &rslType; }
	Resource *clone(void);
	RWCString StrValue(void);
	int LogicalValue() { return val; }
	ResStatus execute(int method, ResList& arglist);
	void Clear() { val = 0; }
	void print(ostream &out=cout) { out << val; }
	void rslprint(ostream &out=cout) { print(out); }
	void SetFromInline(RWTPtrSlist<Resource>& inliner);
	void Assign(Resource *r) { val = IntFromResource(r); }
	int IsEqual(Resource *r) { return (val == IntFromResource(r)); }

	// R_Integer specific

	int intval(void) { return val; }
	void Set(int i)  { val = i; }
	void operator=(const int i) { val = i; }
	static R_Integer *New(RWCString n, int v);
	static int IntFromResource(Resource *r);
	static int Int(Resource *r);
	
	// R_Integer RSL methods
	
	ResStatus OpAdd(const ResList& arglist);
	ResStatus OpSubt(const ResList& arglist);
	ResStatus OpMult(const ResList& arglist);
	ResStatus OpDiv(const ResList& arglist);
	ResStatus OpMod(const ResList& arglist);
	
	ResStatus OpLT(const ResList& arglist);
	ResStatus OpGT(const ResList& arglist);
	ResStatus OpLE(const ResList& arglist);
	ResStatus OpGE(const ResList& arglist);
	ResStatus OpNE(const ResList& arglist);
	ResStatus OpEQ2(const ResList& arglist);
	
	ResStatus OpEQ(const ResList& arglist);
	ResStatus OpPE(const ResList& arglist);
	ResStatus OpME(const ResList& arglist);
	ResStatus OpTE(const ResList& arglist);
	ResStatus OpDE(const ResList& arglist);
	
	ResStatus text(const ResList& arglist);
	ResStatus range(const ResList& arglist);


};

#endif



