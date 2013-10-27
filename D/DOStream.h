/************************************************************************


OStream is an abstract class
	DR_OStream - the "smart pointer" reference to the implementation
	DO_OStream - the implementation


$Id: DOStream.h,v 1.1 1998/11/12 18:52:37 holtrf Exp $
************************************************************************/



#include "D.h"
#include "DComposite.h"

#ifndef _D_OStream_
#define _D_OStream_

#define _D_OStream_ID 707926386

class DO_OStream;

// a DR_Object:DRef class - a strongly-typed smart pointer
class DR_OStream : public DR_Object {
public:
	DR_OStream (D *d=0);
	DR_OStream (const DRef& ref);
	virtual ~DR_OStream();

	DO_OStream *const_get() const;
	DO_OStream *safe_get();
	DO_OStream *safe_set(D* d);

	inline DO_OStream *operator->() { return safe_get(); }
};

// a DO_Object class - the guts
class DO_OStream : public DO_Object {

public:
	DO_OStream();
	DO_OStream(DRef r); // casting constructor
	virtual ~DO_OStream();

	DRef route(DR_Message m);
	void init();
	void destroy();
	DR_String toString();

	// Methods generated from OStream.rsl
	virtual DR_String stream ( const DRef& ) =0;
	virtual DR_String composite ( const DR_Composite& ) =0;
	virtual DR_String collection ( const DR_Collection& ) =0;

};
	
#endif

