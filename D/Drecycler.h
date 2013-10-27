// Drecycler.h contains the classes
// DR_recycler and DO_recycler
//
// $Id: Drecycler.h,v 1.1 1998/11/12 18:53:58 holtrf Exp $

/*******************************************

	class Drecycler

	manages object recycling. may create and destroy objects instead
	depending on circumstances. There should be an instance of this
	class for every class in the system. Some classes, such as
	the enumerators, are not recycled but destroyed due to
	implememntation issues.

 *******************************************/

#include "D.h"

#include "freed.h"

#ifndef _D_recycler_
#define _D_recycler_

#define _D_recycler_ID 285804043

class DO_recycler;

// a DR_Object:DRef class - a strongly-typed smart pointer
class DR_recycler : public DR_Object {
public:
	DR_recycler (D *d=0);
	DR_recycler (const DRef& ref);
	virtual ~DR_recycler();

	DO_recycler *safe_get();
	DO_recycler *safe_set(D* d);
	DO_recycler *New();

	void Recycle();

	inline DO_recycler *operator->() { return safe_get(); }
};

// a DO_Object class - the guts
class DO_recycler : public DO_Object {
	freeManager *freeList;

	/**  class name that we are attached to  */
	RWCString classname ;

public:
	DO_recycler();
	DO_recycler(DRef r); // casting constructor
	virtual ~DO_recycler();

	DRef route(DR_Message m);
	void init();
	void destroy();
	DR_String toString();

	D* get (  );
	void recycle (D* o );

	void setClassName(RWCString clname);
};

	
#endif
