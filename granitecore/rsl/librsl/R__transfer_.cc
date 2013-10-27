// *******************************************************************
// R__transfer_.cc
//
// automatically generated from _transfer_.rsl and the template:
// $Id: R__transfer_.cc,v 1.1 1998/11/17 23:53:46 toddm Exp $
// *******************************************************************

#include "R__transfer_.h"
#include "destiny.h"
#include "rsldefaults.h"


// R__transfer_ static member
rc__transfer_ R__transfer_::rslType("_transfer_");

extern "C" res_class *Create__transfer__RC()
{
	return &(R__transfer_::rslType);
}


// Spawn - create a new resource of this type (R__transfer_)
// Called by res_class::New() if there is no object
// to pull off the free list.
Resource* rc__transfer_::spawn(RWCString nm)
{
	return new R__transfer_(nm);	// or other constructor
}

// New
// static convenience function
// See R_String::New(), R_Integer::New(), etc.
// If no functionality beyond res_class::New() is required
// (ie, no special values to be set conveniently), then this
// function simply eliminates the need to cast the result
// of res_class::New().
R__transfer_ *R__transfer_::New(RWCString n)
{
	Resource *r= R__transfer_::rslType.New(n);

	return (R__transfer_ *) r;
}

// R__transfer_ constructor
R__transfer_::R__transfer_(RWCString nm)
	: ResStructure(nm.data(), nm.data(), BUCKETS_IN_NAMESPACE)
{

}


// SetFromInline()
// Overrides ResStructure::SetFromInline()
// Difference being that if a named resource (member of the
// rvalue, that is, a key/value) is  not found in the
// lvalue (this), it is added (otherwise, assigned).
void R__transfer_::SetFromInline(RWTPtrSlist<Resource>& inliner)
{
	RWTPtrSlistIterator<Resource> iter(inliner);
	Resource *r=NULL;

	while(iter())
	{
		r = iter.key();
		ResStatus stat = locals.Find(r->Name());
		if (stat.status == ResStatus::rslOk)
			stat.r->Assign(r);
		else
			locals.AddResource(r);
	}
}


// *************************************************************
// execute()
// This is the interface between RSL and C++ Resources.
//
// Automatically generated from "_transfer_.rsl"
// DO NOT MODIFY !
// *************************************************************

ResStatus R__transfer_::execute(int method, ResList& arglist)
{
	switch(method)
	{
		default: ;
	}

	return ResStatus(ResStatus::rslFail);
}
