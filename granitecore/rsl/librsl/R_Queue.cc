// *******************************************************************
// R_Queue.cc
//
// automatically generated from Queue.rsl and the template:
// $Id: R_Queue.cc,v 1.1 1998/11/17 23:53:28 toddm Exp $
// *******************************************************************

#include "R_Queue.h"
#include "b.h"
#include "rslEvents.h"

#define _hADD 6382692	// add
#define _hREMOVE 67136879	// remove

extern void elist___kill(event *& el);

// R_Queue static member
rc_Queue *R_Queue::rslType = NULL;

extern "C" res_class *Create_Queue_RC()
{
	return R_Queue::rslType = new rc_Queue("Queue");
}


// Spawn - create a new resource of this type (R_Queue)
// Called by res_class::New() if there is no object
// to pull off the free list.
Resource* rc_Queue::spawn(RWCString nm)
{
	return new R_Queue(nm);	// or other constructor
}

// New
// static convenience function
// See R_String::New(), R_Integer::New(), etc.
// If no functionality beyond res_class::New() is required
// (ie, no special values to be set conveniently), then this
// function simply eliminates the need to cast the result
// of res_class::New().
R_Queue *R_Queue::New(RWCString n /*, some value(s) */)
{
	if (!R_Queue::rslType)
		(void) Create_Queue_RC();

	if (!R_Queue::rslType)
		return NULL;

	Resource *r= R_Queue::rslType->New(n);
//	((R_Queue *) r)->Set( /* values (add this function if needed) */ );
	return (R_Queue *) r;
}

// R_Queue constructor
R_Queue::R_Queue(RWCString nm)
	: Resource(nm)
{
	theQ = NULL;
}

// StrValue()
// Return a String version of this Resource, only if applicable.
RWCString R_Queue::StrValue(void)
{
		return "a queue";
}

// LogicalValue()
// Evaluate the "trueness" of this Resource (1 for true, 0 for false)
// Used in logical comparisons.
int R_Queue::LogicalValue()
{
		return (theQ != NULL);

}

// IsEqual()
// Test for equality with another Resource.
// (ResStructure provides a default version)
int R_Queue::IsEqual(Resource *r)
{
		return 0;
}

// SetFromInline
// Given a list of Resources, match with a data member of the same
// name and assign. eg, in RSL, "myclass { a:1, b:2, /* etc */ }"
// an object of type `myclass' is created and SetFromInline() is called
// for the list of resources enclosed in { }.
// (ResStructure provides a default version)
void R_Queue::SetFromInline(RWTPtrSlist<Resource>& inliner)
{
	/* modify this */
}

// Assign
// set this resource equal to r.
// (ResStructure provides a default version)
void R_Queue::Assign(Resource *r)
{
	/* modify this */
}

// Clear()
// Memory management - called to restore an object
// to a "just created" state, for free-list management.
// (ResStructure provides a default version)
void R_Queue::Clear()
{
	/* this is questionable */
	elist___kill(theQ);
	theQ = NULL;

}

// print()
// ECI syntax
// (ResStructure provides a default version)
void R_Queue::print(ostream &out)
{
		if (theQ)
			theQ->print(out);
}

// rslprint()
// Printing from within RSL
// (ResStructure provides a default version)
void R_Queue::rslprint(ostream &out)
{
	/* modify this */
}

// *************************************************************
// execute()
// This is the interface between RSL and C++ Resources.
//
// Automatically generated from "Queue.rsl"
// DO NOT MODIFY !
// *************************************************************

ResStatus R_Queue::execute(int method, ResList& arglist)
{
	switch(method)
	{
		case _hADD:	// "add"
			return rsl_add(arglist);

		case _hREMOVE:	// "remove"
			return rsl_remove(arglist);

		default: ;
	}

	return ResStatus(ResStatus::rslFail);
}

void R_Queue::add(ResReference *res)
{
	if (theQ == NULL)
		theQ = new elistArg;

	ResArg *ra = new ResArg(res);
	ra->argName = res->Name();
	theQ->add(ra);
}

// RSL method "add"
ResStatus R_Queue::rsl_add(const ResList& arglist)
{
	ResReference ref;

	for (int i=0; i<arglist.entries(); i++)
		add((ResReference *) arglist.getref(i));

	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "remove"
ResStatus R_Queue::rsl_remove(const ResList& arglist)
{
		cerr << "not implemented\n\n";
	return ResStatus(ResStatus::rslOk, NULL);
}
