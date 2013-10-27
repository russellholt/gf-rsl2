// $Id: D.h,v 1.2 1998/11/24 14:54:58 holtrf Exp $
#ifndef _D_DESTINY_
#define _D_DESTINY_

#include "Dcommon.h"

class DRef;
class DR_Message;
class DR_String;


// ***********************************************************

#define _D_ID 68


enum ref_t { secondary_ref=0, primary_ref=1 };

class RefCount {
	short primary, secondary;

public:

	RefCount() { init(); }

	inline int Primary() const {	return primary; }
	inline int Secondary() const {	return secondary; }

	inline int newReference() { return newPrimary(); }
	inline int decReference() { return decPrimary(); }

	int newReference(ref_t);
	int decReference(ref_t);

	inline int newPrimary() { return ++primary; }
	inline int decPrimary() { return --primary; }
	inline int newSecondary() { return ++secondary; }
	inline int decSecondary() { return --secondary; }

	inline int operator++() { return newPrimary(); }	// prefix
	inline int operator++(int) { return newPrimary(); } // postfix

	inline int operator--() { return decPrimary(); }
	inline int operator--(int) { return decPrimary(); }

	inline void init() { primary=0, secondary=0; }
};

enum dcompare_t { c_less=-1, c_equal=0, c_greater=1 };


class D {
	// int ref_count;
	void zeroref();

public:
	RefCount ref_count;

	D() { }
	virtual ~D();

	// basic ref count access. use the ref_count object
	// directly for access to the Secondary reference.
	virtual inline int decRef() { return --ref_count; }
	virtual inline int newRef() { return ++ref_count; }
	inline int refCount() const { return ref_count.Primary(); }
	

	virtual DRef doesNotUnderstand(DR_Message m);
	virtual DRef route(DR_Message m)=0;

	virtual void _init();
	virtual void _destroy();

	virtual void init()=0;	// "constructor"
	virtual void destroy()=0;	// "destructor"
	
	virtual DR_String toString()=0;
	inline virtual BOOLEAN toBoolean() const { return DR_FALSE; }
	virtual DRef deepCopy() const;
	virtual void assign(const DRef& obj);
	virtual dcompare_t compare(const DRef& d) const;

	virtual DR_String cpp_classname();
	virtual inline unsigned int dtypeid() { return _D_ID; }
	
	virtual DRef Class();
	static DRef d_class;

	int operator==(const D&);	// pointer comparison
};

// ***********************************************************
#include "D_macros.h"

// ***********************************************************
#include "DRef.h"

// ***********************************************************
#include "DO_Object.h"

// ***********************************************************
#include "DR_Object.h"

// ***********************************************************
#include "DString.h"

// ***********************************************************
#include "DMessage.h"

// ***********************************************************

unsigned int theIDHash(const char *);


// Global comparison operators!
int operator< (const DRef& left, const DRef& right);
int operator<= (const DRef& left, const DRef& right);
int operator> (const DRef& left, const DRef& right);
int operator>= (const DRef& left, const DRef& right);


#endif _D_DESTINY_
