/************************************************************************


Int is
	DR_Int - the "smart pointer" reference to the implementation
	DO_Int - the implementation

  and probably
	DOC_Int - class object (referred to through DR_Class)


$Id: DInt.h,v 1.2 1998/11/24 14:55:41 holtrf Exp $
************************************************************************/


#include "DMagnitude.h"

#ifndef _D_Int_
#define _D_Int_

#define _D_Int_ID 4812404

class DR_Int;


// a DO_Object class - the guts
class DO_Int : public DO_Magnitude {
	int val;
	
public:
	DO_Int();
	DO_Int(DRef r); // casting constructor
	virtual ~DO_Int();

	static DR_Class Intclass;
	static DR_Int New();
	DR_Class DClass();

	DRef route(DR_Message m);
	void init();
	void destroy();
	DR_String toString();
	BOOLEAN toBoolean() const;
	dcompare_t compare(const DRef& d) const;
	void assign(const DRef& obj);
	
	inline int& i() { return val; }
	int i_val() const { return val; }

	DR_Magnitude Assign( const DR_Magnitude& n );

	// Methods generated from Int.rsl
	DR_Int Add ( const DR_Int& n );

};

// a DR_Object:DRef class - a strongly-typed smart pointer
class DR_Int : public DR_Magnitude {
public:
	DR_Int (D *d=0);
	DR_Int (const DRef& ref);
	DR_Int (int n);
	virtual ~DR_Int();

	DO_Int *const_get() const;
	DO_Int *safe_get();
	DO_Int *safe_set(D* d);
	DO_Int *New();

	inline DO_Int *operator->() { return safe_get(); }
	
	inline int& i() { return safe_get()->i(); }
	inline int& operator()() { return safe_get()->i(); }

	int i_val() const;
	
	inline DR_Int& operator=(const int n) { i() = n; return *this; }
	inline DR_Int& operator=(const DR_Int& n) { safe_get()->Assign(n); return *this; }
/* 	DR_Int& operator=(const DR_Magnitude& n); */
	
	inline DR_Int& operator+=(const int n) { i() += n; return *this; }
	DR_Int& operator+=(const DR_Int& n);
/* 	DR_Int& operator+=(const DR_Magnitude& n); */

	inline DR_Int& operator-=(const int n) { i() -= n; return *this; }
	DR_Int& operator-=(const DR_Int& n);
/* 	DR_Int& operator-=(const DR_Magnitude& n); */
	
};

int operator==(const DR_Int& left, int right);
int operator==(const DR_Int& left, const DR_Int& right);
int operator!=(const DR_Int& left, int right);
int operator!=(const DR_Int& left, const DR_Int& right);


#endif
