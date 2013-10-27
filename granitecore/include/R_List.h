// R_List.h
// $Id: R_List.h,v 1.1 1998/11/17 23:55:09 toddm Exp $
// *******************
// * System Includes *
// *******************
#include <iostream.h>
#include <rw/cstring.h>

// ******************
// * Local Includes *
// ******************
#include "res_class.h"
#include "Resource.h"
#include "destiny.h"

#ifndef _R_LIST_H_
#define _R_LIST_H_

#define R_List_ID 1281979252

// ************************************
// * rc_List -- the List RSL class
// ************************************
class rc_List : public res_class {
	Resource *spawn(RWCString aname);
public:
	rc_List(RWCString aname) : res_class(aname) { }
};

// ************************************
// * R_List -- the List Resource
// * A _value_ list of ResReferences
// ************************************

class R_List : public Resource {
	RWTValSlist<ResReference> locals;

public:
	static rc_List rslType;

	R_List();
	R_List(RWCString n);
	~R_List();

	// Resource virtuals

	unsigned int TypeID() { return R_List_ID; }
	res_class *memberOf(void) { return &rslType; }
	Resource *clone(void);
	RWCString StrValue(void) { return RWCString(""); }
	int LogicalValue() { return locals.entries(); }
	ResStatus execute(int method, ResList& arglist);
	void Clear();
	void print(ostream &out=cout);
	void rslprint(ostream &out=cout);
	void SetFromInline(RWTPtrSlist<Resource>& inliner);
	void Assign(Resource *r);

	// R_List specific

	static R_List *New(RWCString n) { return (R_List *) (rslType.New(n)); }
	void append(Resource *r);

	// GetList(): OBSOLETE. use elements()
	RWTValSlist<ResReference>& GetList() { return locals; }

	ResIterator *elements();

	// R_List RSL methods (see List.rsl)

	ResStatus rsl_append(const ResList& arglist);
	ResStatus insertCopy(const ResList& arglist, bool atHead);
	ResStatus rsl_prepend(const ResList& arglist);
	ResStatus rsl_front(const ResList& arglist);
	ResStatus rsl_removeFront(const ResList& arglist);
	ResStatus rsl_tail(const ResList& arglist);
	ResStatus rsl_removeTail(const ResList& arglist);
	ResStatus rsl_findByValue(const ResList& arglist);
	ResStatus OpBrack(const ResList& arglist);
	ResStatus rsl_deleteByName(const ResList& arglist);
	ResStatus rsl_deleteByIndex(const ResList& arglist);
	ResStatus rsl_deleteByValue(const ResList& arglist);
	ResStatus OpEQ(const ResList& arglist);
	ResStatus rsl_length(const ResList& arglist);
	ResStatus rsl_clear(const ResList& arglist);
    ResStatus rsl_transform(const ResList& arglist);
	ResStatus rsl_distribute(const ResList& arglist);
	ResStatus rsl_distributeByType(const ResList& arglist);
	
	ResStatus rsl_before(const ResList& arglist);
	ResStatus rsl_after(const ResList& arglist);
	ResStatus rsl_sublist(const ResList& arglist);

};

#endif



