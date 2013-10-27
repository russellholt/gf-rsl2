/************************************************************************


XML_OStream is
	DR_XML_OStream - the "smart pointer" reference to the implementation
	DO_XML_OStream - the implementation

  and probably
	DOC_XML_OStream - class object (referred to through DR_Class)


$Id: DXML_OStream.h,v 1.1 1998/11/12 18:53:23 holtrf Exp $
************************************************************************/

// Translation Notes: (delete if desired)
// // Loading templates from directory "/dest/applied/gf2.5/Releases/gf/gf-2.5a3/packages/templatizer/templates/D"
// method String stream( Object o);
//   generating.
// method String composite( Composite c);
//   generating.




//#include "DComposite.h"

// TEMPORARY NOTE: (as of Sept 29, 1998)
// if the following #include is DObject.h, change it to D.h
#include "DOStream.h"

#ifndef _D_XML_OStream_
#define _D_XML_OStream_

#define _D_XML_OStream_ID 1920947501

class DO_XML_OStream;

// a DR_Object:DRef class - a strongly-typed smart pointer
class DR_XML_OStream : public DR_OStream {
public:
	DR_XML_OStream (D *d=0);
	DR_XML_OStream (const DRef& ref);
	virtual ~DR_XML_OStream();

	DO_XML_OStream *const_get() const;
	DO_XML_OStream *safe_get();
	DO_XML_OStream *safe_set(D* d);
	DO_XML_OStream *New();

	inline DO_XML_OStream *operator->() { return safe_get(); }
};

// a DO_Object class - the guts
class DO_XML_OStream : public DO_OStream {

public:
	DO_XML_OStream();
	DO_XML_OStream(DRef r); // casting constructor
	virtual ~DO_XML_OStream();

	static DR_Class XML_OStreamclass;
	static DR_XML_OStream New();
	DRef Class();

	DRef route(DR_Message m);
	void init();
	void destroy();
	DR_String toString();

	// Methods generated from XML_OStream.rsl
	DR_String stream ( const DRef& );
	DR_String composite ( const DR_Composite& );
	virtual DR_String collection ( const DR_Collection& );

};
	
#endif

