/**********************************************************************


 Dictionary is
     DR_Dictionary
     DO_Dictionary
     DO_DictionaryEnumerator


 $Id: DDictionary.h,v 1.3 1998/11/20 18:15:13 holtrf Exp $
**********************************************************************/

// // Loading templates from directory "/dest/home/holtrf/dev/packages/rsl2cpp/templates/DCollection/"
// method void add( String key, Object o);
//   generating.


#include "DKeyedCollection.h"

#include <rw/tvhdict.h>

#ifndef _D_Dictionary_
#define _D_Dictionary_

#define _D_Dictionary_ID 1602161941

#define DR_DICT_DEFAULT_SIZE 5

class DO_Dictionary;

// **********************************************************************
// class DR_Dictionary is a DR_KeyedCollection
// a smart pointer
// **********************************************************************
class DR_Dictionary : public DR_KeyedCollection {
  public:
	DR_Dictionary (D *d=0);
	DR_Dictionary (const DRef& ref);
	virtual ~DR_Dictionary();

	DO_Dictionary *const_get() const;
	DO_Dictionary *safe_get();
	DO_Dictionary *safe_set(D* d);
	DO_Dictionary *New();

	inline DO_Dictionary *operator->() { return safe_get(); }
};

// **********************************************************************
// DO_Dictionary is a DO_KeyedCollection
// the DObject component: the real thing
// **********************************************************************
class DO_Dictionary : public DO_KeyedCollection {
protected:
	RWTValHashDictionary<RWCString, DRef> table;
	
public:
	static DR_Class Dictionaryclass;
	static DR_Dictionary New();

	DO_Dictionary();
	DO_Dictionary(DRef r); // casting constructor
	virtual ~DO_Dictionary();
	
	// override DO_Object virtual functions
	void init();
	void destroy();
	DRef route(DR_Message m);
	
	DRef deepCopy() const;
	
	// DO_Collection virtuals
	DO_Enumerator *elements();
	DR_void add ( DR_Object o, ref_t=primary_ref );
	inline size_t size() const { return table.entries(); }

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


// **********************************************************************
// Collection Enumerator
// * If you derive from an existing collection with well-defined, this
// class may not be necessary.
//
// * All DO_Enumerator subclasses are used through the
// DR_Enumerator interface class; no DR_DictionaryEnumerator
// class should be necessary.
// **********************************************************************
class DO_DictionaryEnumerator : public DO_KeyedEnumerator {
	// enumerator data goes here
	RWTValHashDictionaryIterator<RWCString, DRef> iter;

public:
	DO_DictionaryEnumerator(RWTValHashDictionary<RWCString, DRef>& t) : iter(t) { }
	virtual ~DO_DictionaryEnumerator();

	// superclass virtuals
	inline BOOLEAN hasMoreElements() { return (BOOLEAN) (iter()); }
	inline DRef nextElement() { return iter.value(); }
	inline DR_String nextKey() { return DR_String(iter.key()); }

	DR_String toString();
};
#endif
