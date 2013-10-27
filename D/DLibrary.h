// DLibrary.h contains the classes
// DR_Library and DO_Library
//
// Translation Notes: (delete if desired)
// Loading templates from directory "/dest/home/holtrf/dev/packages/rsl2cpp/templates/D/"
// method Object create( String classname);
//   generating.

//
// $Id: DLibrary.h,v 1.2 1998/11/20 18:16:03 holtrf Exp $

/*******************************************

 *******************************************/

//#include "DComposite.h"
#include "D.h"

#ifndef _D_Library_
#define _D_Library_

#define _D_Library_ID 756751218

class DO_Library;

// a DR_Object:DRef class - a strongly-typed smart pointer
class DR_Library : public DR_Object {
public:
	DR_Library (D *d=0);
	DR_Library (const DRef& ref);
	virtual ~DR_Library();

	DO_Library *const_get() const;
	DO_Library *safe_get();
	DO_Library *safe_set(D* d);
	DO_Library *New();

	inline DO_Library *operator->() { return safe_get(); }
};

// a DO_Object class - the guts
class DO_Library : public DO_Object {
public:
	DR_String libname;

	DO_Library();
	DO_Library(DRef r); // casting constructor
	virtual ~DO_Library();

	DRef route(DR_Message m);
	void init();
	void destroy();
	DR_String toString();

	// Methods generated from Library.rsl
	virtual DRef create ( DR_String& classname );

};
	
#endif
