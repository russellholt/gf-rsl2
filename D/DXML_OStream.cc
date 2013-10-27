#include "DXML_OStream.h"

#include <iostream.h>

static char rcsid[] = "$Id: DXML_OStream.cc,v 1.1 1998/11/12 18:31:31 holtrf Exp $";

// ************************
// * DR_XML_OStream
// ************************

#define _hSTREAM 303657573
#define _hCOMPOSITE 1763443716
#define _hCOLLECTION 1768036357


DR_XML_OStream::DR_XML_OStream(D *d) : DR_OStream(d)
{

}

DR_XML_OStream::DR_XML_OStream(const DRef& ref) : DR_OStream(ref)
{

}

// If you redefine dump() or Recycle(), you must
// call dump() from this destructor because DRef::dump(), invoked
// from DRef::~DRef(), may call Recycle() - and destructors do not
// call functions in derived classes. otherwise, don't call it.
DR_XML_OStream::~DR_XML_OStream()
{

}

// const_get()
// get the object as the correct type, but without the potential
// to modify it if it is null.
DO_XML_OStream* DR_XML_OStream::const_get() const
{
	return dynamic_cast<DO_XML_OStream *> (unsafe_get());
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
DO_XML_OStream* DR_XML_OStream::safe_get()
{
	SAFE_GET(DO_XML_OStream);	// defined in D_macros.h
}

// rarely used.
// replace() when safe_get() will be used avoids double checking.
DO_XML_OStream* DR_XML_OStream::safe_set(D* d)
{
	SAFE_SET(DO_XML_OStream,d);	// defined in D_macros.h
}

DO_XML_OStream *DR_XML_OStream::New()
{
	DO_XML_OStream *dobj = DO_XML_OStream::New().const_get();
	replace(dobj);

	return dobj;
}

// ************************
// * DO_XML_OStream
// ************************

// C++ constructor
// use init() for individual object initialization
DO_XML_OStream::DO_XML_OStream()
{

}

// C++ destructor
// use destroy() for individual object de-construction
DO_XML_OStream::~DO_XML_OStream()
{
}

// init(): the "constructor"
void DO_XML_OStream::init()
{
	// initialize superclass first
	DO_OStream::init();

}

// destroy(): the "destructor"
void DO_XML_OStream::destroy()
{

	// destroy superclass last
	DO_OStream::destroy();
}

DR_String DO_XML_OStream::toString()
{
	return DR_String("XML_OStream toString() not implemented");
}

// ********************************************************

// DOC_XML_OStream: the XML_OStream class class
// By defining this class in the .cc file, it is
// unavailable anywhere else. This is good, for now.
class DOC_XML_OStream : public DO_Class {
	D *spawn();

  public:
	DOC_XML_OStream() : DO_Class("XML_OStream") { }

};

DR_Class DO_XML_OStream::XML_OStreamclass = new DOC_XML_OStream();
//DR_Class DO_XML_OStream::XML_OStreamclass;

// The only place DO_XML_OStream objects should be created.
// This function is private; it is called from
// DO_Class::New().
D *DOC_XML_OStream::spawn() {
	return new DO_XML_OStream();
}

DRef DO_XML_OStream::Class()
{
	return XML_OStreamclass;
}

// New()
// Create a new DO_XML_OStream by asking for one from
// the static class object, DO_XML_OStream::XML_OStreamclass.
// That object will either create one, by calling the virtual
// function spawn(), that is, DOC_XML_OStream::spawn(),
// or getting one from its list of free objects.
//
// A static function
DR_XML_OStream DO_XML_OStream::New()
{
	return DO_XML_OStream::XML_OStreamclass->New();
}

// Create_DOC_XML_OStream()
// not currently in use as of Oct 1, 1998
// It will be used as the "bootstrap" function
// to create the class object, which will in turn
// create and recycle instances of DO_XML_OStream
extern "C" DRef& Create_DOC_XML_OStream()
{
	DO_XML_OStream::XML_OStreamclass = new DOC_XML_OStream();
	return DO_XML_OStream::XML_OStreamclass;
}


// ********************************************************



// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
DRef DO_XML_OStream::route(DR_Message m)
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
	return DO_OStream::route(m);
}



/**
	stream

	In RSL: String stream( Object o);
*/
DR_String DO_XML_OStream::stream( const DRef& ref)
{
	cerr << "XML_OStream is not implemented.\n";
	return DR_null;
}

/**
	composite

	In RSL: String composite( Composite c);
*/
DR_String DO_XML_OStream::composite( const DR_Composite& obj )
{
	cerr << "XML_OStream is not implemented.\n";
	return DR_null;
}


DR_String DO_XML_OStream::collection ( const DR_Collection& obj)
{
	cerr << "XML_OStream is not implemented.\n";
	return DR_null;
}
