// ******************************************************************
// R_D.h
//
// automatically generated from D.rsl and the template:
// $Id: R_D.h,v 1.2 1998/11/24 20:14:47 toddm Exp $
// ******************************************************************

#ifndef _R_D_H_
#define _R_D_H_

/* #include <iostream.h> */
/* #include <rw/cstring.h> */
/* #include "res_class.h" */
#include "Resource.h"
#include "runtime.h"
#include "R_List.h"

#include "D.h"
#include "DCollection.h"
#include "DClassGroup.h"

#define R_D_ID 68

// ********************************************
// * rc_D -- the D RSL class
// ********************************************
class rc_D : public res_class {
	Resource *spawn(RWCString aname);

  public:
	
	static DR_ClassGroup dclasses;
	
	
	rc_D(RWCString aname) : res_class(aname)
	{	}
	
	Resource *New(RWCString nm, ResList *constructor_args=NULL,
		ResContext *constructor_context=NULL);

	void Delete(Resource *rp);
	void AddFree(Resource *r);
};


/*
class R_DIterator : public ResIterator {
  protected:
	DR_Enumerator dre;
  public:
	R_DIterator(RWTValSlist<ResReference>& list);
	int hasMoreElements();
	ResReference nextElement();
};
*/


// *************************************************
// * R_D -- the D Resource
// *************************************************
class R_D : public Resource {
  public:
	static rc_D *Dmaster;

	rc_D *owner;
 	DRef dref;

// Constructors
	
	R_D(RWCString n, rc_D *rc);
	
// Resource virtuals

	// Info

	unsigned int TypeID() { return R_D_ID; }
	res_class *memberOf(void) { return owner ? owner : Dmaster; }
	RWCString StrValue(void);
	int LogicalValue();
	int IsEqual(Resource *r);
	
	// Execution
	void cpp_Init(ResList *constructor_args,
		ResContext *constructor_context);
	ResStatus execute(int method, ResList& arglist);
	void SetFromInline(RWTPtrSlist<Resource>& inliner);
	void Assign(Resource *r);
	void Clear();
	Resource* clone(void);
	
	ResIterator* elements();

	virtual DRef D_Route(const ResList& arglist);
	virtual DRef memberAccess(RWCString methodname, RWCString objname);
	static Resource* DtoR(DRef dr);
	static R_List* refToList(DRef dr);
	static DRef RtoD(ResReference ref);

	inline unsigned int InternalType(void) const { return Resource::DType; }

	// output
	
	void print(ostream &out=cout);	// ECI
	void rslprint(ostream &out=cout);

// R_D specific
	
	static R_D *New(RWCString n, rc_D *own=NULL);
	
// R_D RSL methods
	
	ResStatus rsl_cppClassName(const ResList& arglist);

};

#endif

