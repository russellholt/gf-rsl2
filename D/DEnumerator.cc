#include "DCollection.h"

#include <iostream.h>

static char rcsid[] = "$Id: DEnumerator.cc,v 1.1 1998/11/12 18:30:18 holtrf Exp $";

// ************************
// * DR_Enumerator
// ************************

#define _hHASMOREELEMENTS 84019309
#define _hNEXTELEMENT 1315399961


DR_Enumerator::DR_Enumerator(D *d) : DR_Object(d)
{

}

DR_Enumerator::DR_Enumerator(const DRef& ref) : DR_Object(ref)
{

}

DR_Enumerator::DR_Enumerator(DR_Collection collection)
	: DR_Object(collection->elements())
{

}

DR_Enumerator::~DR_Enumerator()
{
	// must call dump() because we override Recycle()
	dump();
}

DO_Enumerator* DR_Enumerator::safe_get()
{
	D *d = unsafe_get();
	if (d)
	{
		DO_Enumerator *x = dynamic_cast<DO_Enumerator *> (d);
		if (x)
			return x;
	}

	return New();
}

DO_Enumerator* DR_Enumerator::safe_set(D* d)
{
	SAFE_SET(DO_Enumerator,d);
}

DO_Enumerator *DR_Enumerator::New()
{
	cerr << "can't allocate an abstract DO_Enumerator!\n";
	return (DO_Enumerator *)0;
}

DO_Enumerator *DR_Enumerator::New(DR_Collection collection)
{
	dump(collection->elements());
	return safe_get();
}

void DR_Enumerator::Recycle()
{
#ifdef DMEMORY
	cerr << "DR_Enumerator::Recycle()\n";
#endif

	if (unsafe_get())
		delete unsafe_get();

	_unsafe_clear();
}

// ************************
// * DO_Enumerator
// ************************

// C++ constructor
// use init() for individual object initialization
DO_Enumerator::DO_Enumerator()
{

}

// C++ destructor
// use destroy() for individual object de-construction
DO_Enumerator::~DO_Enumerator()
{
}

// init(): the "constructor"
void DO_Enumerator::init()
{
	// initialize superclass first
	DO_Object::init();

}

// destroy(): the "destructor"
void DO_Enumerator::destroy()
{

	// destroy superclass last
	DO_Object::destroy();
}

//// ******************************************
//// route()
////
//// the message-oriented interface.
//// Transforms messages into function calls.
//// ******************************************
//DRef DO_Enumerator::route(DR_Message m)
//{
//	switch(m.messageID())
//	{
//		case _hHASMOREELEMENTS:
//			return hasMoreElements(  );
//		case _hNEXTELEMENT:
//			return nextElement(  );
//
//
//		default: ;
//	}
//	return DO_Object::route(m);
//}


///**
//	hasMoreElements
//	
//
//	In RSL: Boolean hasMoreElements( );
//*/
//DR_Boolean DO_Enumerator::hasMoreElements(  )
//{
//	return DR_null;
//}
///**
//	nextElement
//	
//
//	In RSL: Object nextElement( );
//*/
//DR_Object DO_Enumerator::nextElement(  )
//{
//	return DR_null;
