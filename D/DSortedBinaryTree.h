/************************************************************************

	A sorted binary tree extends the basic a binary tree
	to provide more useful insertion


SortedBinaryTree is
	DR_SortedBinaryTree - the "smart pointer" reference to the implementation
	DO_SortedBinaryTree - the implementation

  and probably
	DOC_SortedBinaryTree - class object (referred to through DR_Class)


$Id: DSortedBinaryTree.h,v 1.1 1998/11/12 18:53:05 holtrf Exp $
************************************************************************/

// Translation Notes: (delete if desired)
// // Loading templates from directory "/dest/applied/gf2.5/Releases/gf/gf-2.5a3/packages/templatizer/templates/Composite"
// method void add( Object o);
//   generating.
// declaration: BinaryTree root;
//   generating.


#include "DComposite.h"
#include "DBinaryTree.h"
#include "Dcomparator.h"

#ifndef _D_SortedBinaryTree_
#define _D_SortedBinaryTree_

#define _D_SortedBinaryTree_ID 202909441

class DO_SortedBinaryTree;

// a DR_Object:DRef class - a strongly-typed smart pointer
class DR_SortedBinaryTree : public DR_Object {
public:
	DR_SortedBinaryTree (D *d=0);
	DR_SortedBinaryTree (const DRef& ref);
	virtual ~DR_SortedBinaryTree();

	DO_SortedBinaryTree *const_get() const;
	DO_SortedBinaryTree *safe_get();
	DO_SortedBinaryTree *safe_set(D* d);
	DO_SortedBinaryTree *New();

	inline DO_SortedBinaryTree *operator->() { return safe_get(); }
};


// a DO_Object class - the guts
class DO_SortedBinaryTree : public DO_Composite {

public:
	DR_BinaryTree root ;
	DR_comparator comparator;

	DO_SortedBinaryTree();
	DO_SortedBinaryTree(DRef r); // casting constructor
	virtual ~DO_SortedBinaryTree();

	static DR_Class SortedBinaryTreeclass;
	static DR_SortedBinaryTree New();
	DR_Class DClass();

	DRef route(DR_Message m);
	void init();
	void destroy();

	// Methods generated from SortedBinaryTree.rsl
	DR_void add ( DR_Object o , ref_t=primary_ref);
	DR_void add ( DR_Object o, const DR_comparator& comp , ref_t=primary_ref);

	DR_void insertContents(const DR_Collection& l, ref_t=primary_ref);

protected:
	void addAt(DR_BinaryTree& bt, DRef& d, ref_t t=primary_ref);
};

inline DR_SortedBinaryTree& operator<<(DR_SortedBinaryTree& t, DR_Composite& c)
	{ t->add(c); return t; }
	
#endif

