// R_String.h
// $Id: R_String.h,v 1.2 1998/11/24 19:27:56 toddm Exp $
#ifndef _R_String2_H_
#define _R_String2_H_

#include <iostream.h>
#include <rw/cstring.h>

#include "res_class.h"
#include "Resource.h"
#include "runtime.h"

#define R_String_ID 1024684649

#include "DString.h"

// ************************************
// * rc_String -- the String RSL class
// ************************************
class rc_String : public res_class {
	Resource *spawn(RWCString aname);
public:
	rc_String(RWCString aname) : res_class(aname) { }
};


// ************************************
// * R_String -- the String Resource
// ************************************
class R_String : public Resource, public DR_String {
public:
	static rc_String rslType;

	R_String(RWCString v);
	R_String(char* v);
	R_String(RWCString n, RWCString v);
	
	// Resource virtuals
	
	inline unsigned int TypeID() { return R_String_ID; }
	res_class *memberOf(void) { return &rslType; }
	Resource *clone(void);
	RWCString StrValue(void);
	int LogicalValue();
	ResStatus execute(int method, ResList& arglist);
	void Clear();
	void print(ostream &out=cout);
	void rslprint(ostream &out=cout);
	void SetFromInline(RWTPtrSlist<Resource>& inliner);
	void Assign(Resource *r);
	int IsEqual(Resource *r);

	// R_String specific
	
	static R_String *New(RWCString val);
	static R_String *New(RWCString nm, RWCString val);
// 	static R_String *New(RWCString nm, DR_String val);

	void stripControl(int stripNewlines);
	void Set(RWCString v);
// 	void operator=(RWCString v) { drs = v; }
// 	void operator=(DR_String v) { drs = v; }
	void ConvertEscapes();
	int matches(const RWCRegexp& reg);
	RWCString OutEscape();
	
	// R_String RSL methods
	
	ResStatus OpEQ(const ResList& arglist);
	ResStatus OpAdd(const ResList& arglist);
	ResStatus OpPE(const ResList& arglist);
	ResStatus rsl_prepend(const ResList& arglist);
	ResStatus rsl_append(const ResList& arglist);
	ResStatus OpBrack(const ResList& arglist);
	ResStatus OpEQ2(const ResList& arglist);
	ResStatus OpNE(const ResList& arglist);
	ResStatus OpGT(const ResList& arglist);
	ResStatus OpGE(const ResList& arglist);
	ResStatus OpLT(const ResList& arglist);
	ResStatus OpLE(const ResList& arglist);
	ResStatus rsl_contains(const ResList& arglist);
	ResStatus rsl_containsRegex(const ResList& arglist);
	ResStatus rsl_matchesRegex(const ResList& arglist);
	ResStatus rsl_isNumeric(const ResList& arglist);
	ResStatus rsl_isCash(const ResList& arglist);
	ResStatus rsl_length(const ResList& arglist);
	ResStatus rsl_squeeze(const ResList& arglist);
	ResStatus rsl_replace(const ResList& arglist);
	ResStatus rsl_replaceRegex(const ResList& arglist);
    ResStatus rsl_prePad(const ResList& arglist);
    ResStatus rsl_postPad(const ResList& arglist);
    ResStatus OpMult(const ResList& arglist);
    ResStatus rsl_repeat(const ResList& arglist);
	ResStatus rsl_stripControl(const ResList& arglist);
	ResStatus rsl_split(const ResList& arglist);
	ResStatus rsl_upcase(const ResList& arglist);
	ResStatus rsl_downcase(const ResList& arglist);
	ResStatus rsl_before(const ResList& arglist);
	ResStatus rsl_beforeRegex(const ResList& arglist);
	ResStatus rsl_after(const ResList& arglist);
	ResStatus rsl_afterRegex(const ResList& arglist);

};

#endif

