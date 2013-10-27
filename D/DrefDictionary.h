/**********************************************************************


 refDictionary is
     DR_refDictionary
     DO_refDictionary
     DO_refDictionaryEnumerator


 $Id: DrefDictionary.h,v 1.1 1998/11/24 22:19:29 toddm Exp $
**********************************************************************/

// // Loading templates from directory "/dest/home/holtrf/dev/packages/rsl2cpp/templates/DCollection/"
// method void add( String key, Object o);
//   generating.


#include "DDictionary.h"

#include <rw/tvhdict.h>

#ifndef _D_refDictionary_
#define _D_refDictionary_

#define _D_refDictionary_ID 1602161941

#define DR_DICT_DEFAULT_SIZE 5

class DO_refDictionary;


/*
class ref2DRef {
public:
	DRef& DR;

	ref2DRef(const DRef& dr) : DR(dr) { }
};
*/

// **********************************************************************
// class DR_refDictionary is a DR_KeyedCollection
// a smart pointer
// **********************************************************************
class DR_refDictionary : public DR_KeyedCollection {
  public:
	DR_refDictionary (D *d=0);
	DR_refDictionary (const DRef& ref);
	virtual ~DR_refDictionary();

	DO_refDictionary *const_get() const;
	DO_refDictionary *safe_get();
	DO_refDictionary *safe_set(D* d);
	DO_refDictionary *New();

	inline DO_refDictionary *operator->() { return safe_get(); }
};

// **********************************************************************
// DO_refDictionary is a DO_KeyedCollection
// the DObject component: the real thing
// **********************************************************************
class DO_refDictionary : public DO_Dictionary {
	
public:
	static DR_Class refDictionaryclass;
	static DR_refDictionary New();

	DO_refDictionary();
	DO_refDictionary(DRef r); // casting constructor
	virtual ~DO_refDictionary();
	
	// override DO_Object virtual functions
	void init();
	void destroy();
	DRef route(DR_Message m);
	
	DRef deepCopy() const;
	
	// DO_Collection virtuals
	DO_Enumerator *elements();
	DR_void add ( DR_Object o, ref_t=primary_ref );

	// DO_KeyedCollection virtuals
	DR_void add (DR_String& key, const DRef& dref, ref_t=primary_ref );
	DRef get (const DR_String& key);
	
	// the functions rw_add, rw_get, cc_add, cc_get
	// are differentiated by name
	// to indicate that they're less desireable than
	// the DR_String versions because they cannot
	// be used by the messaging interface.
	DR_void rw_add (const RWCString key, const DRef& dref, ref_t=primary_ref );
	DRef rw_get (const RWCString key );

	DR_void cc_add ( const char *key, const DRef& dref, ref_t=primary_ref );
	DRef cc_get ( const char *key );

	DR_void remove(DR_String& key);
	DR_void cc_remove(const char *c);
	DR_void rw_remove(const RWCString& key);

	DRef Class();

};

#endif

