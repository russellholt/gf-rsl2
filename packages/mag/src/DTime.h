/************************************************************************


Time is
	DR_Time - the "smart pointer" reference to the implementation
	DO_Time - the implementation

  and probably
	DOC_Time - class object (referred to through DR_Class)

Implemented using RWDBDateTime, which uses copy on write, that is values can be passed
around without cost.

These classes internally store the time as  a date/time.  Thus if you compare a DO_TIME
created today at 11:59PM with one created tomorrow at 12:01AM, todays DO_TIME will be less.

Currently only times whose date is today can be created.

$Id: DTime.h,v 1.2 1998/11/30 23:44:06 mbridges Exp $
************************************************************************/

// Translation Notes: (delete if desired)
// // Loading templates from directory "/dest/applied/gf2.5/Releases/gf/gf-2.5a3/packages/templatizer/templates/D"
// method Time dummy( );
//   generating.




//#include "DComposite.h"

// TEMPORARY NOTE: (as of Sept 29, 1998)
// if the following #include is DObject.h, change it to D.h
#include "DMagnitude.h"
#include "DBool.h"
#include <rw/db/datetime.h>

#ifndef _D_Time_
#define _D_Time_

#define _D_Time_ID 1416195429

class DR_Time;
// a DO_Object class - the guts
class DO_Time : public DO_Magnitude {
	// Instance variables generated from Time.rsl
friend class DR_Time;
RWDBDateTime rwdt;

public:
	DO_Time();
	DO_Time(DRef r); // casting constructor
	virtual ~DO_Time();

	static DR_Class Timeclass;
	static DR_Time New();
	DR_Class DClass(){return Timeclass;}
	DR_Magnitude Assign( const DR_Magnitude& n );


	DRef Class();

	DRef route(DR_Message m);
	void init();
	void destroy();
	DR_String toString();


	DR_Time opAssign(const DRef &dr); // :=, args must be String or Date


	DR_Bool opLT(const DR_Time &dr);
	DR_Bool opGT(const DR_Time &dr);
	DR_Bool opLE(const DR_Time &dr);
	DR_Bool opGE(const DR_Time &dr);
	DR_Bool opNE(const DR_Time &dr);
	DR_Bool opEQ2(const DR_Time& dr); // ==
	dcompare_t compare(const DRef& d) const;

};


// a DR_Object:DRef class - a strongly-typed smart pointer
class DR_Time : public DR_Magnitude {
public:
	DR_Time (D *d=0);
	DR_Time (const DRef& ref);
	virtual ~DR_Time();

	DO_Time *const_get() const;
	DO_Time *safe_get();
	DO_Time *safe_set(D* d);
	DO_Time *New();

	inline DO_Time *operator->() { return safe_get(); }
	DR_Magnitude Assign( const DR_Magnitude& n );
	DR_Time(DR_String str);
	DR_Time(RWDBDateTime rwdt);

	DR_Time& operator=(DR_Time d)
		{
		if (d.DRef::operator==(DR_null))
			{
			DRef::operator=(DR_null);
			return *this;
			}
		safe_get()->rwdt = d->rwdt;
		return *this;
		}

	int operator<( DR_Time& d){return (safe_get()->rwdt < d->rwdt);}
	int operator<=( DR_Time& d){return (safe_get()->rwdt <= d->rwdt);}
	int operator>( DR_Time& d){return (safe_get()->rwdt > d->rwdt);}
	int operator>=( DR_Time& d){return (safe_get()->rwdt >= d->rwdt);}
	int operator==( DR_Time& d){return (safe_get()->rwdt == d->rwdt);}

	

};


	
#endif
