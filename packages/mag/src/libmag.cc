#include "DDate.h"
#include "DDecimal.h"
#include "DTime.h"


#include "R_D.h"

class DO_libmag : public DO_Library {
  public:
	DO_libmag();
	virtual ~DO_libmag();

	DRef create ( DR_String& classname );
};

extern "C" res_class *Create_libmag_RC()
{
	DR_Library testlib = new DO_libmag(); // should invoke DR_Library(D *);
	
	rc_D::dclasses->addClass("Date", testlib);
	rc_D::dclasses->addClass("Decimal", testlib);
	rc_D::dclasses->addClass("Time", testlib);		

	return new rc_D("libmag");
}

DO_libmag::DO_libmag()
{
	libname = "libmag";
}

DO_libmag::~DO_libmag() { }

DRef DO_libmag::create(DR_String& classname)
{
	unsigned int hashno = Resource::theIDHash(classname());

	switch(hashno)
	{
		case _D_Date_ID: {
			  DR_Date dt;
			  return dt.New(); // will invoke DRef(D*)
			}
		case _D_Decimal_ID: {
			  DR_Decimal dec;
			  return dec.New(); // will invoke DRef(D*)
			}
		case _D_Time_ID: {
			  DR_Time tim;
			  return tim.New(); // will invoke DRef(D*)
			}						
	
	}

	return DR_null;
}

