// R_Status.h
// $Id: R_Status.h,v 1.1 1998/11/17 23:55:23 toddm Exp $
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
#include "StatMessages.h"

#ifndef _R_Status_H_
#define _R_Status_H_

#define R_Status_ID 638017908

// ************************************
// * rc_Status -- the Status RSL class
// ************************************
class rc_Status : public res_class {
	Resource *spawn(RWCString aname);
public:
	rc_Status(RWCString aname) : res_class(aname)
	{
//		ResClasses.insert((res_class *) this);
	}
};


// ************************************
// * R_Status -- the Status Resource
// ************************************
class R_Status : public Resource {
	int number, severity;
	RWCString tag;

public:
	static rc_Status rslType;
	static StatusMessageMap stat_messages;

	R_Status(int v = statusMessage::statusUnset);
	R_Status(RWCString n, int v = statusMessage::statusUnset);

	// Resource virtuals

	res_class *memberOf(void) { return &rslType; }
	Resource *clone(void);
	RWCString StrValue(void);
	int LogicalValue() { return (number != 0); }
	ResStatus execute(int method, ResList& arglist);
	void Clear() { number = severity = statusMessage::statusUnset; tag = STATUS_UNSET_TAG; }
	void print(ostream &out=cout);
	void rslprint(ostream &out=cout);
	void SetFromInline(RWTPtrSlist<Resource>& inliner);
	void Assign(Resource *r);
	int IsEqual(Resource *r);
	unsigned int TypeID() { return R_Status_ID; }

	// R_Status specific

	static R_Status *New(RWCString n, int v);
	void Set(int num, int s = statusMessage::statusUnset, RWCString t=STATUS_UNSET_TAG);
	void Set(void);

	int Number(void) const { return number; }
	int Severity(void) const { return severity; }
	RWCString Tag(void) const { return tag; }

	int intval(void) const { return Number(); }
	int matchField(RWCString left, RWCString right) const;
	int matchField(int left, int right) const;
	int operator==(const R_Status& stat) const { return (stat.Number() == number); }
	void operator=(const int i) { number = i; }

	// R_Status RSL methods
	
	ResStatus rsl_LoadFile(const ResList& arglist);
	ResStatus OpEQ(const ResList& arglist);
	ResStatus rsl_SetNumber(const ResList& arglist);
	ResStatus rsl_Number(const ResList& arglist);
	ResStatus rsl_Severity(const ResList& arglist);
    ResStatus rsl_Tag(const ResList& arglist);
	ResStatus rsl_Message(const ResList& arglist);
	ResStatus OpEQ2(const ResList& arglist);

};

#endif



