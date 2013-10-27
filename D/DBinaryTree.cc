#include "DBinaryTree.h"

static char rcsid[] = "$Id: DBinaryTree.cc,v 1.1 1998/11/12 18:29:14 holtrf Exp $";

// ************************
// * DR_BinaryTree
// ************************

#define _hSETOBJECT 1695486252
#define _hGETOBJECT 1896812844
#define _hSETLEFT 369295436
#define _hSETRIGHT 436345894
#define _hGETLEFT 33751116
#define _hGETRIGHT 235019302
#define _hINORDER 218840434
#define _hPREORDER 34996285
#define _hPOSTORDER 1293752081


DR_BinaryTree::DR_BinaryTree(D *d) : DR_Composite(d)
{

}

DR_BinaryTree::DR_BinaryTree(const DRef& ref) : DR_Composite(ref)
{

}

// If you redefine dump() or Recycle(), you must
// call dump() from this destructor because DRef::dump(), invoked
// from DRef::~DRef(), may call Recycle() - and destructors do not
// call functions in derived classes. otherwise, don't call it.
DR_BinaryTree::~DR_BinaryTree()
{

}

// const_get()
// get the object as the correct type, but without the potential
// to modify it if it is null.
DO_BinaryTree* DR_BinaryTree::const_get() const
{
	return dynamic_cast<DO_BinaryTree *> (unsafe_get());
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
DO_BinaryTree* DR_BinaryTree::safe_get()
{
	SAFE_GET(DO_BinaryTree);	// defined in D_macros.h
}

// rarely used.
// replace() when safe_get() will be used avoids double checking.
DO_BinaryTree* DR_BinaryTree::safe_set(D* d)
{
	SAFE_SET(DO_BinaryTree,d);	// defined in D_macros.h
}

DO_BinaryTree *DR_BinaryTree::New()
{
	DO_BinaryTree *dobj = DO_BinaryTree::New().const_get();
	replace(dobj);

	return dobj;
}

// ************************
// * DO_BinaryTree
// ************************

// C++ constructor
// use init() for individual object initialization
DO_BinaryTree::DO_BinaryTree()
{

}

// C++ destructor
// use destroy() for individual object de-construction
DO_BinaryTree::~DO_BinaryTree()
{
}

// init(): the "constructor"
void DO_BinaryTree::init()
{
	// must initialize superclass *first*
	DO_Composite::init();

	// can't add instance variables because they have
	// no implementations yet..

}

// destroy(): the "destructor"
void DO_BinaryTree::destroy()
{
	thing.dump();
	left.dump();
	right.dump();

	// must destroy superclass *last*
	DO_Composite::destroy();
}

// ********************************************************

// DOC_BinaryTree: the BinaryTree class class
// By defining this class in the .cc file, it is
// unavailable anywhere else. This is good, for now.
class DOC_BinaryTree : public DO_Class {
	D *spawn();

  public:
	DOC_BinaryTree() : DO_Class("BinaryTree") { }

};

DR_Class DO_BinaryTree::BinaryTreeclass = new DOC_BinaryTree();

// spawn()
// The only place DO_BinaryTree objects should be created.
// This function is private; it is called from DO_Class::New().
D *DOC_BinaryTree::spawn()
{
	return new DO_BinaryTree();
}

DR_Class DO_BinaryTree::DClass()
{
	return BinaryTreeclass;
}

// New()
// Create a new DO_BinaryTree by asking for one from
// the static class object, DO_BinaryTree::BinaryTreeclass.
// That object will either create one, by calling the virtual
// function spawn(), that is, DOC_BinaryTree::spawn(),
// or getting one from its list of free objects.
//
// A static function
DR_BinaryTree DO_BinaryTree::New()
{
	return DO_BinaryTree::BinaryTreeclass->New();
}

// Create_DOC_BinaryTree()
// not currently in use as of Oct 1, 1998
// It will be used as the "bootstrap" function
// to create the class object, which will in turn
// create and recycle instances of DO_BinaryTree
extern "C" DRef& Create_DOC_BinaryTree()
{
	DO_BinaryTree::BinaryTreeclass = new DOC_BinaryTree();
	return DO_BinaryTree::BinaryTreeclass;
}


// ********************************************************



// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
DRef DO_BinaryTree::route(DR_Message m)
{
	switch(theIDHash(m->message->data()))
	{
		case _hSETOBJECT: // RSL: void setObject( Object o);
			return setObject( DR_Object(m("o")) );

		case _hGETOBJECT: // RSL: Object getObject( );
			return getObject(  );

		case _hSETLEFT: // RSL: void setLeft( BinaryTree t);
			return setLeft( DR_BinaryTree(m("t")) );

		case _hSETRIGHT: // RSL: void setRight( BinaryTree t);
			return setRight( DR_BinaryTree(m("t")) );

		case _hGETLEFT: // RSL: BinaryTree getLeft( );
			return getLeft(  );

		case _hGETRIGHT: // RSL: BinaryTree getRight( );
			return getRight(  );

		case _hINORDER: // RSL: List inOrder( );
			return inOrder(  );

		case _hPREORDER: // RSL: List preOrder( );
			return preOrder(  );

		case _hPOSTORDER: // RSL: List postOrder( );
			return postOrder(  );

		default: ;
	}
	return DO_Composite::route(m);
}

/**
	inOrder
	
	front end to s_inOrder().
	Create the list object that all invocations of s_inOrder()
	will fill.

	In RSL: List inOrder( );
*/
DR_List DO_BinaryTree::inOrder( )
{
	DR_List l = DO_List::New();

	s_inOrder(l);
	return l;
}

// s_inOrder
// basic in order traversal: left tree, object, right tree.
void DO_BinaryTree::s_inOrder(DR_List& drl)
{
	if (left.isValid())
		left->s_inOrder(drl);

	drl->append(thing);

	if (right.isValid())
		right->s_inOrder(drl);
}

/**
	postOrder
	
	front end to s_postOrder().
	Create the list object that all invocations of s_postOrder()
	will fill.

	In RSL: List postOrder( );
*/
DR_List DO_BinaryTree::postOrder( )
{
	DR_List l = DO_List::New();

	s_postOrder(l);
	return l;
}

// s_postOrder
// basic post order traversal: right tree, object, left tree.
void DO_BinaryTree::s_postOrder(DR_List& drl)
{
	if (right.isValid())
		right->s_postOrder(drl);

	drl->append(thing);

	if (left.isValid())
		left->s_postOrder(drl);
}

/**
	preOrder
	
	front end to s_preOrder().
	Create the list object that all invocations of s_preOrder()
	will fill.

	In RSL: List preOrder( );
*/
DR_List DO_BinaryTree::preOrder( )
{
	DR_List l = DO_List::New();

	s_preOrder(l);
	return l;
}

// s_preOrder
// basic pre order traversal: object (root), left tree, right tree.
void DO_BinaryTree::s_preOrder(DR_List& drl)
{
	drl->append(thing);

	if (right.isValid())
		right->s_preOrder(drl);

	if (left.isValid())
		left->s_preOrder(drl);
}

