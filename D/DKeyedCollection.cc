
#include "DKeyedCollection.h"
#include "DDictionary.h"

#include <iostream.h>

// ************************
// * DR_KeyedCollection
// ************************

#define _hADD 6382692
#define _hGET 6776180
static char rcsid[] = "$Id: DKeyedCollection.cc,v 1.2 1998/11/20 18:15:17 holtrf Exp $";


DR_KeyedCollection::DR_KeyedCollection(D *d) : DR_Collection(d) { }

DR_KeyedCollection::DR_KeyedCollection(DRef ref) : DR_Collection(ref) { }

DR_KeyedCollection::~DR_KeyedCollection() { }

DO_KeyedCollection* DR_KeyedCollection::safe_get()
{
	if (!unsafe_get())
	{
		DR_Dictionary dict;
		dict.New();
		replace(dict.unsafe_get());
		
		return dict.safe_get();
	}
	
	// return const_get();
	return dynamic_cast<DO_KeyedCollection *> (unsafe_get());
}

DO_KeyedCollection *DR_KeyedCollection::const_get() const
{
	return dynamic_cast<DO_KeyedCollection *> (unsafe_get());
}

DO_KeyedCollection* DR_KeyedCollection::safe_set(D* d)
{
	SAFE_SET(DO_KeyedCollection,d);
}

// ************************
// * DO_KeyedCollection
// ************************

DO_KeyedCollection::DO_KeyedCollection() { }

DO_KeyedCollection::~DO_KeyedCollection() { }

DR_void DO_KeyedCollection::add ( DR_Object o, ref_t t)
{
	cerr << "KeyedCollection requires key/object pairs so it can't support generic add().\n";
	return DR_null;
}

// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************

DRef DO_KeyedCollection::route(DR_Message m)
{
//	switch(m.messageID())
//	{
//		case _hADD:
//			return add( DR_String(m[0]), DR_Object(m[1]) );
//		case _hGET:
//			return get( DR_String(m[0]) );
//		default: ;
//	}
	return DO_Collection::route(m);
}


// *********************************************************************


// KeyedEnumerator
//
// Note: what should happen during destruction is that
// the ~DR_Enumerator() calls dump() (probably DRef::dump()),
// which may invoke DR_Enumerator::Recycle(). because these functions
// are inherited by DR_KeyedEnumerator, they will work here,
// and the destructors here can be empty.
DR_KeyedEnumerator::DR_KeyedEnumerator(DR_KeyedCollection collection)
	: DR_Enumerator(collection)
{
	
}

DO_KeyedEnumerator* DR_KeyedEnumerator::safe_get()
{
	D *d = unsafe_get();
	if (d)
	{
		DO_KeyedEnumerator *x = dynamic_cast<DO_KeyedEnumerator *> (d);
		if (x)
			return x;
	}

	return dynamic_cast<DO_KeyedEnumerator *> (New());
}

DO_KeyedEnumerator* DR_KeyedEnumerator::safe_set(D* d)
{
	SAFE_SET(DO_KeyedEnumerator,d);
}

DO_Enumerator *DR_KeyedEnumerator::New()
{
	cerr << "can't allocate an abstract DO_KeyedEnumerator!\n";
	return (DO_KeyedEnumerator *)0;
}

DO_Enumerator *DR_KeyedEnumerator::New(DR_Collection collection)
{
	dump(collection->elements());
	return safe_get();
}

/////

