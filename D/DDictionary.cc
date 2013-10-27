// $Id: DDictionary.cc,v 1.2 1998/11/20 18:15:09 holtrf Exp $

#include "DDictionary.h"
#include "DClass.h"

#include <iostream.h>

// ************************
// * DR_Dictionary
// ************************
// $Id: DDictionary.cc,v 1.2 1998/11/20 18:15:09 holtrf Exp $

// Dictionary
#define _hADD 6382692
#define _hAPPEND 252997733
#define _hELEMENTS 135454
#define _hREMOVE 67136879
#define _hSIZE 1936292453


DR_Dictionary::DR_Dictionary(D *d) : DR_KeyedCollection(d)
{

}

DR_Dictionary::DR_Dictionary(const DRef& ref) : DR_KeyedCollection(ref)
{

}

DR_Dictionary::~DR_Dictionary()
{

}

DO_Dictionary *DR_Dictionary::const_get() const
{
	return dynamic_cast<DO_Dictionary *> (unsafe_get());
}

DO_Dictionary* DR_Dictionary::safe_get()
{
	SAFE_GET(DO_Dictionary);
}

DO_Dictionary* DR_Dictionary::safe_set(D* d)
{
	SAFE_SET(DO_Dictionary,d);
}

DO_Dictionary *DR_Dictionary::New()
{
	DO_NEW(DO_Dictionary);
}

// ************************
// * DO_Dictionary
// ************************

DO_Dictionary::DO_Dictionary() : table(RWCString::hash, DR_DICT_DEFAULT_SIZE)
{

}

DO_Dictionary::~DO_Dictionary()
{

}

// init(): the "constructor"
void DO_Dictionary::init()
{
	// initialize superclass first
	DO_Object::init();

}

// destroy(): the "destructor"
void DO_Dictionary::destroy()
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
DRef DO_Dictionary::route(DR_Message m)
{
	switch(m->messageCode)
	{
		case _hADD:
			  return add( DR_String(m("key")), m("o") );

		case _hSIZE: // RSL: Int size( );
			return DR_Int(size(  ));

		case _hREMOVE: // RSL: void remove( String key);
			return remove( DR_String(m("key")) );

		default: ;
	}

	return DO_KeyedCollection::route(m);
}



/**
	add
	In RSL: void add( Object o);
*/
DR_void DO_Dictionary::add( DR_Object o, ref_t t)
{
	cerr << "Dictionary::add() requires a key!\n";
	return DR_null;
}

// add
// In RSL: void add( String key, Object o);
DR_void DO_Dictionary::add(DR_String& key, const DRef& o, ref_t t )
{
	table.insertKeyAndValue(key(), DRef(o, t));
	return DR_null;
}

DRef DO_Dictionary::get(const DR_String& key)
{
	DRef dref;
	table.findValue(key.const_val(), dref);
	return dref;
}

DR_void DO_Dictionary::rw_add (const RWCString key, const DRef& dref, ref_t t )
{
	table.insertKeyAndValue(key, DRef(dref, t));

	return DR_null;
}

DRef DO_Dictionary::rw_get (const RWCString key )
{
	// findValue(const K& key, V& retVal) const;
	DRef dref;
	table.findValue(key, dref);
	return dref;
}

DR_void DO_Dictionary::cc_add ( const char *key, const DRef& dref, ref_t t )
{
	table.insertKeyAndValue(RWCString(key), DRef(dref, t));
	return DR_null;
}

DRef DO_Dictionary::cc_get ( const char *key )
{
	DRef dref;
	table.findValue(RWCString(key), dref);
	return dref;
}


DO_Enumerator *DO_Dictionary::elements()
{
	return new DO_DictionaryEnumerator(table);
}


// DOC_Dictionary: the Dictionary class class
class DOC_Dictionary : public DO_Class {
	D *spawn();

  public:
	DOC_Dictionary() : DO_Class("Dictionary") { }

};


DR_Class DO_Dictionary::Dictionaryclass = new DOC_Dictionary();

// The only place DO_Dictionary objects should be created.
// This function is private; it is called from
// DO_Class::create().
D *DOC_Dictionary::spawn() {
	return new DO_Dictionary();
}

DRef DO_Dictionary::Class()
{
	return Dictionaryclass;
}

DR_Dictionary DO_Dictionary::New()
{
	// this calls the function
	// D* DO_Class::New()
	// so returning a D* invokes the constructor
	// DR_Dictionary(D *)
	return DO_Dictionary::Dictionaryclass->New();
}

DRef DO_Dictionary::deepCopy() const
{
	cerr << "DO_Dictionary::deepCopy() not implemented.\n";
	return DR_null;
}

DR_void DO_Dictionary::remove(DR_String& key)
{
	if (key.isValid())
		table.remove(key.val());
	return DR_null;
}

DR_void DO_Dictionary::cc_remove(const char *c)
{
	if (c)
		table.remove(RWCString(c));
	return DR_null;
}
DR_void DO_Dictionary::rw_remove(const RWCString& key)
{
	table.remove(key);
	return DR_null;
}


// ************************
// * DO_DictionaryEnumerator
// ************************

// DictionaryEnumerator
#define _hHASMOREELEMENTS 84019309
#define _hNEXTELEMENT 1315399961

// C++ destructor
// use destroy() for individual object de-construction
DO_DictionaryEnumerator::~DO_DictionaryEnumerator()
{
#ifdef DMEMORY
	cerr << "\tDO_DictionaryEnumerator::~DO_DictionaryEnumerator()\n";
#endif
}


DR_String DO_DictionaryEnumerator::toString()
{
	return DR_String(DNULL);
}


