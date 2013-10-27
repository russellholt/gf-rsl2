// Resource.h
// Core RSL declarations for Resources, data storage, and memory management.
//
//	ResStatus		RSL system return codes with a riding Resource object
//	Resource		object in the RSL environment
//	ResReference	memory management interface to a Resource
//	ResContext		named storage for Resources and other contexts
//	ResStructure	C++ implemented Resource with RSL-accessible data members
//	ResObj			total RSL-implemented Resource
//	ResList			fixed, safe array of ResReferences.
//
// $Id: Resource.h,v 1.2 1999/01/12 15:07:22 toddm Exp $

// *******************
// * System Includes *
// *******************
#include <rw/cstring.h>
#include <rw/tpordvec.h>
#include <rw/tvslist.h>
#include <rw/tvhset.h>
#include <iostream.h>

// ******************
// * Local Includes *
// ******************
#include "res_class.h"
#include "evtstat.h"

#ifndef _Resource2_H_
#define _Resource2_H_

class res_class;
class ResList;
class Resource;

class ResStatus {
public:

	// RSL internal status values
	enum { rslFalse=0, rslTrue, rslFail, rslOk, rslUninitialized,
		rslBreak, rslContinue, rslReturn, rslExit };

	int status;
	Resource *r;

	ResStatus(void);
	ResStatus(int st, Resource *res=NULL);
};

class ResReference;


class ResIterator {
  public:
	virtual int hasMoreElements() { return 0; }
	virtual ResReference nextElement();
};

class Resource {
	int refcount;

protected:
	RWCString name;

public:
	// Memory management (see also res_class.h)
	
	virtual int NewReference();	// these
	virtual int DecReference(); // need to be overridden by ResReference
	inline int RefCount() const { return refcount; }

	Resource(void);
	Resource(RWCString nm);
	virtual ~Resource();
	
	// info

	virtual inline RWCString Name() const { return name; }
	virtual unsigned int TypeID();
	virtual RWCString ClassName(void);
	int HierarchyContains(unsigned int typeID);
	int HierarchyContains(RWCString classname);
	
	// OBSOLETE data access
	
	inline RWCString Value() { return StrValue(); }	// use StrValue() !
	Resource *GetSet(RWCString& themember, Resource *f);
	Resource *GetSet(int& themember, Resource *f);

	// Functions that should be overridden

	virtual res_class* memberOf(void);
	virtual Resource* clone(void);
	virtual RWCString StrValue();
	virtual int LogicalValue();
	virtual ResStatus execute(int method, ResList& arglist);

	virtual void Clear();
	virtual void print(ostream &out=cout);
	virtual void rslprint(ostream &out=cout);
	virtual void SetFromInline(RWTPtrSlist<Resource>& inliner);
	virtual void Assign(Resource *r);
	virtual int IsEqual(Resource *r);

	virtual void cpp_Init(ResList *constructor_args,
		ResContext *constructor_context);

	virtual void AddOrReplace(ResReference ref);
	
	/** Preferred iteration mechanism. By default, does nothing,
		meaning "I don't support iteration." */
	virtual ResIterator *elements();

	// functions for RW template hash collections

	static unsigned int theIDHash(const char *s0);
	static unsigned int hash(const Resource& r);
	int operator==(const Resource& r);

	// RSL internals
	
	enum  { regularType=0, resRefType=1, resStructType=2, resObjType=4,
		resLangType=8, DType=16 };
	
	virtual inline unsigned int InternalType(void) const { return regularType; }
	int isRSLStruct(void) const;
	int isLang() const;
	int isD() const;
	virtual inline void SetName(RWCString nm) { name = nm; }

	// Friends - access for memory management

	friend class res_class;
	friend class ResReference;
};


typedef enum access_t { vPublic, vPrivate, vProtected };

// class ResReference
//  Safe pointer to a Resource, used for memory management,
//	as well as to give another name to a Resource.
//	Controls the refcount of the associated Resource on
//	construction/destruction, Set, etc.
class ResReference : public Resource {
	Resource *that;
	access_t visibility;
	static int count; // internal debugging

public:
	ResReference(void);
	ResReference(RWCString nm, access_t vis = vPublic);
	ResReference(Resource *thatone, access_t vis = vPublic);
	ResReference(RWCString nm, Resource *thatone, access_t vis = vPublic);
	ResReference(const ResReference& rr);
	~ResReference();

// Resource virtuals

	RWCString Name(void) const;
	RWCString ClassName(void);
	unsigned int TypeID(void);
	res_class* memberOf(void);

	RWCString StrValue(void);
	int LogicalValue();
	Resource* clone(void);

	ResStatus execute(int method, ResList& arglist);

	int NewReference();
	int DecReference();

	int IsEqual(Resource *r);
	void Clear();
	void print(ostream &out=cout);
	void rslprint(ostream &out=cout);
	
	void AddOrReplace(ResReference ref);

	ResIterator *elements();

		
// ResReference specific

	// update
	ResReference& operator=(const ResReference& rr);
	void Set(RWCString nm, Resource *thatone);
	
	// info
	int operator==(const ResReference& rr);
	int isType(unsigned t);
	inline int isValid(void) const { return (that != NULL); }
	inline unsigned InternalType(void) const { return Resource::resRefType; }
	
	// access
	inline Resource *RealObject(void) const { return that; }
	inline Resource *operator()(void) const { return that; }
	inline Resource *operator->(void) const { return that; }

	// access control
	inline access_t Visibility(void) const { return visibility; }
	inline int isPublic(void) const { return visibility == vPublic; }
	inline int isProtected(void) const { return visibility == vProtected; }
	inline int isPrivate(void) const { return visibility == vPrivate; }

	static unsigned int hash(const ResReference& r);
};


// ResContext
// A "namespace"; a collection of resources and other contexts,
// eg. a "user session" or local variable table in an executing method
class ResContext {
	RWCString contextName;
	RWTValHashSet<ResReference> *locals;
	RWTPtrSlist<ResContext> *contexts;
	Resource *contextOwner;

	void Replace(ResReference& what, Resource *with);

public:
	ResContext(const char *nm, size_t nlocals);
	~ResContext();
	
	// Context info
	
	RWCString Name() const { return contextName; }
	size_t LocalResources() const { return (locals? (locals->entries()) : (size_t) 0); }
	
	// Data insertion and extraction
	
	void AddResource(Resource *r, access_t vis = vPublic);
	void AddReferenceTo(RWCString newname, Resource *r, access_t vis = vPublic);
	void AddResListContents(ResList& rl);
	void AddContext(ResContext *c);
	void PrependContext(ResContext *c);
	void RemoveContext(const char *nm);
	void RemoveContext(ResContext *c);
	void RemoveResource(RWCString nm);
	void ResizeLocalSpace(int n);


	Resource *Owner(void);
	inline void SetOwner(Resource *r) { contextOwner = r; }
	inline void SetLocalContextName(RWCString nm) { contextName = nm; }
	ResStatus Find(RWCString what, Resource *replaceWith=NULL);
	
	/** GetLocals() is OBSOLETE */
	RWTValHashSet<ResReference> *GetLocals(void);
	
	/** Preferred iteration mechanism. Must delete the object returned. */
	ResIterator *elements();

	void Clear(void);

	// Data output

	void print(ostream& out=cout);
	void printContextInfo(ostream& out=cout, RWCString indent="");

	// hash functions
	
	static unsigned hash(const ResContext& rc);
	int operator==(const ResContext& rc);
};


// ResStructure -- abstract
// Resource with explicit storage made available to RSL.
// All C++ implemented resources which have RSL-accessible
// resource data members (as in "a.b") derive from ResStructure.
// Those Resource which allow a mix of C++ and RSL implemented
// methods must derive from ResObj, however.
class ResStructure : public Resource {
protected:
	ResContext locals;
	ResStructure(const char *nm, const char *contextname,
		size_t localsize=RWDEFAULT_CAPACITY);

	enum EoA_t { equality, assignment };
	int EqualOrAssign(EoA_t eqOrAssign, Resource *r);

public:
	~ResStructure();

	// Resource virtuals
	
	virtual void SetFromInline(RWTPtrSlist<Resource>& inliner);
	virtual inline void Assign(Resource *r) { EqualOrAssign(assignment, r); }
	virtual inline int IsEqual(Resource *r) { return EqualOrAssign(equality, r); }
	virtual void SetName(RWCString nm);
	virtual void Clear(void);
	unsigned InternalType(void) const { return Resource::resStructType; }
	
	virtual void AddOrReplace(ResReference ref);

	void print(ostream &out=cout);
	void rslprint(ostream &out=cout);

	ResStatus execute(int method, ResList& arglist);
	
	// ResStructure-specific	

	EventStatus RSLexecute(RWCString method, ResList& arglist,
		ResContext *context);

	inline ResContext& GetLocalContext() { return locals; }

	void RemoveResource(RWCString nm);
	ResReference GetDataMember(RWCString theName);
	
	/** Preferred iteration mechanism. Must delete the object returned. */
	virtual	ResIterator *elements();
};


// ResObj -- an RSL implemented resource OR a C++ resource which
// allows RSL-implemented methods as well
// the default res_class::spawn creates one of these.
class ResObj : public ResStructure {
	res_class *owner;

public:
	ResObj(const char *nm, res_class *rc);
	~ResObj();
	
	// Inherited from resource
	
	inline res_class* memberOf(void) { return owner; }
	Resource* clone(void);
	inline unsigned InternalType(void) const { return Resource::resObjType; }
	void rslprint(ostream &out=cout);
	ResStatus execute(int method, ResList& arglist);


	// These methods will eventually call an rslMethod
	// allowing them to be implemented in RSL for each RSL-defined class.
	
	inline RWCString StrValue(void) { return name; }
	inline int LogicalValue() { return 1; }
	
	// Inherited from ResStructure
	
	EventStatus RSLexecute(rslMethod* method, ResList& arglist, ResContext* context);
};


class Request;

// ResList
// A fixed-size array of Resource references (ResReference).
// Operates most like a bounded stack (ordered insertion), but
// random element access.
// Used mainly in argument passing to methods.
#define RL_NULL_RESOURCE "-*-"
class ResList {
	ResReference *items;
	int size,			// number of allocated elements
		nextinsert;		// where the next Add() will put an element

public:
	Request *theRequest;	// actual request
	ResContext *enclosingContext;	// where the request was made

	ResList(int size=10, Request *req=NULL, ResContext *ctxt=NULL);
	~ResList();

	// data insertion

	void Add(Resource *r);
	void Add(Resource *r, RWCString pseudoname);

	// data extraction - real object

	Resource *get(const char *nm) const;
	inline Resource *get(int index) const
	{	return validIndex(index) ? (items[index].RealObject()) : (Resource *) NULL;	}

	// data extraction - reference

	Resource *getref(const char *nm) const;
	inline Resource *getref(int index) const
	{	return validIndex(index) ? (items+index) : (ResReference *) NULL;	}
	inline ResReference operator[](int index) const
	{	return validIndex(index) ? items[index] : ResReference(""); }
		
	// info
	
	inline int validIndex(int index) const { return (index > -1 && index < size); }
	inline int isNull(int index) const { return (get(index) == NULL); }
	inline int entries(void) const { return size; }
	inline int isEmpty(void) const { return (nextinsert == 0); }
	void print(ostream& out=cout) const;
};

#endif



