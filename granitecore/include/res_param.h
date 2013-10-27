//	res_param.h
//	
//	classes for RSL2 formal parameter
//	representation and matching
//	
//	Formal parameter variations in RSL 2:
//		
//		Name
//		Name = default-literal
//		Name : match-value
//		Type Name
//		Type Name = default-literal
//		Type Name : match-value
//
// $Id: res_param.h,v 1.1 1998/11/17 23:55:53 toddm Exp $
// *******************
// * System Includes *
// *******************
#include <rw/cstring.h>
#include <rw/tpslist.h>
#include <rw/tvslist.h>

// ******************
// * Local Includes *
// ******************
//#include "Resource.h"

#ifndef _RES_PARAM_H_
#define _RES_PARAM_H_

class event;
class Resource;
class ResReference;
class rslMethod;
class ResList;
class ResContext;

// class param -- name and type.
// It is expected that most parameters will
// give both a name and a type.
#define PARAM_ALL_TYPES "*"
class param {
public:
	RWCString type;
	int kind;
	enum { rejected=0, notRejected,
			paramKind, presetKind, presetIRKind };
	RWCString name;
	unsigned int typeID;

	param(RWCString nm);
	inline int isType(unsigned int t) const { return (typeID == t); }
	virtual void SetType(RWCString t);
	virtual int operator==(const param& that);
	virtual int Match(ResReference& ref);
	virtual int equals(param *p);
	virtual void print(ostream& out=cout);
	virtual void html(ostream& out=cout);
	
#ifdef RSL_MEMORY_INFO
	static int NParams;
#endif

};


// param_preset
// a formal which either matches an actual by value or
// holds an initial value
class param_preset : public param
{
public:
	Resource *what;
	param_preset(RWCString nm, Resource *w=NULL) : param(nm)
		{ what = w; kind = presetKind; }
	int Match(ResReference& ref);
	int equals(param *p);
	virtual void print(ostream& out=cout);
	virtual void html(ostream& out=cout);
};

class param_presetIR : public param_preset
{
public:
	event *toEval;
	param_presetIR(RWCString nm, event *eval) : param_preset(nm, NULL)
		{ kind = presetIRKind; toEval = eval; }

	void SetType(RWCString t);
	void print(ostream& out=cout);
	void html(ostream& out=cout);

//	int Match(ResReference& ref);
//	int equals(param *p);	
};

class decl {
public:
	enum declflag_t { undefined=0, data=1, method=2, vconst=4,
		vprivate=8, vprotected=16, vpublic=32, vnative=64, vdeprecated=128,
		vabstract=256 };

	declflag_t flags;

	RWCString description;

	inline void setFlag(declflag_t k) { flags = (declflag_t) (((int) flags) | ((int) k)); }
	inline declflag_t Flags(void) { return flags; }
	inline int hasFlag(declflag_t k) { return (flags & k); }

	virtual void print(ostream& out=cout, int printScope=0)=0;
	virtual void html(ostream& out=cout, int printScope=0)=0;

protected:
	decl(void);
};

// Data declarations
// A type, followed by names
// It is likely that the list of varnames will become a list of
// "variable declaration" objects- a name and optionally an
// initial value, letting this class be used for variable declarations
// in both classes and methods (local variables).
class data_decl : public decl {
public:
	RWCString type;	// will become a hashed int
	RWTValSlist<RWCString> varnames;

	data_decl(RWCString initialVar);
	inline void SetType(RWCString t) { if (type == "unset") type = t; }
	inline void AddVar(RWCString v) { varnames.insert(v); }
	void print(ostream& out=cout, int printScope=0);
	void html(ostream& out=cout, int printScope=0);
	int install(ResContext *context);
	inline size_t entries() { return varnames.entries(); }
	inline int Match(const RWCString& member)
	{
		return varnames.contains(member);
	}
};

// *******************************************
// * class method_decl
// *  a method declaration ("prototype");
// *  an interface definition.
// *******************************************
class method_decl : public decl {

public:
	RWCString name;
	//	int memberOf, returnType;
	RWCString returnType, memberOf;
	rslMethod *implementation;
	RWTPtrSlist<param> fparams;	// formal parameters
	int infinity;

	method_decl(void);
	
	inline void AddParam(param *pa) { fparams.append(pa); }

	int IsIdentical(const method_decl& obj);
	int Match(int method, ResList& arglist);
	int operator==(const method_decl& obj);
	int operator<(const method_decl& obj);
	void print(ostream& out=cout, int printScope=0);
	void html(ostream& out=cout, int printScope=0);
};

extern param InfinityParam;		// in res_param.cc


#endif



