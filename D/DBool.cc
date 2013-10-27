#include "DBool.h"
#include <iostream.h>
#include <stdio.h>

static char rcsid[] = "$Id: DBool.cc,v 1.2 1998/11/24 14:55:04 holtrf Exp $";

// ************************
// * DR_Bool
// ************************

#define _hADD 4285540


// textual representation
DR_String DO_Bool::true_text = "true", DO_Bool::false_text = "false";

DR_Bool::DR_Bool(D *d) : DR_Magnitude(d)
{

}

DR_Bool::DR_Bool(const DRef& ref) : DR_Magnitude(ref)
{

}

// If you redefine dump() or Recycle(), you must
// call dump() from this destructor because DRef::dump(), invoked
// from DRef::~DRef(), may call Recycle() - and destructors do not
// call functions in derived classes. otherwise, don't call it.
DR_Bool::~DR_Bool()
{

}

// const_get()
// get the object as the correct type, but without the potential
// to modify it if it is null.
DO_Bool* DR_Bool::const_get() const
{
	return dynamic_cast<DO_Bool *> (unsafe_get());
}

// safe_get()
// this method is how operator->() is implemented; 
// the purpose is to get the object, correctly typed,
// in a way that can be directly dereferenced witout checking.
// Because it is possible for the object to be null, safe_get()
// will create it by calling New(). This is the correct behavior
// 99% of the time. If it is not, use either const_get() or
// look in D_macros.h to find the actual code. It's possible that
// throwing an exception is more appropriate in some cases instead
// of creating a new object.
//
// Note that this is not a virtual function.
DO_Bool* DR_Bool::safe_get()
{
	SAFE_GET(DO_Bool);	// defined in D_macros.h
}

// rarely used.
// replace() when safe_get() will be used avoids double checking.
DO_Bool* DR_Bool::safe_set(D* d)
{
	SAFE_SET(DO_Bool,d);	// defined in D_macros.h
}

DO_Bool *DR_Bool::New()
{
	DO_Bool *dobj = DO_Bool::New().const_get();
	replace(dobj);

	return dobj;
}

// ************************
// * DO_Bool
// ************************

// C++ constructor
// use init() for individual object initialization
DO_Bool::DO_Bool()
{
	val = 0;
}

// C++ destructor
// use destroy() for individual object de-construction
DO_Bool::~DO_Bool()
{
}

// init(): the "constructor"
void DO_Bool::init()
{
	// initialize superclass first
	DO_Magnitude::init();
	val = 0;
}

// destroy(): the "destructor"
void DO_Bool::destroy()
{
	val = 0;
	// destroy superclass last
	DO_Magnitude::destroy();
}

DR_String DO_Bool::toString()
{
	return val ? true_text : false_text;
}

// ********************************************************

// DOC_Bool: the Bool class class
// By defining this class in the .cc file, it is
// unavailable anywhere else. This is good, for now.
class DOC_Bool : public DO_Class {
	D *spawn();

  public:
	DOC_Bool() : DO_Class("Bool") { }

};

DR_Class DO_Bool::Boolclass = new DOC_Bool();

// The only place DO_Bool objects should be created.
// This function is private; it is called from
// DO_Class::New().
D *DOC_Bool::spawn() {
	return new DO_Bool();
}

DR_Class DO_Bool::DClass()
{
	return Boolclass;
}

// New()
// Create a new DO_Bool by asking for one from
// the static class object, DO_Bool::Boolclass.
// That object will either create one, by calling the virtual
// function spawn(), that is, DOC_Bool::spawn(),
// or getting one from its list of free objects.
//
// A static function
DR_Bool DO_Bool::New()
{
	return DO_Bool::Boolclass->New();
}

// Create_DOC_Bool()
// not currently in use as of Oct 1, 1998
// It will be used as the "bootstrap" function
// to create the class object, which will in turn
// create and recycle instances of DO_Bool
extern "C" DRef& Create_DOC_Bool()
{
	DO_Bool::Boolclass = new DOC_Bool();
	return DO_Bool::Boolclass;
}


// ********************************************************



// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
DRef DO_Bool::route(DR_Message m)
{
#ifdef DMEMORY
	cerr << "Bool::route() for `" << m->message << "'\n";
#endif

	/*
	switch(theIDHash(m->message->data()))
	{

		default: ;
	}
	*/

	return DO_Magnitude::route(m);
}

void DO_Bool::assign(const DRef& obj)
{
	val = DR_Bool(obj).b();
}

dcompare_t DO_Bool::compare(const DRef& d) const
{
	int& x = DR_Bool(d)->b();
	return val < x ? c_less : (val == x ? c_equal : c_greater );
}

BOOLEAN DO_Bool::toBoolean() const
{
	return val;
}

/**
	Add
	In RSL: Bool Add( Booleger n);
*/
DR_Bool DO_Bool::Add( const DR_Bool& n )
{
	return DR_null;
}


/* 	DR_Bool& operator=(const DR_Magnitude& n); */

DR_Magnitude DO_Bool::Assign( const DR_Magnitude& n )
{
	if (n.isValid())
	{
		// instead of simply saying replace(n.unsafe_get())
		// we really want to assign our DO_Bool  to the
		// DO_Bool inside of n because this object may be
		// pointed to by many

		DO_Bool * doi = DR_Bool(n).const_get();

		if (doi)
			val = doi->b();
	}

	return this;
}


