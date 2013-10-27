/************************************************************************


Decimal is
	DR_Decimal - the "smart pointer" reference to the implementation
	DO_Decimal - the implementation

  and probably
	DOC_Decimal - class object (referred to through DR_Class)


$Id: DDecimal.h,v 1.4 1999/01/06 23:02:50 mbridges Exp $
************************************************************************/

// Translation Notes: (delete if desired)
// // Loading templates from directory "/dest/applied/gf2.5/Releases/gf/gf-2.5a3/packages/templatizer/templates/D"
// method Int round( );
//   generating.




//#include "DComposite.h"

// TEMPORARY NOTE: (as of Sept 29, 1998)
// if the following #include is DObject.h, change it to D.h
#include <DMagnitude.h>
#include <DInt.h>
#include <DBool.h>
#include <rw/decport.h>

#ifndef _D_Decimal_
#define _D_Decimal_

#define _D_Decimal_ID 688131945

class DO_Decimal;

// a DR_Object:DRef class - a strongly-typed smart pointer
class DR_Decimal : public DR_Magnitude {
public:
	DR_Decimal (D *d=0);
	DR_Decimal (const DRef& ref);
	DR_Decimal(RWDecimalPortable value);
	DR_Decimal(double value);
	DR_Decimal(int value);
	DR_Decimal(DR_String value);
	DR_Decimal(DR_Int value);
	DR_Decimal(RWCString value);
	DR_Decimal(char *value);
	virtual ~DR_Decimal();

	DO_Decimal *const_get() const;
	DO_Decimal *safe_get();
	DO_Decimal *safe_set(D* d);
	DO_Decimal *New();

	DR_Decimal operator+(DR_Decimal ref);
	DR_Decimal operator-(DR_Decimal ref);
	DR_Decimal operator*(DR_Decimal ref);
	DR_Decimal operator/(DR_Decimal ref);

	DR_Decimal& operator=(DR_Decimal ref);  
	DR_Decimal& operator+=( DR_Decimal ref);
	DR_Decimal& operator-=(DR_Decimal ref);
	DR_Decimal& operator*=(DR_Decimal ref);
	DR_Decimal& operator/=(DR_Decimal ref);

	int operator<(DR_Decimal ref);
	int operator>(DR_Decimal ref);
	int operator<=(DR_Decimal ref);
	int operator>=(DR_Decimal ref);
	int operator!=(DR_Decimal ref);
	int operator==(DR_Decimal ref);

	DR_String toString();

	inline DO_Decimal *operator->() { return safe_get(); }
	static DR_Decimal DR_Decimal_null;
};

// a DO_Object class - the guts
class DO_Decimal :  public DO_Magnitude{
	friend class DR_Decimal;
	// Instance variables generated from Decimal.rsl


public:
	DO_Decimal();
	DO_Decimal(DRef r); // casting constructor
	virtual ~DO_Decimal();
	DR_Magnitude Assign( const DR_Magnitude& n );
	static DR_Class Decimalclass;
	DR_Class DClass(){return Decimalclass;}
	static DR_Decimal New();
	DRef Class();

	DRef route(DR_Message m);
	void init();
	void destroy();
	DR_String toString();

	RWDecimalPortable& getValue(){return value;}

	// Methods generated from Decimal.rsl
	DR_String asMoneyFormat(DR_String prefix);
	DR_String asPercentageFormat();

	DR_Decimal opAdd(const DRef &dr);
	DR_Decimal opSubt(const DRef &dr);
	DR_Decimal opMult(const DRef &dr);
	DR_Decimal opDiv(const DRef &dr);

	DR_Decimal opAssign(const DRef &dr); // :=
	DR_Decimal opPE(const DRef &dr); // +=
	DR_Decimal opME(const DRef &dr); // -=
	DR_Decimal opTE(const DRef &dr); // *=
	DR_Decimal opDE(const DRef &dr); // /=

	DR_Bool opLT(const DRef &dr);
	DR_Bool opGT(const DRef &dr);
	DR_Bool opLE(const DRef &dr);
	DR_Bool opGE(const DRef &dr);
	DR_Bool opNE(const DRef &dr);
	DR_Bool opEQ2(const DRef& dr); // ==


	dcompare_t compare(const DRef& d) const;
private:
	RWDecimalPortable value;

};
	
#endif

