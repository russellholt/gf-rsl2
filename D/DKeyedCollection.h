// main D C++ template
// Translation Notes:
// Loading templates from directory "/dest/home/holtrf/dev/packages/rsl2cpp/templates/D/"
// method void add( String key, Object o);//   no RSL implementation, but not declared `native'.
//   generating.
// method Object get( String key);//   no RSL implementation, but not declared `native'.
//   generating.

//
// $Id: DKeyedCollection.h,v 1.2 1998/11/20 18:15:20 holtrf Exp $

/*******************************************

 *******************************************/

#include "DCollection.h"

#ifndef _D_KeyedCollection_
#define _D_KeyedCollection_

#define _D_KeyedCollection_ID 707533693

class DO_KeyedCollection;

// the DRef component: smart pointer
class DR_KeyedCollection : public DR_Collection {
public:
	DR_KeyedCollection (D *);
	DR_KeyedCollection (DRef ref);
	virtual ~DR_KeyedCollection();

	DO_KeyedCollection *safe_get();
	DO_KeyedCollection *const_get() const;
	DO_KeyedCollection *safe_set(D* d);

	inline DO_KeyedCollection *operator->() { return safe_get(); }
};

// the DObject component: the real thing
class DO_KeyedCollection : public DO_Collection {
public:

	DO_KeyedCollection();
	virtual ~DO_KeyedCollection();
	
	DRef route(DR_Message m);

	// Methods generated from KeyedCollection.rsl
	virtual DR_void add (DR_String& key, const DRef& dref, ref_t=primary_ref )=0;
	virtual DRef get (const DR_String& key)=0;
	DR_void add ( DR_Object o, ref_t=primary_ref);	// not supported.

	virtual DR_void remove(DR_String& key)=0;
	virtual DR_void cc_remove(const char *c)=0;
	virtual DR_void rw_remove(const RWCString& key)=0;

	// the functions rw_add, rw_get, cc_add, cc_get
	// are differentiated by name
	// to indicate that they're less desireable than
	// the DR_String versions because they cannot
	// be used through the messaging interface.
	virtual DR_void rw_add (const  RWCString key, const DRef& dref, ref_t=primary_ref )=0;
	virtual DRef rw_get (const  RWCString key )=0;

	virtual DR_void cc_add ( const char *key, const DRef& dref, ref_t=primary_ref )=0;
	virtual DRef cc_get ( const char *key )=0;
};

class DO_KeyedEnumerator : public DO_Enumerator {
public:
	virtual DR_String nextKey (  )=0;

	DO_KeyedEnumerator() { }
	virtual ~DO_KeyedEnumerator() { }
};

class DR_KeyedEnumerator : public DR_Enumerator {
public:
    DR_KeyedEnumerator (D *d=0) : DR_Enumerator(d) { }
    DR_KeyedEnumerator (const DRef& ref) : DR_Enumerator(ref) { }
	DR_KeyedEnumerator (DR_KeyedCollection collection);
	virtual ~DR_KeyedEnumerator() { }
	
	DO_KeyedEnumerator *safe_get();
	DO_KeyedEnumerator *safe_set(D* d);
	inline DO_KeyedEnumerator *operator->() { return safe_get(); }

	DO_Enumerator *New(DR_Collection collection);
	DO_Enumerator *New(void);
};

#endif
