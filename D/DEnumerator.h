// DEnumerator.h contains the classes
// DR_Enumerator and DO_Enumerator
//
// $Id: DEnumerator.h,v 1.1 1998/11/12 18:51:43 holtrf Exp $

#include "D.h"

#ifndef _D_Enumerator_
#define _D_Enumerator_

#define _D_Enumerator_ID 1332614169

class DO_Enumerator;

// a DR_Object:DRef class - a strongly-typed smart pointer
class DR_Enumerator : public DR_Object {
public:
	DR_Enumerator (D *d=0);
	DR_Enumerator (const DRef& ref);	
	DR_Enumerator (DR_Collection collection);

	virtual ~DR_Enumerator();
	
	DO_Enumerator *safe_get();
	DO_Enumerator *safe_set(D* d);
	inline DO_Enumerator *operator->() { return safe_get(); }

	virtual DO_Enumerator *New(DR_Collection collection);
	virtual DO_Enumerator *New();

	void Recycle();
};

// a DO_Object class - the guts
class DO_Enumerator : public DO_Object {

	// Instance variables generated from Enumerator.rsl


public:
	// Methods generated from Enumerator.rsl
	virtual BOOLEAN hasMoreElements (  )=0;
	virtual DRef nextElement (  )=0;

	DO_Enumerator();
	DO_Enumerator(DRef r); // casting constructor
	virtual ~DO_Enumerator();

	void init();
	void destroy();

};
	
#endif
