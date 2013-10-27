/************************************************************************


Date is
	DR_Date - the "smart pointer" reference to the implementation
	DO_Date - the implementation

  and probably
	DOC_Date - class object (referred to through DR_Class)

Implemented using RWDBDateTime, which uses copy on write, that is values can be passed
around without cost.


$Id: DDate.h,v 1.4 1999/01/06 23:00:23 mbridges Exp $
************************************************************************/

// Translation Notes: (delete if desired)
// // Loading templates from directory "/dest/applied/gf2.5/Releases/gf/gf-2.5a3/packages/templatizer/templates/D"
// method Time round( );
//   generating.




//#include "DComposite.h"

// TEMPORARY NOTE: (as of Sept 29, 1998)
// if the following #include is DObject.h, change it to D.h
#include <DMagnitude.h>
#include <DBool.h>
#include <DInt.h>
#include <rw/db/datetime.h>
#ifndef _D_Date_
#define _D_Date_



#define _D_Date_ID 1147237477



// a DO_Object class - the guts
class DO_Date : public DO_Magnitude {
	friend class DR_Date;
 	RWDBDateTime rwdt;

public:
	DO_Date();
	DO_Date(DRef r); // casting constructor
	virtual ~DO_Date();
	DR_Magnitude Assign( const DR_Magnitude& n );
	static DR_Class Dateclass;
	static DR_Date New();
	DRef Class();
	DR_Class DClass(){return Dateclass;}
	DRef route(DR_Message m);
	void init();
	void destroy();
	DR_String toString();
	RWDBDateTime getRWDBDateTime() const {return rwdt;} // class getRWDBDateTime copy on write
	RWDBDateTime getValue() const {return getRWDBDateTime();}
	int daysFromNowCC();
// Used from RSL, probably should not be used in C++ 
	DR_Date opAssign(DRef &dr); // :=, args must be String or Date
	DR_Date opPE(const DR_Int &dr);
	DR_Date opME(const DR_Int &dr); 

	DR_Date opAdd(DR_Int &i);
	DRef opSubt(const DRef &dr);

	DR_Bool opLT(DRef &dr);
	DR_Bool opGT( DRef &dr);
	DR_Bool opLE( DRef &dr);
	DR_Bool opGE( DRef &dr);
	DR_Bool opNE( DRef &dr);
	DR_Bool opEQ2( DRef& dr); // ==

	dcompare_t compare(const DRef& d) const;
	
	};



// a DR_Object:DRef class - a strongly-typed smart pointer
class DR_Date : public DR_Magnitude {
public:

	DR_Date (D *d=0);
	DR_Date (const DRef& ref);
	DR_Date(RWCString);
	DR_Date(char *);
	virtual ~DR_Date();

	DO_Date *const_get() const;
	DO_Date *safe_get();
	DO_Date *safe_set(D* d);
	DO_Date *New();
	inline DO_Date *operator->() { return safe_get(); }

	DR_Date(RWDBDateTime rwdt){safe_get()->rwdt = 
		RWDBDateTime(rwdt.rwdate());}

	DR_Date(DR_String s);
	DR_Date(unsigned day, unsigned month, unsigned year);

	DR_Date& operator=(DR_Date d)
		{
		if (d.DRef::operator==(DR_null))
			{
			DRef::operator=(DR_null);
			return *this;
			}
		safe_get()->rwdt = d->rwdt;
		return *this;
		}
	DR_Date& operator+=(int numDays){(safe_get()->rwdt).addDays(numDays);return *this;}

	DR_Date& operator-=(int numDays){(safe_get()->rwdt).addDays(-numDays);return *this;}

	//returns number of days between dates
	long int operator-(DR_Date &d)
		{return ((safe_get()->rwdt).rwdate()).julian() - ((d->rwdt).rwdate()).julian();}

	int operator<( DR_Date& d){return (safe_get()->rwdt < d->rwdt);}
	int operator<=( DR_Date& d){return (safe_get()->rwdt <= d->rwdt);}
	int operator>( DR_Date& d){return (safe_get()->rwdt > d->rwdt);}
	int operator>=( DR_Date& d){return (safe_get()->rwdt >= d->rwdt);}
	int operator==( DR_Date& d)
		{return 
			(safe_get()->rwdt == d->rwdt);}
	static DR_Date getNow() {return DR_Date(RWDBDateTime());}

	

	
};


	
#endif

