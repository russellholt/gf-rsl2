//
// $Id: DrefDictionary.cc,v 1.1 1998/11/24 22:19:14 toddm Exp $
//

#include "DrefDictionary.h"
#include "DClass.h"

#include <iostream.h>

// ************************
// * DR_refDictionary
// ************************
// $Id: DrefDictionary.cc,v 1.1 1998/11/24 22:19:14 toddm Exp $

// refDictionary
#define _hADD 6382692
#define _hAPPEND 252997733
#define _hELEMENTS 135454
#define _hREMOVE 67136879
#define _hSIZE 1936292453


DR_refDictionary::DR_refDictionary(D *d) : DR_KeyedCollection(d)
{

}

DR_refDictionary::DR_refDictionary(const DRef& ref) : DR_KeyedCollection(ref)
{

}

DR_refDictionary::~DR_refDictionary()
{

}

DO_refDictionary *DR_refDictionary::const_get() const
{
	return dynamic_cast<DO_refDictionary *> (unsafe_get());
}

DO_refDictionary* DR_refDictionary::safe_get()
{
	SAFE_GET(DO_refDictionary);
}

DO_refDictionary* DR_refDictionary::safe_set(D* d)
{
	SAFE_SET(DO_refDictionary,d);
}

DO_refDictionary *DR_refDictionary::New()
{
	DO_NEW(DO_refDictionary);
}

// ************************
// * DO_refDictionary
// ************************

DO_refDictionary::DO_refDictionary()
{

}

DO_refDictionary::~DO_refDictionary()
{

}

// init(): the "constructor"
void DO_refDictionary::init()
{
	// initialize superclass first
	DO_Object::init();

}

// destroy(): the "destructor"
void DO_refDictionary::destroy()
{
	table.clear();

	// destroy superclass last
	DO_Object::destroy();
}



// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
DRef DO_refDictionary::route(DR_Message m)
{
	return DO_Dictionary::route(m);
}



/**
	add
	In RSL: void add( Object o);
*/
DR_void DO_refDictionary::add( DR_Object o, ref_t t)
{
	return DO_Dictionary::add(o, t);
}

// add
// In RSL: void add( String key, Object o);
DR_void DO_refDictionary::add(DR_String& key, const DRef& o, ref_t t )
{
	//table.insertKeyAndValue(key(), DRef(o, t));

	// insert the DRef itself!
	table.insertKeyAndValue(key(), DRef((D*)(&o), t));

	return DR_null;
}

DRef DO_refDictionary::get(const DR_String& key)
{
	// findValue(const K& key, V& retVal) const;
	DRef dref;
	table.findValue(key.const_val(), dref);

	if (dref.isValid())
	{
		// do we point to a DRef?
		DRef *pdref = dynamic_cast<DRef *> (dref.unsafe_get());
		if (pdref)
			return *(pdref); // return the dref we point to (which is copied)
	}

	return dref;
}

DR_void DO_refDictionary::rw_add (const RWCString key, const DRef& dref, ref_t t )
{
	//table.insertKeyAndValue(key, DRef(dref, t));

	// insert the DRef itself!
	table.insertKeyAndValue(key, DRef((D*)(&dref), t));

	return DR_null;
}

DRef DO_refDictionary::rw_get (const RWCString key )
{
	// findValue(const K& key, V& retVal) const;
	DRef dref;
	table.findValue(key, dref);

	if (dref.isValid())
	{
		// do we point to a DRef?
		DRef *pdref = dynamic_cast<DRef *> (dref.unsafe_get());
		if (pdref)
			return *(pdref); // return the dref we point to (which is copied)
	}

	return dref;
}

DR_void DO_refDictionary::cc_add ( const char *key, const DRef& dref, ref_t t )
{
	//table.insertKeyAndValue(RWCString(key), DRef(dref, t));

	// insert the DRef itself!
	table.insertKeyAndValue(RWCString(key), DRef((D*)(&dref), t));

	return DR_null;
}

DRef DO_refDictionary::cc_get ( const char *key )
{
	DRef dref;
	table.findValue(RWCString(key), dref);

	if (dref.isValid())
	{
		// do we point to a DRef?
		DRef *pdref = dynamic_cast<DRef *> (dref.unsafe_get());
		if (pdref)
			return *(pdref); // return the dref we point to (which is copied)
	}

	return dref;
}


// DOC_refDictionary: the refDictionary class class
class DOC_refDictionary : public DO_Class {
	D *spawn();

  public:
	DOC_refDictionary() : DO_Class("refDictionary") { }

};


DR_Class DO_refDictionary::refDictionaryclass = new DOC_refDictionary();

// The only place DO_refDictionary objects should be created.
// This function is private; it is called from
// DO_Class::create().
D *DOC_refDictionary::spawn() {
	return new DO_refDictionary();
}

DRef DO_refDictionary::Class()
{
	return refDictionaryclass;
}

DR_refDictionary DO_refDictionary::New()
{
	// this calls the function
	// D* DO_Class::New()
	// so returning a D* invokes the constructor
	// DR_refDictionary(D *)
	return DO_refDictionary::refDictionaryclass->New();
}

DRef DO_refDictionary::deepCopy() const
{
	cerr << "DO_refDictionary::deepCopy() not implemented.\n";
	return DR_null;
}

DR_void DO_refDictionary::remove(DR_String& key)
{
	if (key.isValid())
		table.remove(key.val());
	return DR_null;
}

DR_void DO_refDictionary::cc_remove(const char *c)
{
	if (c)
		table.remove(RWCString(c));
	return DR_null;
}

DR_void DO_refDictionary::rw_remove(const RWCString& key)
{
	table.remove(key);
	return DR_null;
}


// ************************
// * DO_refDictionaryEnumerator
// ************************

// **********************************************************************
// Collection Enumerator
// * If you derive from an existing collection with well-defined, this
// class may not be necessary.
//
// * All DO_Enumerator subclasses are used through the
// DR_Enumerator interface class; no DR_refDictionaryEnumerator
// class should be necessary.
// **********************************************************************
class DO_refDictionaryEnumerator : public DO_KeyedEnumerator {
	// enumerator data goes here
	RWTValHashDictionaryIterator<RWCString, DRef> iter;

public:
	DO_refDictionaryEnumerator(RWTValHashDictionary<RWCString, DRef>& t) : iter(t) { }
	virtual ~DO_refDictionaryEnumerator();

	// superclass virtuals
	inline BOOLEAN hasMoreElements() { return (BOOLEAN) (iter()); }
	inline DR_String nextKey() { return DR_String(iter.key()); }
	DRef nextElement();

	DR_String toString();
};

DO_Enumerator *DO_refDictionary::elements()
{
	return new DO_refDictionaryEnumerator(table);
}

// C++ destructor
// use destroy() for individual object de-construction
DO_refDictionaryEnumerator::~DO_refDictionaryEnumerator()
{
#ifdef DMEMORY
	cerr << "\tDO_refDictionaryEnumerator::~DO_refDictionaryEnumerator()\n";
#endif
}


DR_String DO_refDictionaryEnumerator::toString()
{
	return DR_String(DNULL);
}


DRef DO_refDictionaryEnumerator::nextElement()
{
	// The element itself may actually be a DRef;
	// that is, the element in the collection of DRef by value may
	// point to a DRef itself, and in that case we want to return
	// the object itself rather than the DRef.
	// As if this weren't confusing enough, what we return is a
	// DRef wrapper....
	DRef r = iter.value();

	// Is the object that r points to a DRef?
	// that is, is r.unsafe_get() a (DRef *) ?
	DRef *psubr = dynamic_cast<DRef *> (r.unsafe_get());

	// If so, then return what psubr points to, which invokes
	// DRef::DRef(D *) to create the actual return object.
	if (psubr)
		return psubr->unsafe_get();	// could also return *psubr;

	// Otherwise, just return the ref itself.
	return r;
}

