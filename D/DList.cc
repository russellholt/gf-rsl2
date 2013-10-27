// $Id: DList.cc,v 1.2 1998/12/14 15:29:55 holtrf Exp $

#include "DList.h"
#include "DClass.h"

#include "DSortedBinaryTree.h"

#include <iostream.h>

// ************************
// * DR_List
// ************************
static char rcsid[] = "$Id: DList.cc,v 1.2 1998/12/14 15:29:55 holtrf Exp $";

// List
#define _hAPPEND 252997733
#define _hELEMENTS 135454


// ********************************************************


// DOC_List: the List class class
class DOC_List : public DO_Class {
	D *spawn();

  public:
	DOC_List() : DO_Class("List") { }

};

// Create_DOC_List() not currently in use
extern "C" DRef& Create_DOC_List()
{
	DO_List::Listclass = new DOC_List();
	return DO_List::Listclass;
}

DR_Class DO_List::Listclass = new DOC_List();


// The only place DO_List objects should be created.
// This function is private; it is called from
// DO_Class::create().
D *DOC_List::spawn()
{
#ifdef DMEMORY
	cerr << "DOC_List::spawn()\n";
#endif
	return new DO_List();
}

DRef DO_List::Class()
{
	return Listclass;
}

// New()
// A static function
DR_List DO_List::New()
{
	// this calls the function
	// D* DO_Class::New()
	// so returning a D* invokes the constructor
	// DR_List(D *)
	return DO_List::Listclass->New();
}

// ********************************************************


DR_List::DR_List(D *d) : DR_Collection(d)
{

}

DR_List::DR_List(const DRef& ref) : DR_Collection(ref)
{

}

DR_List::~DR_List()
{
#ifdef DMEMORY
	cerr << "\t~DR_List();\n";
#endif
}

DO_List* DR_List::const_get()
{
	return dynamic_cast<DO_List *> (unsafe_get());
}

DO_List* DR_List::safe_get()
{
	SAFE_GET(DO_List);
}

DO_List* DR_List::safe_set(D* d)
{
	SAFE_SET(DO_List,d);
}

DO_List *DR_List::New()
{
	DO_List *dobj = DO_List::New().const_get();
	replace(dobj);

	return dobj;
}

// ************************
// * DO_List
// ************************

DO_List::DO_List()
{

}

DO_List::~DO_List()
{

}

// init(): the "constructor"
void DO_List::init()
{
	// initialize superclass first
	DO_Object::init();

}

// destroy(): the "destructor"
void DO_List::destroy()
{
	thelist.clear();

	// destroy superclass last
	DO_Object::destroy();
}


void DO_List::assign(const DRef& obj)
{
	DR_List dl = DR_List(obj);

	if (dl.const_get())
	{
		clear();
		mergeAppend(dl);	
	}
}

// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
/*
DRef DO_List::route(DR_Message m)
{
	switch(m.messageID())
	{
		case _hAPPEND:
			return append( DR_Object(m[0]) );
		case _hELEMENTS:
			return elements(  );


		default: ;
	}
	return DO_Object::route(m);
}
*/

/**
	append

	In RSL: void append( Object o);
*/
DR_void DO_List::append( DR_Object o, ref_t t )
{
	if (t == secondary_ref)
		thelist.append(DRef(o, t));
	else
		thelist.append(o);

	return DR_null;
}

/**
	prepend

	In RSL: void prepend( Object o);
*/
DR_void DO_List::prepend( DR_Object o, ref_t t )
{
	if (t == secondary_ref)
		thelist.prepend(DRef(o, t));
	else
		thelist.prepend(o);

	return DR_null;
}

DO_Enumerator *DO_List::elements()
{
#ifdef DMEMORY
	cerr << "DO_List::elements()\n";
#endif

	return new DO_ListEnumerator(thelist);
}


DR_void DO_List::mergeAppend(DR_Collection dc, ref_t t)
{
	DR_Enumerator dre = dc->elements();
	while(dre->hasMoreElements())
		append(dre->nextElement(), t);

	return DR_null;
}

DR_void DO_List::mergePrepend(DR_Collection dc, ref_t t)
{
	DR_Enumerator dre = dc->elements();
	while(dre->hasMoreElements())
		prepend(dre->nextElement(), t);

	return DR_null;
}

void DO_List::sort()
{
	DR_SortedBinaryTree drsb;

	// Remove each item from thelist and
	// add it to the sorted binary tree.
	// Because no comparator is specified, the
	// default compare() method is used.
	while(!thelist.isEmpty())
		drsb->add(thelist.get());

	// append each item from the inOrder() traversal.
	mergeAppend(drsb->root->inOrder());
}

void DO_List::sort(DRef comparator)
{
	DR_comparator comp = comparator;

	// if invalid comparator is specified, then
	// do the default sort.
	if (!comp.const_get())
		sort();

	DR_SortedBinaryTree drsb;
	drsb->comparator = comp;

	// Remove each item from theList and
	// add it to the sorted binary tree.
	while(!thelist.isEmpty())
		drsb->add(thelist.get());

	// append each item from the inOrder() traversal.
	mergeAppend(drsb->root->inOrder());
}


DRef DO_List::find(const DR_Composite& what, const DRef& comparator)
{
	DR_comparator with = comparator;
	if (with.const_get())
	{
		// Walk the list and compare against each one
		DR_Enumerator e = elements();
		DRef elem;

		if (e.isValid())
			while (e->hasMoreElements())
			{
				elem = e->nextElement();
				if (elem.isValid() && with->compare(what, elem) == c_equal)
					return elem;
			}
	}

	return DNULL;
}

// find()
// interface to find(composite, comparator)
DRef DO_List::find(const DR_String& aName, const DRef& aValue)
{
	// Create a composite, any composite with the name and value
	// as its one and only relevant attribute.
	DR_Composite dc;
	dc->add(aName, aValue);

	// create a comparator to use
	DR_comparator c;
	c << aName;

	return find(dc, c);
}

// ************************
// * DO_ListEnumerator
// ************************

// ListEnumerator
#define _hHASMOREELEMENTS 84019309
#define _hNEXTELEMENT 1315399961

// C++ constructor
// use init() for individual object initialization
//DO_ListEnumerator::DO_ListEnumerator()
//{
////	iter=0;
//}

// C++ destructor
// use destroy() for individual object de-construction
DO_ListEnumerator::~DO_ListEnumerator()
{
#ifdef DMEMORY
	cerr << "\tDO_ListEnumerator::~DO_ListEnumerator()\n";
#endif
}

// init(): the "constructor"
void DO_ListEnumerator::init()
{
	// initialize superclass first
	DO_Enumerator::init();
}

// destroy(): the "destructor"
void DO_ListEnumerator::destroy()
{
	// destroy superclass last
	DO_Enumerator::destroy();
}

DR_String DO_ListEnumerator::toString()
{
	return DR_String(DNULL);
}

// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
/*
DRef DO_ListEnumerator::route(DR_Message m)
{
	switch(m.messageID())
	{
		case _hHASMOREELEMENTS:
			return hasMoreElements(  );
		case _hNEXTELEMENT:
			return nextElement(  );

		default: ;
	}
	return DO_Enumerator::route(m);
}
*/



