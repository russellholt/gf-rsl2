/**********************************************************************
 a composite object 

 Composite is
     DR_Composite
     DO_Composite
     DO_CompositeEnumerator


 $Id: DComposite.h,v 1.3 1998/12/04 22:24:42 holtrf Exp $
**********************************************************************/

#include "DKeyedCollection.h"
#include "DClass.h"

#ifndef _D_Composite_
#define _D_Composite_

#define _D_Composite_ID 1226572804

class DO_Composite;

// **********************************************************************
// class DR_Composite is a DR_Collection
// a smart pointer
// **********************************************************************
class DR_Composite : public DR_Collection {
  public:
	DR_Composite (D *d=0);
	DR_Composite (const DRef& ref);
	virtual ~DR_Composite();

	DO_Composite *const_get() const;
	DO_Composite *safe_get();
	DO_Composite *safe_set(D* d);

	DO_Composite *New();
	DO_Composite *New(DR_KeyedCollection storage);

	inline DO_Composite *operator->() { return safe_get(); }
};

// **********************************************************************
// DO_Composite is a DO_Collection
// the DObject component: the real thing
// **********************************************************************
class DO_Composite : public DO_Collection {
protected:
	DR_KeyedCollection storage ;


public:
	DO_Composite();
	DO_Composite(DRef r); // casting constructor
	virtual ~DO_Composite();
	
	// override DO_Object virtual functions
	void init();
	void destroy();
	DRef route(DR_Message m);
	DR_String toString();
	DRef deepCopy() const;
	void assign(const DRef& obj);
	dcompare_t compare(const DRef& d) const;

	static DRef Compositeclass; // points to a DO_Message
	static DR_Composite New();
	inline DRef Class() { return DClass(); }
	virtual DR_Class DClass() { return Compositeclass; }
	
	// DO_Collection virtuals

	DR_void add(DR_Object o, ref_t=primary_ref);
	DO_Enumerator *elements();
	size_t size() const;

	// DO_Composite methods

	void setStorage(DR_KeyedCollection drk, ref_t=primary_ref);
	
	DR_void add( const DR_String& name, const DRef& obj, ref_t=primary_ref );
	DRef get ( const DR_String& name );

	DR_void add( const char *name, const DRef& obj, ref_t=primary_ref );
	DRef get ( const char *name );

	DR_void remove ( DR_String name );
};

// ***********************************************
// Member access macros for convenience and safety
// ***********************************************
#define DC_replacePtr(dref_what, ptr_with)\
	dref_what.replace(ptr_with);\
	DO_Composite::remove( #dref_what );\
	DO_Composite::add( #dref_what , dref_what)

#define DC_replaceRef(dref_what, dref_with)\
	dref_what.replace(dref_with.unsafe_get());\
	DO_Composite::remove( #dref_what );\
	DO_Composite::add( #dref_what , dref_what)

#define DC_dump(dr)\
	dr.dump();\
	DO_Composite::remove( (dr, #dr ) )

#define DC_get( var ) DO_Composite::get( (var, #var ) )

#define DC_remove( var ) DO_Composite::remove( (var, #var ) )

#define DC_sync( var ) var.replace( get( #var ) )

#define DC_add(x) add( #x, x)


#endif


