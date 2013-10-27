#include "DSortedBinaryTree.h"

static char rcsid[] = "$Id: DSortedBinaryTree.cc,v 1.1 1998/11/12 18:31:20 holtrf Exp $";

// ************************
// * DR_SortedBinaryTree
// ************************

#define _hADD 6382692


DR_SortedBinaryTree::DR_SortedBinaryTree(D *d) : DR_Object(d)
{

}

DR_SortedBinaryTree::DR_SortedBinaryTree(const DRef& ref) : DR_Object(ref)
{

}

// If you redefine dump() or Recycle(), you must
// call dump() from this destructor because DRef::dump(), invoked
// from DRef::~DRef(), may call Recycle() - and destructors do not
// call functions in derived classes. otherwise, don't call it.
DR_SortedBinaryTree::~DR_SortedBinaryTree()
{

}

// const_get()
// get the object as the correct type, but without the potential
// to modify it if it is null.
DO_SortedBinaryTree* DR_SortedBinaryTree::const_get() const
{
	return dynamic_cast<DO_SortedBinaryTree *> (unsafe_get());
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
DO_SortedBinaryTree* DR_SortedBinaryTree::safe_get()
{
	SAFE_GET(DO_SortedBinaryTree);	// defined in D_macros.h
}

// rarely used.
// replace() when safe_get() will be used avoids double checking.
DO_SortedBinaryTree* DR_SortedBinaryTree::safe_set(D* d)
{
	SAFE_SET(DO_SortedBinaryTree,d);	// defined in D_macros.h
}

DO_SortedBinaryTree *DR_SortedBinaryTree::New()
{
	DO_SortedBinaryTree *dobj = DO_SortedBinaryTree::New().const_get();
	replace(dobj);

	return dobj;
}

// ************************
// * DO_SortedBinaryTree
// ************************

// C++ constructor
// use init() for individual object initialization
DO_SortedBinaryTree::DO_SortedBinaryTree()
{

}

// C++ destructor
// use destroy() for individual object de-construction
DO_SortedBinaryTree::~DO_SortedBinaryTree()
{
}

// init(): the "constructor"
void DO_SortedBinaryTree::init()
{
	// must initialize superclass *first*
	DO_Composite::init();



}

// destroy(): the "destructor"
void DO_SortedBinaryTree::destroy()
{
	root.dump();


	// must destroy superclass *last*
	DO_Composite::destroy();
}

// ********************************************************

// DOC_SortedBinaryTree: the SortedBinaryTree class class
// By defining this class in the .cc file, it is
// unavailable anywhere else. This is good, for now.
class DOC_SortedBinaryTree : public DO_Class {
	D *spawn();

  public:
	DOC_SortedBinaryTree() : DO_Class("SortedBinaryTree") { }

};

DR_Class DO_SortedBinaryTree::SortedBinaryTreeclass = new DOC_SortedBinaryTree();

// spawn()
// The only place DO_SortedBinaryTree objects should be created.
// This function is private; it is called from DO_Class::New().
D *DOC_SortedBinaryTree::spawn()
{
	return new DO_SortedBinaryTree();
}

DR_Class DO_SortedBinaryTree::DClass()
{
	return SortedBinaryTreeclass;
}

// New()
// Create a new DO_SortedBinaryTree by asking for one from
// the static class object, DO_SortedBinaryTree::SortedBinaryTreeclass.
// That object will either create one, by calling the virtual
// function spawn(), that is, DOC_SortedBinaryTree::spawn(),
// or getting one from its list of free objects.
//
// A static function
DR_SortedBinaryTree DO_SortedBinaryTree::New()
{
	return DO_SortedBinaryTree::SortedBinaryTreeclass->New();
}

// Create_DOC_SortedBinaryTree()
// not currently in use as of Oct 1, 1998
// It will be used as the "bootstrap" function
// to create the class object, which will in turn
// create and recycle instances of DO_SortedBinaryTree
extern "C" DRef& Create_DOC_SortedBinaryTree()
{
	DO_SortedBinaryTree::SortedBinaryTreeclass = new DOC_SortedBinaryTree();
	return DO_SortedBinaryTree::SortedBinaryTreeclass;
}


// ********************************************************



// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
DRef DO_SortedBinaryTree::route(DR_Message m)
{

	switch(theIDHash(m->message->data()))
	{
		case _hADD: // RSL: void add( Object o);
			return add( DR_Object(m("o")) );


		default: ;
	}
	return DO_Composite::route(m);
}



/**
	add
	

	In RSL: void add( Object o);
*/
DR_void DO_SortedBinaryTree::add(DR_Object o, ref_t t)
{
	addAt(root, o, t);
	return DR_null;
}

DR_void DO_SortedBinaryTree::add (DR_Object o, const DR_comparator& comp, ref_t t )
{
	comparator = comp;
	addAt(root, o, t);

	return DR_null;
}

void DO_SortedBinaryTree::addAt(DR_BinaryTree& bt, DRef& d, ref_t t)
{
	//if (!bt->thing.isValid())
	if (!bt.isValid())
	{
		bt->thing = d;
		return;
	}

	dcompare_t which;
	if (comparator.isValid())
		which = comparator->compare(d, bt->thing);
	else
		which = d.compare(bt->thing);

	if (which == c_less)
		addAt(bt->left, d);
	else	// greater or equal
		addAt(bt->right, d);
}


DR_void DO_SortedBinaryTree::insertContents(const DR_Collection& l, ref_t t)
{
	DO_Collection *doc = l.const_get();
	if (doc)
	{
		DR_Enumerator dre = doc->elements();
		while (dre->hasMoreElements())
		{
			addAt(root, dre->nextElement(), t);
		}
	}

	return DR_null;
}


