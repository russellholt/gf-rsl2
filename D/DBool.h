/************************************************************************


Bool is
	DR_Bool - the "smart pointer" reference to the implementation
	DO_Bool - the implementation

  and probably
	DOC_Bool - class object (referred to through DR_Class)


$Id: DBool.h,v 1.2 1998/11/24 14:55:10 holtrf Exp $
************************************************************************/


#include "DMagnitude.h"

#ifndef _D_Bool_
#define _D_Bool_

#define _D_Bool_ID 1114599276

class DR_Bool;


// a DO_Object class - the guts
class DO_Bool : public DO_Magnitude {
	int val;
	
public:
	DO_Bool();
	DO_Bool(DRef r); // casting constructor
	virtual ~DO_Bool();

	static DR_Class Boolclass;
	static DR_Bool New();
	DR_Class DClass();

	DRef route(DR_Message m);
	void init();
	void destroy();
	DR_String toString();
	virtual BOOLEAN toBoolean() const;
	dcompare_t compare(const DRef& d) const;
	void assign(const DRef& obj);

	inline int& b() { return val; }

	DR_Magnitude Assign( const DR_Magnitude& n );

	// Methods generated from Bool.rsl
	DR_Bool Add ( const DR_Bool& n );

	static DR_String true_text, false_text;
};

// a DR_Object:DRef class - a strongly-typed smart pointer
class DR_Bool : public DR_Magnitude {
public:
	DR_Bool (D *d=0);
	DR_Bool (const DRef& ref);
	DR_Bool (int truthVal){safe_get()->b() = truthVal;}
	virtual ~DR_Bool();

	DO_Bool *const_get() const;
	DO_Bool *safe_get();
	DO_Bool *safe_set(D* d);
	DO_Bool *New();

	inline DO_Bool *operator->() { return safe_get(); }
	
	inline int& b() { return safe_get()->b(); }
	inline int& operator()() { return safe_get()->b(); }
	
	inline DR_Bool& operator=(const int n) { b() = n; return *this; }
	inline DR_Bool& operator=(const DR_Bool& n) { safe_get()->Assign(n); return *this; }
/* 	DR_Bool& operator=(const DR_Magnitude& n); */
	
};



#endif
