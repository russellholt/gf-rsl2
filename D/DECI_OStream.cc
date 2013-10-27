#include "DECI_OStream.h"

#include "DInt.h"
#include "DBool.h"

static char rcsid[] = "$Id: DECI_OStream.cc,v 1.4 1998/12/03 22:08:01 holtrf Exp $";

static RWCString outgoing_escapes(const RWCString& s);
static RWCString incoming_escapes(const RWCString& s);

// ************************
// * DR_ECI_OStream
// ************************

#define _hSTREAM 303657573
#define _hCOMPOSITE 1763443716
#define _hCOLLECTION 1768036357

DR_ECI_OStream::DR_ECI_OStream(D *d) : DR_OStream(d)
{

}

DR_ECI_OStream::DR_ECI_OStream(const DRef& ref) : DR_OStream(ref)
{

}

// If you redefine dump() or Recycle(), you must
// call dump() from this destructor because DRef::dump(), invoked
// from DRef::~DRef(), may call Recycle() - and destructors do not
// call functions in derived classes. otherwise, don't call it.
DR_ECI_OStream::~DR_ECI_OStream()
{

}

// const_get()
// get the object as the correct type, but without the potential
// to modify it if it is null.
DO_ECI_OStream* DR_ECI_OStream::const_get() const
{
	return dynamic_cast<DO_ECI_OStream *> (unsafe_get());
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
DO_ECI_OStream* DR_ECI_OStream::safe_get()
{
	SAFE_GET(DO_ECI_OStream);	// defined in D_macros.h
}

// rarely used.
// replace() when safe_get() will be used avoids double checking.
DO_ECI_OStream* DR_ECI_OStream::safe_set(D* d)
{
	SAFE_SET(DO_ECI_OStream,d);	// defined in D_macros.h
}

DO_ECI_OStream *DR_ECI_OStream::New()
{
	DO_ECI_OStream *dobj = DO_ECI_OStream::New().const_get();
	replace(dobj);

	return dobj;
}

// ************************
// * DO_ECI_OStream
// ************************

// C++ constructor
// use init() for individual object initialization
DO_ECI_OStream::DO_ECI_OStream()
{

}

// C++ destructor
// use destroy() for individual object de-construction
DO_ECI_OStream::~DO_ECI_OStream()
{
}

// init(): the "constructor"
void DO_ECI_OStream::init()
{
	// initialize superclass first
	DO_OStream::init();

}

// destroy(): the "destructor"
void DO_ECI_OStream::destroy()
{

	// destroy superclass last
	DO_OStream::destroy();
}

DR_String DO_ECI_OStream::toString()
{
	return DR_String("ECI_OStream toString() not implemented");
}

// ********************************************************

// DOC_ECI_OStream: the ECI_OStream class class
// By defining this class in the .cc file, it is
// unavailable anywhere else. This is good, for now.
class DOC_ECI_OStream : public DO_Class {
	D *spawn();

  public:
	DOC_ECI_OStream() : DO_Class("ECI_OStream") { }

};

DR_Class DO_ECI_OStream::ECI_OStreamclass = new DOC_ECI_OStream();
//DR_Class DO_ECI_OStream::ECI_OStreamclass;

// The only place DO_ECI_OStream objects should be created.
// This function is private; it is called from
// DO_Class::New().
D *DOC_ECI_OStream::spawn() {
	return new DO_ECI_OStream();
}

DRef DO_ECI_OStream::Class()
{
	return ECI_OStreamclass;
}

// New()
// Create a new DO_ECI_OStream by asking for one from
// the static class object, DO_ECI_OStream::ECI_OStreamclass.
// That object will either create one, by calling the virtual
// function spawn(), that is, DOC_ECI_OStream::spawn(),
// or getting one from its list of free objects.
//
// A static function
DR_ECI_OStream DO_ECI_OStream::New()
{
	return DO_ECI_OStream::ECI_OStreamclass->New();
}

// Create_DOC_ECI_OStream()
// not currently in use as of Oct 1, 1998
// It will be used as the "bootstrap" function
// to create the class object, which will in turn
// create and recycle instances of DO_ECI_OStream
extern "C" DRef& Create_DOC_ECI_OStream()
{
	DO_ECI_OStream::ECI_OStreamclass = new DOC_ECI_OStream();
	return DO_ECI_OStream::ECI_OStreamclass;
}


// ********************************************************



// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
DRef DO_ECI_OStream::route(DR_Message m)
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
	stream a dref

	Five cases:

		1. composite: use the composite() method
		2. Bool: use the toString() method, which produces
			valid ECI as ``true'' or ``false''
		3. Int: same as Bool.
		4. collection: use the collection() method
		5. all others get a quoted via toString(), such as Date.
*/
DR_String DO_ECI_OStream::stream( const DRef& ref)
{
	D *d = ref.unsafe_get();
	DR_String s;
	if (d)
	{
		if (dynamic_cast<DO_Composite *> (d) )
			s << composite(ref);

		else
		if (dynamic_cast<DO_Bool *> (d) )
			s << d->toString();
		else
		if (dynamic_cast<DO_Int *> (d) )
			s << d->toString();

		else
		if (dynamic_cast<DO_Collection *> (d) )
			s << collection(ref);

		else // use the toString() method and quote it!
			s << "\"" << outgoing_escapes(d->toString().val()).data() << "\"";

	}

	return s;
}

/**
	stream a composite
	composite ::= classname { memberlist }

	memberlist ::= val [, memberlist]
	val ::= keyname : dref
*/
DR_String DO_ECI_OStream::composite( const DR_Composite& obj )
{
	DO_Composite *doc = obj.const_get();
	if (doc)
	{
		DR_String s = doc->DClass()->className() + " { "; 

		DR_KeyedEnumerator e = doc->elements();
		int first=1;
		if (e.isValid())
			while (e->hasMoreElements())
			{
				if (!first)
					s += ", ";
				else
					first = 0;

				s << e->nextKey() << ": " << stream(e->nextElement());
			}

		return s << " }";
	}

	return DR_null;
}

/**
	stream a collection
	collection ::= classname { memberlist }
	memberlist ::= dref [, memberlist]

*/
DR_String DO_ECI_OStream::collection ( const DR_Collection& obj)
{
	DO_Collection *doc = obj.const_get();
	if (doc)
	{
		DR_String s = DR_Class(doc->Class())->className() + " { ";

		DR_Enumerator e = doc->elements();
		int first=1;
		if (e.isValid())
			while (e->hasMoreElements())
			{
				if (!first)
					s += ", ";
				else
					first = 0;

				s << stream(e->nextElement());
			}

		return s << " }";
	}

	return DR_null;
}

/**
	translate control chars into escape sequences
	how many times have i written this @#$! function?
	-russell
*/
RWCString outgoing_escapes(const RWCString& s)
{
	size_t more_chars = s.length();
	RWCString sub;
	const char *c  = s.data();

	for( ; more_chars; c++, --more_chars)
	{
		if (!c) break;
		switch( *c )
		{
			case '\n': sub += "\\n"; break;
			case '\r': sub += "\\r"; break;
			case '\t': sub += "\\t"; break;
			case '\\': sub += "\\\\"; break;

			default: sub += char (*c);
		}
	}
	return sub;
}

/**
	translate escape sequences into control chars
	should move to DO_ECI_IStream
*/
RWCString incoming_escapes(const RWCString& s)
{
	RWCString sub;
	size_t len = s.length();
	const char *c = s.data();

	for ( ; c++, len; len--)
	{
		if (!c) break;

		if (*c == '\\' && len>1)
		{
			if (!c) break;

			switch ( *c )
			{
				case 'n':
					sub += '\n'; break;
				case 't':
					sub += '\t'; break;
				case 'r':
					sub += '\r'; break;
				case '\\':
					sub += '\\'; break;

				default:
					sub += char(*c);
			}
		}
		else
			 if (c)
			 	sub += char(*c);
	}

	return sub;
}

