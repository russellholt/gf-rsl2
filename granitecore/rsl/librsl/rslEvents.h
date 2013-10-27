// rslEvents.h
// $Id: rslEvents.h,v 1.2 1998/12/14 19:04:44 toddm Exp $
// *******************
// * System Includes *
// *******************
// ******************
// * Local Includes *
// ******************
#include "b.h"
#include "res_param.h"

#ifndef _RSL_EVENTS_H
#define _RSL_EVENTS_H

// BinaryRequest -- an operator (binary branch node in parse tree)
class BinaryRequest : public Request {
public:
	enum brKind_t { biError=0, biUninitialized, biCompare, biMethod,
		biRefAssign } ;

	brKind_t brKind;
	event *left, *right;

	event *execute(ResContext *context);

	BinaryRequest(void);
	BinaryRequest(event *l, const char *op, event *r, int br=1, int groupExpr=0);
	virtual ~BinaryRequest();

	void print(ostream& out=cout);
};

class BiRefRequest : public BinaryRequest {
public:
	BiRefRequest(event *l, event *r, int groupExpr=0);
	virtual ~BiRefRequest();
	event *execute(ResContext *context);
};

class LocalDecl : public event {
	data_decl *local;
public:
	LocalDecl(data_decl *dd) { local=dd; kind=event::localDeclKind; }
	event *execute(ResContext *context);
	void print(ostream& out=cout) { if (local) local->print(out); }
};


class hijackContext : public event {
	RWCString named_object;
public:

	hijackContext(RWCString name);
	event *execute(ResContext *context);
	void print(ostream& out=cout);
};

class IfRequest : public Request {
	event *expression, *truebranch, *falsebranch;
public:
	IfRequest(event *exp, event *t, event *f=NULL);
	~IfRequest();
	event *execute(ResContext *context);
	void print(ostream& out=cout);
	void printBranch(event *branch, ostream& out, RWCString margin="");
};


class ResEvent : public event {
public:
	Resource *r;
	ResEvent(Resource *res) { r=res; kind=resKind; }
	event *execute(ResContext *context);
	void print(ostream& out=cout) { if (r) r->print(out); else out << "(NULL)"; }
};

// Argument - abstract
// UNNAMED_ARG is the value assumed by argName when it is "unnamed"
// (it's just a symbol that cannot be the name of a formal parameter).
#define UNNAMED_ARG "*"
class Argument : public event {
public:
	RWCString argName;
	Argument(void);
	event *execute(ResContext *context);
};

class elistArg : public elist {
public:
	elistArg(void);
	~elistArg();
	void print(ostream& out=cout);
	event *execute(ResContext *context);
};

// ObjRequestArg
// object lookup in an argument list
class ObjRequestArg : public Argument {
	RWCString object;
public:
	ObjRequestArg(const char* nm);
	void print(ostream& out=cout);
	event *execute(ResContext *context);
	inline RWCString Object(void) { return object; }
};

// ResArg
// Data coming in
class ResArg : public Argument {
public:
	ResReference ref;

	ResArg(Resource *res);
	ResArg(ResReference *aref);
	ResArg(const ResArg &ra);

	void print(ostream& out=cout);
	event *execute(ResContext *context);
};

// ListArg
// An (inline resource) argument that specifies a type
class ListArg : public Argument {

public:
	elist *events;
	RWCString argType;

	ListArg(const char* type, elist *evl=NULL);
	void print(ostream& out=cout);
	event *execute(ResContext *context);
};

// RequestArg
// A request which is an argument
class RequestArg : public Argument  {
public:
	event *req;
	RequestArg(event *r) { req = r; kind |= requestArgKind; }
	void print(ostream& out=cout);
	event *execute(ResContext *context);
};

// controlEvent
// Language control statements such as break and continue which
// do NOT take arguments.
class controlEvent : public event {
	int ctKind;
	event *what;
	
public:
	enum { ctUndefined=0, ctBreak=1, ctContinue=2};
	
	controlEvent(int k, event *ev=NULL);
	~controlEvent();

	void print(ostream& out=cout);
	event *execute(ResContext *context);
};

// controlRequest
// A Request which controls execution, such as "return" or
// an exception, and which takes arguments (in contrast to controlEvent)
class controlRequest : public Request {
	int crFlags;

public:
	enum { crUndefined=0, crReturn=1,
			crOutput=2, crException=4,	// currently unused
			argsResolved=16};

	controlRequest(int ck, elist *args=NULL);
	controlRequest(const controlRequest& cR);
	int CRFlags() const { return crFlags; }
	int hasFlag(int what) const { return (crFlags & what); }
	void addFlag(int what) { crFlags |= what; }

	event *execute(ResContext *context);
	elist *ResolveArgsInPlace(ResContext *context);

	void print(ostream& out=cout);

};

#endif



