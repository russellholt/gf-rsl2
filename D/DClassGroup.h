// DClassGroup.h contains the classes
// DR_ClassGroup and DO_ClassGroup
//
// Translation Notes: (delete if desired)
// Loading templates from directory "/dest/home/holtrf/dev/packages/rsl2cpp/templates/D/"
// method void add( String classname, Library lib);
//   generating.
// method Object create( String classname);
//   generating.
// declaration: Dictionary classlib;
//   generating.

//
// $Id: DClassGroup.h,v 1.2 1998/11/20 18:15:55 holtrf Exp $

/*******************************************
 group of class libraries 
 *******************************************/

#include "DDictionary.h"
#include "DLibrary.h"

#ifndef _D_ClassGroup_
#define _D_ClassGroup_

#define _D_ClassGroup_ID 1163596572

class DO_ClassGroup;

// a DR_Object:DRef class - a strongly-typed smart pointer
class DR_ClassGroup : public DR_Object {
public:
	DR_ClassGroup (D *d=0);
	DR_ClassGroup (const DRef& ref);
	virtual ~DR_ClassGroup();

	DO_ClassGroup *safe_get();
	DO_ClassGroup *safe_set(D* d);
	DO_ClassGroup *New();

	inline DO_ClassGroup *operator->() { return safe_get(); }
};

// a DO_Object class - the guts
class DO_ClassGroup : public DO_Object {

public:

	/**  two kinds of class name mappings */
	DR_Dictionary stringToLibrary ;
	DR_Dictionary stringToClass ;


	DO_ClassGroup();
	DO_ClassGroup(DRef r); // casting constructor
	virtual ~DO_ClassGroup();

	DRef route(DR_Message m);
	void init();
	void destroy();
	DR_String toString();

	// Methods generated from ClassGroup.rsl
	DR_void addClass ( DR_String classname, const DR_Library& lib );
	DR_void addClass ( const DR_Class& classobj );
	DRef create ( const DR_String& classname );

protected:
	virtual DRef createFromClass(DR_Class theClass);
	virtual DRef createFromLib(DR_Library theLib, DR_String classname);
};
	
#endif
