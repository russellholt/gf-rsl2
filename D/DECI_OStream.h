/************************************************************************


ECI_OStream is
	DR_ECI_OStream - the "smart pointer" reference to the implementation
	DO_ECI_OStream - the implementation


$Id: DECI_OStream.h,v 1.2 1998/11/19 19:32:46 holtrf Exp $
************************************************************************/


#include "DOStream.h"

#ifndef _D_ECI_OStream_
#define _D_ECI_OStream_

#define _D_ECI_OStream_ID 1869697069

class DO_ECI_OStream;

// a DR_Object:DRef class - a strongly-typed smart pointer
class DR_ECI_OStream : public DR_OStream {
public:
	DR_ECI_OStream (D *d=0);
	DR_ECI_OStream (const DRef& ref);
	virtual ~DR_ECI_OStream();

	DO_ECI_OStream *const_get() const;
	DO_ECI_OStream *safe_get();
	DO_ECI_OStream *safe_set(D* d);
	DO_ECI_OStream *New();

	inline DO_ECI_OStream *operator->() { return safe_get(); }
};

// a DO_Object class - the guts
class DO_ECI_OStream : public DO_OStream {
public:
	DO_ECI_OStream();
	DO_ECI_OStream(DRef r); // casting constructor
	virtual ~DO_ECI_OStream();

	static DR_Class ECI_OStreamclass;
	static DR_ECI_OStream New();
	DRef Class();

	DRef route(DR_Message m);
	void init();
	void destroy();
	DR_String toString();

	// Methods generated from ECI_OStream.rsl
	DR_String stream ( const DRef& );
	DR_String composite ( const DR_Composite& );
	DR_String collection ( const DR_Collection& );


};
	
#endif


