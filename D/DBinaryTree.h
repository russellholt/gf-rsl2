/************************************************************************

	A binary tree is nothing more than a thing with
	two other binary trees.

	class <b>SortedBinaryTree</b> is more interesting.


BinaryTree is
	DR_BinaryTree - the "smart pointer" reference to the implementation
	DO_BinaryTree - the implementation

  and probably
	DOC_BinaryTree - class object (referred to through DR_Class)


$Id: DBinaryTree.h,v 1.1 1998/11/12 18:50:31 holtrf Exp $
************************************************************************/


#include "DComposite.h"

#include "DList.h"

#ifndef _D_BinaryTree_
#define _D_BinaryTree_

#define _D_BinaryTree_ID 1433745939

class DO_BinaryTree;

// a DR_Object:DRef class - a strongly-typed smart pointer
class DR_BinaryTree : public DR_Composite {
public:
	DR_BinaryTree (D *d=0);
	DR_BinaryTree (const DRef& ref);
	virtual ~DR_BinaryTree();

	DO_BinaryTree *const_get() const;
	DO_BinaryTree *safe_get();
	DO_BinaryTree *safe_set(D* d);
	DO_BinaryTree *New();

	inline DO_BinaryTree *operator->() { return safe_get(); }
};

// a DO_Object class - the guts
class DO_BinaryTree : public DO_Composite {

public:
	DRef thing ;
	DR_BinaryTree left, right ;


	DO_BinaryTree();
	DO_BinaryTree(DRef r); // casting constructor
	virtual ~DO_BinaryTree();

	static DR_Class BinaryTreeclass;
	static DR_BinaryTree New();
	DR_Class DClass();

	DRef route(DR_Message m);
	void init();
	void destroy();

	// Methods generated from BinaryTree.rsl
	inline DR_void setObject ( const DRef& o ) { thing = o; return DR_null; }
	inline DRef getObject (  ) { return thing; }
	inline DR_void setLeft ( const DR_BinaryTree& t ) { left = t; return DR_null; }
	inline DR_void setRight ( const DR_BinaryTree& t ) { right = t; return DR_null; }
	inline DR_BinaryTree getLeft (  ) { return left; }
	inline DR_BinaryTree getRight (  ) { return right; }

	DR_List inOrder (  );
	void s_inOrder(DR_List& drl);

	DR_List postOrder (  );
	void s_postOrder(DR_List& drl);

	DR_List preOrder (  );
	void s_preOrder(DR_List& drl);
};
	
#endif


