// main D C++ template
// Translation Notes:
// Loading templates from directory "/dest/home/holtrf/dev/packages/rsl2cpp/templates/D/"
// method void append( Object o);
//   generating.

//
// $Id: DList.h,v 1.2 1998/12/14 15:30:05 holtrf Exp $

/*******************************************

 *******************************************/

#include "DCollection.h"
#include "DComposite.h"

#include <rw/tvslist.h>

#ifndef _D_List_
#define _D_List_

#define _D_List_ID 1281979252

class DO_List;

// the DRef component: smart pointer
class DR_List : public DR_Collection {
  public:
	DR_List (D *d=0);
	DR_List (const DRef& ref);
	virtual ~DR_List();

	DO_List *const_get();
	DO_List *safe_get();
	DO_List *safe_set(D* d);
	DO_List *New();

	inline DO_List *operator->() { return safe_get(); }
};

// the DObject component: the real thing
class DO_List : public DO_Collection {
	RWTValSlist<DRef> thelist;
	
public:
	static DR_Class Listclass;
	static DR_List New();

	DO_List();
	DO_List(DRef r); // casting constructor
	virtual ~DO_List();
	
	// override DO_Object virtual functions
	void init();
	void destroy();
	void assign(const DRef& obj);
	
	// DO_Collection virtuals
	inline DR_void add ( DR_Object o, ref_t t=primary_ref ) { return append(o,t); }
	DO_Enumerator *elements();
	inline size_t size() const { return thelist.entries(); }

	// DO_List methods
	DR_void append ( DR_Object o, ref_t=primary_ref );
	DR_void prepend ( DR_Object o, ref_t=primary_ref );
	DR_void mergeAppend(DR_Collection dc, ref_t=primary_ref);
	DR_void mergePrepend(DR_Collection dc, ref_t=primary_ref);
	DR_Int _size() { return DR_Int((int) size()); }
	inline void clear() { thelist.clear(); }
	
	inline unsigned int dtypeid() { return _D_List_ID; }

	void sort();
	void sort(DRef comparator);

	DRef find(const DR_Composite& what, const DRef& comparator);
	DRef find(const DR_String& aName, const DRef& aValue);

	DRef Class();
};


// There is no DR.
class DO_ListEnumerator : public DO_Enumerator {
	RWTValSlistIterator<DRef> iter;

public:
	DO_ListEnumerator(RWTValSlist<DRef>& l) : iter(l) { }
	virtual ~DO_ListEnumerator();

	// superclass virtuals
	inline BOOLEAN hasMoreElements (  ) { return (BOOLEAN) (iter()); }
	inline DRef nextElement ( ) { return iter.key(); }

	void init();
	void destroy();
	DR_String toString();
};
#endif

