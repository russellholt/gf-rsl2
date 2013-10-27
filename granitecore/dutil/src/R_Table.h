// R_Table.h
// $Id: R_Table.h,v 1.1 1998/11/17 23:48:26 toddm Exp $

#include <iostream.h>

#include <rw/cstring.h>
#include <rw/tvslist.h>

#include "Resource.h"
#include "res_class.h"
//#include "runtime.h"

#ifndef _R_Table_H_
#define _R_Table_H_

#define R_Table_ID 828465772


// ********************************************
// * rc_Table -- the Table RSL class
// ********************************************
class rc_Table : public res_class {
	Resource *spawn(RWCString aname);
public:
	rc_Table(RWCString aname) : res_class(aname)
	{
	
	}
};

// *************************************************
// * R_Table -- the Table Resource
// *************************************************
class R_Table : public ResStructure {
	RWCString delimiter;

public:
	static rc_Table rslType;

// Constructors
	
	R_Table(RWCString nm);
	~R_Table();
	
// Resource virtuals

	// Info

	unsigned int TypeID() { return R_Table_ID; }
	res_class *memberOf(void) { return &rslType; }
	
	// Execution
	
	ResStatus execute(int method, ResList& arglist);

// ResStructure virtuals
	
	void SetFromInline(RWTPtrSlist<Resource>& inliner);

// R_Table specific
	
	static R_Table *New(RWCString nm);
	void LoadFile(RWCString fname, RWCString rowclass="");
	
// R_Table RSL methods
	
	ResStatus rsl_loadFile(const ResList& arglist);
	ResStatus rsl_add(const ResList& arglist);
	ResStatus rsl_setDelimiter(const ResList& arglist);
	ResStatus rsl_find(const ResList& arglist);
};

#endif

