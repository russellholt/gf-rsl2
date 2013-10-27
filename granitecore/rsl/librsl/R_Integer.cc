// R_Integer.cc
// $Id: R_Integer.cc,v 1.1 1998/11/17 23:53:18 toddm Exp $

#include <fstream.h>
#include <stdlib.h>

#include "R_Integer.h"
#include "R_Boolean.h"
#include "R_String.h"
#include "R_Status.h"
#include "runtime.h"

//include "string.h"

extern ofstream rslerr;
//static ofstream rslerr("rslerr");

extern "C" res_class *Create_Integer_RC()
{
	return &(R_Integer::rslType);
}

// R_Integer static member
rc_Integer R_Integer::rslType("Integer");

// R_Integer method name hash definitions
#define _hOpAdd 43	// +
#define _hOpSubt 45	// -
#define _hOpMult 42	// *
#define _hOpDiv 47	// /
#define _hOpMod 37	// %
#define _hOpLT 60	// <
#define _hOpGT 62	// >
#define _hOpLE 15421	// <=
#define _hOpGE 15933	// >=
#define _hOpNE 8509	// !=
#define _hOpEQ2 15677	// ==
#define _hOpASSIGN 14909    // :=
#define _hOpEQ 61   // =	(OBSOLETE)
#define _hOpPE 11069	// +=
#define _hOpME 11581	// -=
#define _hOpTE 10813	// *=
#define _hOpDE 12093	// /=
#define _hTEXT 1952807028	// text
#define _hRANGE 392261223	// range

// Spawn - create a new resource of this type (R_Integer)
// Called by res_class::New() if there is no object
// to pull off the free list.
Resource* rc_Integer::spawn(RWCString nm)
{
	return new R_Integer(nm);
}

R_Integer::R_Integer(int v)
{
	val = v;
}

R_Integer::R_Integer(RWCString n, int v) : Resource(n)
{
	val = v;
}

// Static utility creator
R_Integer *R_Integer::New(RWCString n, int v)
{
	Resource *r = R_Integer::rslType.New(n);
	((R_Integer *) r)->Set(v);
	return (R_Integer *) r;
}

Resource *R_Integer::clone(void)
{
	return (Resource *) R_Integer::New("", val);
}


RWCString R_Integer::StrValue(void)
{
char s[15];
	sprintf(s, "%d", val);
	return RWCString(s);
}

ResStatus R_Integer::execute(int method, ResList& arglist)
{
	switch(method)
	{
		case _hOpAdd:	// "+"
			return OpAdd(arglist);
			break;
		case _hOpSubt:	// "-"
			return OpSubt(arglist);
			break;
		case _hOpMult:	// "*"
			return OpMult(arglist);
			break;
		case _hOpDiv:	// "/"
			return OpDiv(arglist);
			break;
		case _hOpMod:	// "%"
			return OpMod(arglist);
			break;
		case _hOpLT:	// "<"
			return OpLT(arglist);
			break;
		case _hOpGT:	// ">"
			return OpGT(arglist);
			break;
		case _hOpLE:	// "<="
			return OpLE(arglist);
			break;
		case _hOpGE:	// ">="
			return OpGE(arglist);
			break;
		case _hOpNE:	// "!="
			return OpNE(arglist);
			break;
		case _hOpEQ2:	// "=="
			return OpEQ2(arglist);
			break;
		case _hOpASSIGN:	// ":="
		case _hOpEQ:	// "="	(OBSOLETE)
			return OpEQ(arglist);
			break;
		case _hOpPE:	// "+="
			return OpPE(arglist);
			break;
		case _hOpME:	// "-="
			return OpME(arglist);
			break;
		case _hOpTE:	// "*="
			return OpTE(arglist);
			break;
		case _hOpDE:	// "/="
			return OpDE(arglist);
			break;
		case _hTEXT:	// "text"
			return text(arglist);
			break;
		case _hRANGE:	// "range"
			return range(arglist);
			break;
		default: ;
	}
	ResStatus stat;
	stat.status = ResStatus::rslFail;
	return stat;
}

int R_Integer::IntFromResource(Resource *r)
{
	return R_Integer::Int(r);
}

// Int
// static conversion function, to turn a resource into an int.
int R_Integer::Int(Resource *r)
{
	if (!r)
		return 0;

int newval;

	switch(r->TypeID())
	{
		case R_Integer_ID:
			newval = ((R_Integer *) r)->intval();
			break;

		case R_Boolean_ID:
			newval = ((R_Boolean *) r)->Bool();
			break;

		case R_Status_ID:
			newval = ((R_Status *) r)->Number();
			break;

		case R_String_ID:
			newval = atoi((r->StrValue()).data());
			break;
			
		default:
			cerr << "Integer: conversion from class `" << r->ClassName()
				<< "' is not implemented.\n";
			newval = 0;
	}
	return newval;
}

void R_Integer::SetFromInline(RWTPtrSlist<Resource>& inliner)
{
	if (inliner.entries() > 0)
		Assign(inliner.first());
}

ResStatus R_Integer::OpAdd(const ResList& arglist)
{
	ResReference ref = arglist[0];
	rslerr << "R_Integer::OpAdd(), with argument `";
	ref.print(rslerr);
	rslerr << "'\n";
	
	int newv = val + Int(ref());
	R_Integer *newobj = R_Integer::New("TempIntVal", newv);

	rslerr << "\tNew object: `";
	newobj->print(rslerr);
	rslerr << "'\n";


	return ResStatus(ResStatus::rslOk, newobj);
}

ResStatus R_Integer::OpSubt(const ResList& arglist)
{
	int newv = val - Int(arglist.get(0));
	return ResStatus(ResStatus::rslOk, R_Integer::New("TempIntVal", newv));
}

ResStatus R_Integer::OpMult(const ResList& arglist)
{
	int newv = val * Int(arglist.get(0));
	return ResStatus(ResStatus::rslOk, R_Integer::New("TempIntVal", newv));
}

ResStatus R_Integer::OpDiv(const ResList& arglist)
{
	int newv=0, dv = Int(arglist.get(0));
	if (dv != 0)
		newv = val / dv;
	else
		cerr << "Error: division by zero.\n";

	return ResStatus(ResStatus::rslOk, R_Integer::New("TempIntVal", newv));
}

ResStatus R_Integer::OpMod(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_Integer::New("modresult", val % Int(arglist.get(0)))
	);
}

ResStatus R_Integer::OpLT(const ResList& arglist)
{
	int argval = Int(arglist.get(0));
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("TempBoolVal", (val < argval))
	);
}

ResStatus R_Integer::OpGT(const ResList& arglist)
{
	int argval = Int(arglist.get(0));
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("TempBoolVal", (val > argval))
	);
}

ResStatus R_Integer::OpLE(const ResList& arglist)
{
	int argval = Int(arglist.get(0));
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("TempBoolVal", (val <= argval))
	);
}

ResStatus R_Integer::OpGE(const ResList& arglist)
{
	int argval = Int(arglist.get(0));
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("TempBoolVal", (val >= argval))
	);
}

ResStatus R_Integer::OpNE(const ResList& arglist)
{
	int argval = Int(arglist.get(0));
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("TempBoolVal", (val != argval))
	);
}

ResStatus R_Integer::OpEQ2(const ResList& arglist)
{
	int argval = Int(arglist.get(0));
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("TempBoolVal", (val == argval))
	);
}

// Assignment/conversion
ResStatus R_Integer::OpEQ(const ResList& arglist)
{
	val = Int(arglist.get(0));
	return ResStatus(ResStatus::rslOk, this);
}

ResStatus R_Integer::OpPE(const ResList& arglist)
{
	val += Int(arglist.get(0));
	return ResStatus(ResStatus::rslOk, this);
}

ResStatus R_Integer::OpME(const ResList& arglist)
{
	val -= Int(arglist.get(0));
	return ResStatus(ResStatus::rslOk, this);
}

ResStatus R_Integer::OpTE(const ResList& arglist)
{
	val *= Int(arglist.get(0));
	return ResStatus(ResStatus::rslOk, this);
}

ResStatus R_Integer::OpDE(const ResList& arglist)
{
	int newval = Int(arglist.get(0));
	if (newval != 0)
		val /= newval;
	else
		cerr << "Error: division by zero.\n";
	return ResStatus(ResStatus::rslOk, this);
}

// text - return a string version of the integer.
ResStatus R_Integer::text(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_String::New("tempStringVal", StrValue() )
	);
}

// range - given two integer arguments l and r, return
// a boolean indicating the truth of the following expression:
// 		l < val < r
// that is, in C,
//		l < val && r > val
ResStatus R_Integer::range(const ResList& arglist)
{
	int l = Int(arglist.get(0)),
		r = Int(arglist.get(0));

	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("tempBoolVal", (l < val && r > val) )
	);
}
