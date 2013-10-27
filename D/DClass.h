// DClass.h defines the classes
// DR_Class and DO_Class
//
// Translation Notes: (delete if desired)
// Loading templates from directory "/dest/home/holtrf/dev/packages/rsl2cpp/templates/D/"
// method Object New( );
//   generating.
// method void Recycle( Object o);
//   generating.
// declaration: String classname;
//   generating.
// declaration: recycler recycler;
//   generating.

//
// $Id: DClass.h,v 1.2 1998/11/20 18:15:47 holtrf Exp $

#include "D.h"

#include "Drecycler.h"


#ifdef D_KEEP_INUSE
#include <rw/tvslist.h>
#include <rw/tpslist.h>
#endif

#ifndef D_CLASS_
#define D_CLASS_

#define _D_Class_ID 812409203

class DO_Class;

class DR_Class : public DR_Object {
  public:
	DR_Class(DO_Class *doc=0);
	DR_Class(const DRef& dref);
	virtual ~DR_Class();

	DO_Class *safe_get();
	DO_Class *const_get() const;
	DO_Class *safe_set(D *d);
	DO_Class *New();
	inline DO_Class *operator->() { return safe_get(); }

	void dump(D* d=0, ref_t t=primary_ref);
	void Recycle();
	
};


/* class DR_recycler; */

class DO_Class : public DO_Object /* DO_Composite */ {
	unsigned int classID;
	RWCString classname;
	DR_recycler recycler;

#ifdef D_KEEP_INUSE
	//RWTPtrSlist<D> p_inUse;
	free_hashset h_inUse;
	//RWTValSlist<t_ref> r_inUse;
#endif

//	DR_Dictionary methods;
//	DR_Dictionary data_members;
//	DR_Dictionary class_variables;

  public:
	DO_Class(const char *name);
	DO_Class(DRef r); // casting constructor
	virtual ~DO_Class();

	DRef route(DR_Message m);
	void init();
	void destroy();
	DR_String toString();

	static DR_Class Classclass;
	DRef Class();

	virtual D* New();
	virtual void Recycle(D* d);
	
	RWCString className();

  private:
	virtual D* spawn();
};

#endif

