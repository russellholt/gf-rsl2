// DCollection.h contains the classes
// DR_Collection and DO_Collection
//
// Translation Notes: (delete if desired)
// Loading templates from directory "/dest/home/holtrf/dev/packages/rsl2cpp/templates/D/"
// method void add( Object o);
//   generating.
// method void elements( );
//   generating.

//
// $Id: DCollection.h,v 1.1 1998/11/12 18:51:04 holtrf Exp $

/*******************************************

 *******************************************/

#include "D.h"
#include "DInt.h"

#ifndef _D_Collection_
#define _D_Collection_

#define _D_Collection_ID 1231165445

class DO_Collection;

// a DR_Object:DRef class - a strongly-typed smart pointer
class DR_Collection : public DR_Object {
public:
	DR_Collection (D *);
	DR_Collection (DRef ref);
	
	virtual ~DR_Collection();

	DO_Collection *const_get() const;
	DO_Collection *safe_get();
	DO_Collection *safe_set(D* d);

	inline DO_Collection *operator->() { return safe_get(); }
};

class DO_Enumerator;

// a DO_Object class - the guts
class DO_Collection : public DO_Object {
public:
	virtual DR_void add ( DR_Object o , ref_t=primary_ref)=0;
	virtual DO_Enumerator *elements (  )=0;

	DO_Collection();
	DO_Collection(DRef r); // casting constructor
	virtual ~DO_Collection();

	virtual size_t size() const =0;
	virtual DR_Int _size();
	virtual DRef deepCopy() const;
	dcompare_t compare(const DRef& d) const;

	void init();
	void destroy();
	DR_String toString();
	BOOLEAN toBoolean() const;
	DRef route(DR_Message m);
};

#include "DEnumerator.h"

#endif
