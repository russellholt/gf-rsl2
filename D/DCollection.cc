// $Id: DCollection.cc,v 1.1 1998/11/12 18:29:43 holtrf Exp $

#include "DCollection.h"

#include <iostream.h>

// ************************
// * DR_Collection
// ************************

static char rcsid[] = "$Id: DCollection.cc,v 1.1 1998/11/12 18:29:43 holtrf Exp $";

#define _hADD 6382692
#define _hELEMENTS 135454
#define _hSIZE 1936292453


DR_Collection::DR_Collection(D *d) : DR_Object(d)
{

}

DR_Collection::DR_Collection(DRef ref) : DR_Object(ref)
{

}



DO_Collection* DR_Collection::const_get() const
{
	return dynamic_cast<DO_Collection *> (unsafe_get());
}

DO_Collection* DR_Collection::safe_get()
{
	DO_Collection *x = dynamic_cast<DO_Collection *> (unsafe_get());
	if (x)
		return x;

	cerr << "DR_Collection::safe_get(): can't instantiate an abstract class.\n";

	return (DO_Collection *)0;
}

DR_Collection::~DR_Collection()
{
	
}

DO_Collection* DR_Collection::safe_set(D* d)
{
	SAFE_SET(DO_Collection,d);
}

//DO_Collection *DR_Collection::New()
//{
//	// TEMPORARY!
//	return safe_set(new DO_Collection());
//}

// ************************
// * DO_Collection
// ************************

DO_Collection::DO_Collection()
{

}

DO_Collection::~DO_Collection()
{
	
}

// init(): the "constructor"
void DO_Collection::init()
{
	// initialize superclass first
	DO_Object::init();

}

// destroy(): the "destructor"
void DO_Collection::destroy()
{

	// destroy superclass last
	DO_Object::destroy();
}

BOOLEAN DO_Collection::toBoolean() const
{
	return (size() > 0);
}

DR_String DO_Collection::toString()
{

	DR_String s= DR_Class(Class())->className() + toString_OPEN;

	if (size() > 0)
	{
		DR_Enumerator dre(elements());

		int first=1;
		while (dre->hasMoreElements())
		{
			if (!first)
				s += toString_SEP;
			else
				first=0;

			s += dre->nextElement().toString();
		}
	}

	return s += toString_CLOSE;
}

DR_Int DO_Collection::_size()
{
	DR_Int i = (int) (size());
	return i;
}

// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
DRef DO_Collection::route(DR_Message m)
{
#ifdef DMEMORY
	cerr << "DO_Collection::route()\n";
#endif

	switch(m->messageCode)
	{
		case _hADD:
			return add( DR_Object(m("o")) );

		case _hSIZE:
			return _size();

		default: ;
	}

	return DO_Object::route(m);
}

// compare()
// generic collection comparator.
dcompare_t DO_Collection::compare(const DRef& d) const
{
	if (!d.isValid())
		return c_less;

	DR_Collection self = (D *) this;

	DR_Enumerator this_en = self->elements();
	DR_Enumerator that_en = DR_Collection(d)->elements();

	if (!this_en.isValid() || !that_en.isValid())
		return c_less;

	dcompare_t comparison = c_equal;
	while (comparison==c_equal
			&& this_en->hasMoreElements()
			&& that_en->hasMoreElements())
	{
		comparison = this_en->nextElement().compare(that_en->nextElement());
	}

	return comparison;
}

///**
//	add
//	
//
//	In RSL: void add( Object o);
//*/
//DR_void DO_Collection::add( DR_Object o )
//{
//	return DR_null;
//}
///**
//	elements
//	
//
//	In RSL: void elements( );
//*/
//DR_void DO_Collection::elements(  )
//{
//	return DR_null;
//}



DRef DO_Collection::deepCopy() const
{
//#ifdef DMEMORY
	cerr << "DO_Collection::deepCopy() not implemented.\n";
//#endif

/*
	DR_Enumerator enum = this->elements();

	if (enum.isValid())
	{
		
	}
*/

	return DR_null;
}

