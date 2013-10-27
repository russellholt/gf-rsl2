// $Id: DComposite.cc,v 1.3 1998/12/04 22:24:33 holtrf Exp $

#include "DComposite.h"
#include "DDictionary.h"

#include "DECI_OStream.h"
#include "DrefDictionary.h"

#include "DBool.h"

#include <iostream.h>

// ************************
// * DR_Composite
// ************************
static char rcsid[] = "$Id: DComposite.cc,v 1.3 1998/12/04 22:24:33 holtrf Exp $";

// Composite
#define _hAPPEND 252997733
#define _hGET 6776180
#define _hREMOVE 67136879

#define _hADD 6382692   // add
#define _hADDPRIVATE 1734873649 // addPrivate
#define _hREMOVE 67136879   // remove
#define _hCONTAINS 33947655 // contains
#define _hDOESNOTCONTAIN 738223441  // doesNotContain
#define _hFIND 1718185572   // find
#define _hOpDiv 47  // /

#ifdef PURIFY
#include "purify.h"
#endif


DR_Composite::DR_Composite(D *d) : DR_Collection(d)
{

}

DR_Composite::DR_Composite(const DRef& ref) : DR_Collection(ref)
{

}

DR_Composite::~DR_Composite()
{

}

DO_Composite *DR_Composite::const_get() const
{
	return dynamic_cast<DO_Composite *> (unsafe_get());
}

DO_Composite* DR_Composite::safe_get()
{
	SAFE_GET(DO_Composite);
}

DO_Composite* DR_Composite::safe_set(D* d)
{
	SAFE_SET(DO_Composite,d);
}

DO_Composite *DR_Composite::New()
{
	DO_Composite *dobj = DO_Composite::New().const_get();

	replace(dobj);
	return dobj;
}

DO_Composite *DR_Composite::New(DR_KeyedCollection storage)
{
	DO_Composite *dobj = DO_Composite::New().const_get();
	dobj->setStorage(storage);

	replace(dobj);
	return dobj;
}

// ************************
// * DO_Composite
// ************************

DO_Composite::DO_Composite() : storage((D*)0)
{

}

DO_Composite::~DO_Composite()
{

}

// init(): the "constructor"
void DO_Composite::init()
{
#ifdef DMEMORY
	cerr << "DO_Composite::init()\n";
#endif

	// initialize superclass first
	DO_Object::init();

	// Don't initialize the storage collection.
	// this will be done the first time it is used.
}

size_t DO_Composite::size() const
{
	if (storage.isValid())
	{
		DO_KeyedCollection *dok = storage.const_get();
		return dok->size();
	}

	return 0;
}

void DO_Composite::setStorage(DR_KeyedCollection drk, ref_t t)
{
	storage.replace(drk.unsafe_get(), t);
}

// destroy(): the "destructor"
void DO_Composite::destroy()
{

#ifdef PURIFY
	purify_printf("DO_Composite::destroy().");
#endif PURIFY

	storage.dump();

	// destroy superclass last
	DO_Object::destroy();
}



DR_String DO_Composite::toString()
{
	DR_ECI_OStream eout;
	return eout->composite(DR_Composite(this));

/*
	DR_String s = DClass()->className() + " { ";

	if (storage.isValid())
	{
		DR_KeyedEnumerator e(storage->elements());
		int first=1;
		while (e->hasMoreElements())
		{
			if (!first)
				s += ", ";
			else
				first = 0;

			s += e->nextKey();
			s += ": ";
			s += e->nextElement().toString();
		}
	}

	return s += " }";
*/
}
	
DR_void DO_Composite::add (DR_Object o, ref_t t )
{
	cerr << "DO_Composite::add(o) not implemented!\n";
	return DR_null;
}

// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
DRef DO_Composite::route(DR_Message m)
{
#ifdef DMEMORY
	cerr << "DO_Composite::route()\n";
#endif

	switch(m->messageCode)
	{
		case _hOpDiv:	// member access operator
			return get( m("objectName") );
		
		case _hADD: // RSL: void add( String name, Object obj);
			return add( m("name"), m("obj") );

		case _hGET: // RSL: Object get( String name);
			return get( m("name") );

		case _hREMOVE: // RSL: Object remove( String name);
			return remove( m("name") );

		case _hCONTAINS: // contains
			return DR_Bool( get(DR_String(m("objectName"))).isValid() );

		case _hDOESNOTCONTAIN: // doesNotContain
			return DR_Bool( ! get("objectName").isValid() );

		case _hADDPRIVATE: // addPrivate
			return add( m("name"), m("x"), secondary_ref );

		default: ;
	}

	return DO_Collection::route(m);
}

// Composite comparison is comparison of their dictionaries, by default,
// in no specified order, of course... to do so, use Dcomparator.
dcompare_t DO_Composite::compare(const DRef& d) const
{
	DO_KeyedCollection *dok = storage.const_get();
	if (dok)
		return dok->compare(d);

	return c_less;
}

// deepCopy is
// create a new object
// assign it to this.
DRef DO_Composite::deepCopy() const
{
	cerr << "DO_Composite::deepCopy() const not implemented.\n";

	DR_Composite obj = New();
	if (obj.const_get())	// it is a valid DO_Composite
	{
		DRef x = (D*)this;
		obj->assign(x);
	}

	return obj;
}

void DO_Composite::assign(const DRef& obj)
{
	cerr << "DO_Composite::assign() not implemented.\n";
}


/**
	add
	 add an object 

	In RSL: void add( String name, Object obj);
*/
DR_void DO_Composite::add( const DR_String& name, const DRef& obj, ref_t t )
{
	// if storage is invalid when add() is called, we want to
	// create a default storage for Composites: a refDictionary.
	/*
	if (!storage.isValid())
	{
		storage = DO_refDictionary::New();
		cerr << "COMPOSITE TEST: using a refDictionary....\n";
	}
	*/

	if (name.isValid())
	{
		DO_String *dop = name.const_get();

		if (dop)
			storage->cc_add(dop->data(), obj, t);
	}

	return DR_null;
}
/**
	get
	 get an object 

	In RSL: Object get( String name);
*/
DRef DO_Composite::get( const DR_String& name )
{
	if (name.isValid())
	{
		DO_String *dop = name.const_get();

		if (dop && storage.isValid()) // ask isValid() only for efficiency
			return storage->cc_get(dop->data());
	}

	return DR_null;
}

DR_void DO_Composite::add( const char *name, const DRef& obj, ref_t t )
{
	// if storage is invalid when add() is called, we want to
	// create a default storage for Composites: a refDictionary.
	/*
	if (!storage.isValid())
	{
		storage = DO_refDictionary::New();
		cerr << "COMPOSITE TEST: using a refDictionary....\n";
	}
	*/

	storage->cc_add(name, obj, t);
	return DR_null;
}

DRef DO_Composite::get( const char *name )
{
	if (storage.isValid()) // ask isValid() only for efficiency
		return storage->cc_get(name);

	return DR_null;
}


/**
	remove

	In RSL: Object remove( String name);
*/
DR_void DO_Composite::remove( DR_String name )
{
	storage->remove(name);
	return DR_null;
}

DO_Enumerator *DO_Composite::elements()
{
	if (storage.isValid())
		return storage.const_get()->elements();

	return (DO_Enumerator *) 0;
}


/*************************************************************/

// DOC_Composite  is the default, "generic" class for
// generic composite objects, that is, those for which
// there is no specific C++ implementation.
class DOC_Composite : public DO_Class {
	D *spawn();

  public:
	DOC_Composite() : DO_Class("Object") { }

};

DRef DO_Composite::Compositeclass = new DOC_Composite();


D *DOC_Composite::spawn() {
	return new DO_Composite();
}

// New()
// Create a new DO_Composite by asking for one from
// the static class object, DO_Composite::Compositeclass.
// That object will either create one, by calling the virtual
// function spawn(), that is, DOC_Composite::spawn(),
// or getting one from its list of free objects.
//
// A static function
DR_Composite DO_Composite::New()
{
	return DR_Class(DO_Composite::Compositeclass)->New();
}


