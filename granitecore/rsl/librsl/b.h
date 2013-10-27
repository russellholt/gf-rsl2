/*
 * b.h --  RSL2 parser data structures
 *
 * $Id: b.h,v 1.2 1998/12/14 15:28:02 holtrf Exp $
 */
// *******************
// * System Includes *
// *******************
#include <iostream.h>
#include <rw/cstring.h>
#include <rw/tpslist.h>
 
// ******************
// * Local Includes *
// ******************
#include "Resource.h"
#include "destiny.h"

#ifndef _B_H_
#define _B_H_

class event {
protected:
	event(void);

public:
	enum evtKind_t { noKind=0, objRequestKind=1, requestKind=2, elistKind=4,
			biRequestKind=8, localDeclKind=16, resKind=32, refKind=64,
			argKind=128, resArgKind=256, argListKind=512, objReqArgKind=1024,
			elistArgKind=2048, eventGroupKind=4096, ifRequestKind=8192,
			requestArgKind=16384, controlKind=32768, controlReqKind=65536,
			dataMemberReqKind=131072, selfReqKind=262144, programCodeKind=524288,
			dynamicRequestKind=1048576
			};
	int kind;

	int isA(evtKind_t what) { return (kind & (int)what); }
	virtual ~event();
	virtual event *execute(ResContext *context) { return NULL; }

	virtual void print(ostream& out=cout)=0;

	int operator==(const event& e) { return this == (&e); }
};

class eventGroup : public event {
	event *ev;
public:
	eventGroup(event *e);
	virtual ~eventGroup() { }
	event *execute(ResContext *context)
	{ if (ev) return ev->execute(context); return NULL; }
	void print(ostream& out=cout);
};

// elist
// list of events
class elist : public event {
	bool use_own_context;
public:
	RWTPtrSlist<event> evtl;

	elist(void);
	virtual ~elist();

	virtual void add(event *e);
	virtual void transferContentsFrom(elist *evts);
	event *execute(ResContext *context);
	void CheckReturnEvent(event *e, event *retev,
		elist *& returnList, ResContext *context);

	inline size_t entries(void)  { return evtl.entries(); }
	inline bool useOwnContext() { return use_own_context; }
	void print(ostream& out=cout);
};


class objRequest : public event {
public:
	RWCString object;
	objRequest(const char* what);
	virtual ~objRequest() { }
	ResStatus Resolve(ResContext *context);
	void print(ostream& out=cout) { }
};

class Request : public objRequest {
public:
	RWCString method;
	elist *arguments;

	Request(const char* obj, const char* meth, elist *args=NULL);
	virtual ~Request();

	void addSimpleArgument(RWCString argName, RWCString argValue);

	virtual event *execute(ResContext *context) { return RExecute(context); }
	virtual event *RExecute(ResContext *context, int reportErrors=1);
	event *executeResource(Resource *r, ResList& rl, ResContext *context, int reportErrors=1);
	event *executeCpp(Resource *r, ResList& rl, int reportErrors=1);
	void ResolveArguments(ResContext *context, ResList& rl);
	event *resolveDataMember(ResStructure *rs);
	void SetArgs(elist *elp) { arguments=elp; }

	void print(ostream& out=cout);
};

// AuditRequest -- a request with auditing information, giving
// identifiers to the object and the method, for
// associating with a caller in a distributed system.
#define AR_CreateOnly_Method "Init"
#define AR_QuitOnly_Method "Kill"

class AuditRequest : public Request {
public:
	RWCString object_id, method_id;

	AuditRequest(char* o, char* oid, char* m, char* mid);
	virtual ~AuditRequest();
	event *execute(ResContext *context);
	int createInitialNew(ResContext *);

	void print(ostream& out=cout);
};


#endif



