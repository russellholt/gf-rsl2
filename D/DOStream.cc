#include "DOStream.h"

#include <iostream.h>

static char rcsid[] = "$Id: DOStream.cc,v 1.1 1998/11/12 18:31:03 holtrf Exp $";

// ************************
// * DR_OStream
// ************************

#define _hSTREAM 303657573
#define _hCOMPOSITE 1763443716
#define _hCOLLECTION 1768036357


DR_OStream::DR_OStream(D *d) : DR_Object(d)
{

}

DR_OStream::DR_OStream(const DRef& ref) : DR_Object(ref)
{

}

// If you redefine dump() or Recycle(), you must
// call dump() from this destructor because DRef::dump(), invoked
// from DRef::~DRef(), may call Recycle() - and destructors do not
// call functions in derived classes. otherwise, don't call it.
DR_OStream::~DR_OStream()
{

}

// const_get()
// get the object as the correct type, but without the potential
// to modify it if it is null.
DO_OStream* DR_OStream::const_get() const
{
	return dynamic_cast<DO_OStream *> (unsafe_get());
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
DO_OStream* DR_OStream::safe_get()
{
	DO_OStream * x = dynamic_cast<DO_OStream *> (unsafe_get());
	if (x)
		return x;

	cerr << "Can't instantiate abstract DO_OStream.\n";
	return (DO_OStream *) 0;
}

// rarely used.
// replace() when safe_get() will be used avoids double checking.
DO_OStream* DR_OStream::safe_set(D* d)
{
	SAFE_SET(DO_OStream,d);	// defined in D_macros.h
}

// ************************
// * DO_OStream
// ************************

// C++ constructor
// use init() for individual object initialization
DO_OStream::DO_OStream()
{

}

// C++ destructor
// use destroy() for individual object de-construction
DO_OStream::~DO_OStream()
{
}

// init(): the "constructor"
void DO_OStream::init()
{
	// initialize superclass first
	DO_Object::init();

}

// destroy(): the "destructor"
void DO_OStream::destroy()
{

	// destroy superclass last
	DO_Object::destroy();
}

DR_String DO_OStream::toString()
{
	return DR_String("OStream toString() not implemented");
}

// ********************************************************


// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
DRef DO_OStream::route(DR_Message m)
{
    // until the semantics of route() are nailed down,
	// its body is commented out in the template text.
	// for the moment it simply bounces the message
	// to its superclass.

	switch(theIDHash(m->message->data()))
	{
		case _hSTREAM: // RSL: String stream( Object o);
			return stream( DR_Object(m("o")) );
		case _hCOMPOSITE: // RSL: String composite( Composite c);
			return composite( DR_Composite(m("c")) );
		case _hCOLLECTION: // RSL: String collection( Collection c);
			return collection( DR_Collection(m("c")) );


		default: ;
	}
	return DO_Object::route(m);
}


