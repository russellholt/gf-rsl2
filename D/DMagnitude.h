/************************************************************************

	A magnitude of any kind


Magnitude is
	DR_Magnitude - the "smart pointer" reference to the implementation
	DO_Magnitude - the implementation

  and probably
	DOC_Magnitude - class object (referred to through DR_Class)


$Id: DMagnitude.h,v 1.2 1998/11/24 14:55:56 holtrf Exp $
************************************************************************/

#include "DAtom.h"
#include "DClass.h"

#ifndef _D_Magnitude_
#define _D_Magnitude_

#define _D_Magnitude_ID 1091899914

class DO_Magnitude;

// a DR_Object:DRef class - a strongly-typed smart pointer
class DR_Magnitude : public DR_Object {
public:
	DR_Magnitude (D *d=0);
	DR_Magnitude (const DRef& ref);
	virtual ~DR_Magnitude();

	DO_Magnitude *const_get();
	DO_Magnitude *safe_get();
	DO_Magnitude *safe_set(D* d);
	DO_Magnitude *New();

	inline DO_Magnitude *operator->() { return safe_get(); }

/* 	virtual DR_Magnitude& operator=(const DR_Magnitude& n )=0; */
};

// a DO_Object class - the guts
class DO_Magnitude : public DO_Atom {
	// Instance variables generated from Magnitude.rsl


public:
	DO_Magnitude();
	DO_Magnitude(DRef r); // casting constructor
	virtual ~DO_Magnitude();

/* 	static DR_Class Magnitudeclass; */
/* 	static DR_Magnitude New(); */

	virtual DRef Class();
	virtual DR_Class DClass()=0;

	DRef route(DR_Message m);
	void init();
	void destroy();
	DR_String toString()=0;
	void assign(const DRef& obj) { }

	virtual DR_Magnitude Assign( const DR_Magnitude& n )=0;

/*
	virtual DR_Magnitude Add( const DR_Magnitude& n );
	virtual DR_Magnitude Subt( const DR_Magnitude& n );
	virtual DR_Magnitude Mult( const DR_Magnitude& n );
	virtual DR_Magnitude Div( const DR_Magnitude& n );
	virtual BOOLEAN isEqual( const DR_Magnitude& n );
	virtual BOOLEAN notEqual( const DR_Magnitude& n );
	virtual BOOLEAN less( const DR_Magnitude& n );
	virtual BOOLEAN greater( const DR_Magnitude& n );
	virtual BOOLEAN lessOrEqual( const DR_Magnitude& n );
	virtual BOOLEAN greaterOrEqual( const DR_Magnitude& n );
*/
};
	
#endif

