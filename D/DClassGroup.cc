#include "DClassGroup.h"

#include <iostream.h>

static char rcsid[] = "$Id: DClassGroup.cc,v 1.2 1998/11/20 18:15:51 holtrf Exp $";

// ************************
// * DR_ClassGroup
// ************************

#define _hADD 6382692
#define _hCREATE 387409249


DR_ClassGroup::DR_ClassGroup(D *d) : DR_Object(d)
{

}

DR_ClassGroup::DR_ClassGroup(const DRef& ref) : DR_Object(ref)
{

}

// If you redefine dump() or Recycle(), you must
// call dump() from this destructor because DRef::dump(), invoked
// from DRef::~DRef(), may call Recycle() - and destructors do not
// call functions in derived classes. otherwise, don't call it.
DR_ClassGroup::~DR_ClassGroup()
{

}

DO_ClassGroup* DR_ClassGroup::safe_get()
{
	SAFE_GET(DO_ClassGroup);	// D_macros.h
}

// rarely used.
// _set() when safe_get() is used avoids double checking.
DO_ClassGroup* DR_ClassGroup::safe_set(D* d)
{
	SAFE_SET(DO_ClassGroup,d);	// D_macros.h
}

DO_ClassGroup *DR_ClassGroup::New()
{
	// TEMPORARY!
	return safe_set(new DO_ClassGroup());
}

// ************************
// * DO_ClassGroup
// ************************

// C++ constructor
// use init() for individual object initialization
DO_ClassGroup::DO_ClassGroup()
{

}

// C++ destructor
// use destroy() for individual object de-construction
DO_ClassGroup::~DO_ClassGroup()
{
}

// init(): the "constructor"
void DO_ClassGroup::init()
{
	// initialize superclass first
	DO_Object::init();
//	add("stringToLibrary", stringToLibrary.New());

}

// destroy(): the "destructor"
void DO_ClassGroup::destroy()
{
	stringToLibrary.dump();

	// destroy superclass last
	DO_Object::destroy();
}

DR_String DO_ClassGroup::toString()
{
	return DR_String("ClassGroup toString() not implemented");
}

// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
DRef DO_ClassGroup::route(DR_Message m)
{
    // until the semantics of route() are nailed down,
	// its body is commented out in the template text.
	// for the moment it simply bounces the message
	// to its superclass.
/*
	switch(m.messageID())
	{
		case _hADD: // RSL: void add( String classname, Library lib);
			return add( DR_String(m("classname")), DR_Library(m("lib")) );
		case _hCREATE: // RSL: Object create( String classname);
			return create( DR_String(m("classname")) );


		default: ;
	}
*/
	return DO_Object::route(m);
}

/**
	add
	 add a name to library mapping for a class 

	In RSL: void addClass( String classname, Library lib);
*/
DR_void DO_ClassGroup::addClass( DR_String classname, const DR_Library& lib )
{
#ifdef DMEMORY
	cerr << "ClassGroup.add(`" << classname << "') for library `"
		<< lib->libname << "'..\n";
#endif

	stringToLibrary->add(classname, lib);
	return DR_null;
}


DR_void DO_ClassGroup::addClass ( const DR_Class& classobj )
{
	DO_Class *doc = classobj.const_get();
	if (!doc)
		return DR_null;

	DR_String clname = doc->className(); 

#ifdef DMEMORY
	cerr << "ClassGroup.add(`" << classname << "') for its class object.\n";
#endif

	stringToClass->add(clname, classobj);

	return DR_null;

}


/**
	create
	 create an object of the given class 

	In RSL: Object create( String classname);
*/
DRef DO_ClassGroup::create( const DR_String& classname )
{
#ifdef DMEMORY
	cerr << "ClassGroup.create(`" << classname << "')...\n";
#endif

	DRef classOrLibrary;

	// Look in Class dictionary first.

	classOrLibrary = stringToClass->get(classname);

	if (classOrLibrary.isValid())
		return createFromClass(DR_Class(classOrLibrary));

	// Not a valid object, so look in library dictionary
	classOrLibrary = stringToLibrary->get(classname);

	if (classOrLibrary.isValid())
		return createFromLib(DR_Library(classOrLibrary), classname);

	cerr << "Class name `" << classname  << "' not found in class group.\n";

	return DR_null;
}

// createFromClass
// Given a class object, let it create what it wants to.
DRef DO_ClassGroup::createFromClass(DR_Class theClass)
{
	if (theClass.const_get())
		return theClass->New();

	cerr << "ClassGroup: invalid Class object.\n";

	return DR_null;
}

// createFromLib
// Given a library object which supposedly knows about the given
// classname, ask it to create one.
DRef DO_ClassGroup::createFromLib(DR_Library theLib, DR_String classname)
{
	if (theLib.const_get())
	{
#ifdef DMEMORY
		cerr << "found `" << classname
			 << "' in library `" << theLib->libname << "', c++ class `"
			<< theLib->cpp_classname() << "', creating...\n";
#endif

		DRef theobj = theLib->create(classname);
		
#ifdef DMEMORY
		if (theobj.isValid()) // means theobj.unsafe_get() is not null
		{
			cerr << "Got an object of c++ class `"
				<< theobj.unsafe_get()->cpp_classname() << "'..\n";
		}
		else
			cerr << "apparently, there is no object.\n";
#endif
		
		return theobj;
	}
	else
		cerr << "can't find `" << classname
			 << "' in the class dictionary.\n";
	
	return DR_null;
}


