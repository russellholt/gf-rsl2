// R_Boolean.cc
// $Id: R_Boolean.cc,v 1.1 1998/11/17 23:53:13 toddm Exp $

#include "R_Boolean.h"
#include "runtime.h"

static char rcsid[] = "$Id: R_Boolean.cc,v 1.1 1998/11/17 23:53:13 toddm Exp $";

extern "C" res_class *Create_Boolean_RC()
{
	return &(R_Boolean::rslType);
}

// R_Boolean static member
rc_Boolean R_Boolean::rslType("Boolean");

// R_Boolean method name hash definitions
#define _hOpASSIGN 14909    // :=
#define _hOpEQ 61   // =	(OBSOLETE)
#define _hOpAnd 9766	// &&
#define _hOpOr 31868	// ||
#define _hOpNE 8509	// !=
#define _hOpEQ2 15677	// ==
#define _hOpEQ 61	// =
#define _hOpEQ 61	// =
#define _hTEXT 1952807028	// text


// Spawn - create a new resource of this type (R_Boolean)
// Called by res_class::New() if there is no object
// to pull off the free list.
Resource* rc_Boolean::spawn(RWCString nm)
{
	return new R_Boolean(nm);
}

R_Boolean::R_Boolean(int v) : val(v)
{ }

R_Boolean::R_Boolean(RWCString n, int v) : Resource(n), val(v)
{ }

R_Boolean *R_Boolean::New(RWCString n, int v)
{
	Resource *r = R_Boolean::rslType.New(n);
	if (r)
		((R_Boolean *) r)->Set(v);
	return (R_Boolean *) r;
}

Resource *R_Boolean::clone(void)
{
//	return new R_Boolean(val);
	return R_Boolean::New("", val);
}

int R_Boolean::BoolFromResource(Resource *r)
{
	return Bool(r);
}

int R_Boolean::Bool(Resource *r)
{
	if (!r)
		return 0;
	
	// Only one special case: string
	if (r->TypeID() == R_String_ID)
	{
		RWCString s = r->StrValue();
		s.toLower();
		if (s == "1" || s == "true" | s == "yes")
			return 1;
	}

	return r->LogicalValue();
}

void R_Boolean::SetFromInline(RWTPtrSlist<Resource>& inliner)
{
	if (inliner.entries() > 0)
		Assign(inliner.first());
}

ResStatus R_Boolean::execute(int method, ResList& arglist)
{
	switch(method)
	{
		case _hOpASSIGN:	// ":="
		case _hOpEQ:	// "="	(OBSOLETE)
			return OpEQ(arglist);
			break;
		case _hOpAnd:	// "&&"
			return OpAnd(arglist);
			break;
		case _hOpOr:	// "||"
			return OpOr(arglist);
			break;
		case _hOpNE:	// "!="
			return OpNE(arglist);
			break;
		case _hOpEQ2:	// "=="
			return OpEQ2(arglist);
			break;
		case _hTEXT:	// "text"
			return text(arglist);
			break;
		default: ;
	}

	return ResStatus(ResStatus::rslFail);
}

ResStatus R_Boolean::OpEQ(const ResList& arglist)
{
	val = Bool(arglist.get(0));
	return ResStatus(ResStatus::rslOk, this);
}

ResStatus R_Boolean::OpAnd(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("tempBool", (val && arglist[0].LogicalValue()))
	);
}

ResStatus R_Boolean::OpOr(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("tempBool", (val || arglist[0].LogicalValue()))
	);
}

ResStatus R_Boolean::OpNE(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("tempBool", (val != arglist[0].LogicalValue()))
	);
}

ResStatus R_Boolean::OpEQ2(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("tempBool", (val == arglist[0].LogicalValue()))
	);
}

ResStatus R_Boolean::text(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_String::New("tempString", StrValue())
	);
}
