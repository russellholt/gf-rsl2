#include "DLibrary.h"

#include <iostream.h>

static char rcsid[] = "$Id: DLibrary.cc,v 1.2 1998/11/20 18:15:59 holtrf Exp $";

// ************************
// * DR_Library
// ************************

#define _hCREATE 387409249


DR_Library::DR_Library(D *d) : DR_Object(d)
{

}

DR_Library::DR_Library(const DRef& ref) : DR_Object(ref)
{

}

// If you redefine dump() or Recycle(), you must
// call dump() from this destructor because DRef::dump(), invoked
// from DRef::~DRef(), may call Recycle() - and destructors do not
// call functions in derived classes. otherwise, don't call it.
DR_Library::~DR_Library()
{

}

DO_Library *DR_Library::const_get() const
{
	return dynamic_cast<DO_Library *> (unsafe_get());
}

DO_Library* DR_Library::safe_get()
{
	cerr << "\t\tDR_Library::safe_get()\n";
	SAFE_GET(DO_Library);	// D_macros.h
}

// rarely used.
// _set() when safe_get() is used avoids double checking.
DO_Library* DR_Library::safe_set(D* d)
{
	SAFE_SET(DO_Library,d);	// D_macros.h
}

DO_Library *DR_Library::New()
{
	cerr << "DR_Library::New()\n";
	// TEMPORARY!
	return safe_set(new DO_Library());
}

// ************************
// * DO_Library
// ************************

// C++ constructor
// use init() for individual object initialization
DO_Library::DO_Library()
{

}

// C++ destructor
// use destroy() for individual object de-construction
DO_Library::~DO_Library()
{
}

// init(): the "constructor"
void DO_Library::init()
{
	// initialize superclass first
	DO_Object::init();

}

// destroy(): the "destructor"
void DO_Library::destroy()
{
	libname.dump();

	// destroy superclass last
	DO_Object::destroy();
}

DR_String DO_Library::toString() {
	return DR_String("library");
}

// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************

DRef DO_Library::route(DR_Message m)
{
/*
	switch(m.messageID())
	{
		case _hCREATE: // RSL: Object create( String classname);
			return create( DR_String(m("classname")) );


		default: ;
	}
*/
	return DO_Object::route(m);
}

/**
	create
	

	In RSL: Object create( String classname);
*/
DRef DO_Library::create( DR_String& classname )
{
	cerr << "DO_Library::create() shouldn't be called.\n";
	return DR_null;
}
