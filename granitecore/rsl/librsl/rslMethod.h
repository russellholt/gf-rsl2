// rslmethod.h
// $Id: rslMethod.h,v 1.1 1998/11/17 23:56:03 toddm Exp $

// *******************
// * System Includes *
// *******************
// ******************
// * Local Includes *
// ******************
#include "res_param.h"
#include "b.h"
#include "rslEvents.h"
#include "Resource.h"

#ifndef _RSL_METHOD_H_
#define _RSL_METHOD_H_

class rslMethod {
	method_decl *prototype;
	event *body;	// actually -- an elist

public:
	RWCString description;

	rslMethod(void);
	rslMethod(method_decl *md, event *bod, char *doc=NULL);
	~rslMethod();
	
	static int traceMode;

	event *execute(ResContext *context);
	virtual void print(ostream& out = cout);
	virtual void html(ostream& out=cout);

	method_decl*& proto(void) { return prototype; }

	int operator==(const rslMethod& m);
//	int operator<(const rslMethod& m);
};

#endif



