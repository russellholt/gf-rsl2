//	res_class.h
//
// $Id: res_class.h,v 1.2 1998/11/24 20:14:58 toddm Exp $
// *******************
// * System Includes *
// *******************
#include <rw/cstring.h>
#include <rw/tphset.h>
#include <rw/tphdict.h>
#include <rw/tvhdict.h>

// ******************
// * Local Includes *
// ******************
#include "res_param.h"

#ifndef _RES_CLASS_H_
#define _RES_CLASS_H_

class decl;
class method_decl;
class data_decl;
class Resource;
class ResStructure;
class ResObj;
class rslMethod;
class ResList;


#define RC_NONAME "*"

class res_class_decl : public decl {
protected:
	virtual inline void AddMember(decl *de) { if (membersOpen) members.insert(de); }
	virtual inline void AddMethod(method_decl *md)	{ if (membersOpen) methods.insert(md); }
	virtual inline void AddData(data_decl *dd) { if (membersOpen) data.insert(dd); }

public:
	unsigned membersOpen;
	RWTPtrSlist<decl> members;
	int totalDataDecl;

// description is now inherited from decl (along with flags)
// 	RWCString description;

	res_class_decl(void) { membersOpen = 1; totalDataDecl=0; }

	// independent methods/data lists -- may go away...
	RWTPtrSlist<method_decl> methods;
	RWTPtrSlist<data_decl> data;

	inline void CloseDeclarations(void) { membersOpen = 0; }
	virtual void AddDecl(decl *dc);
	virtual void ClassifyDecl(const char *nm);

	virtual void PrintMembers(ostream& out=cout, int printScope=0);
	virtual void htmlMembers(ostream& out=cout, int printScope=0);

	virtual void PrintData(ostream& out=cout);
	virtual void htmlData(ostream& out=cout);

	virtual void PrintMethods(ostream& out=cout);
	virtual void htmlMethods(ostream& out=cout);

	virtual int Resolve(int method, ResList& arglist, rslMethod *& rslm);
	virtual int LinkImplementation(rslMethod *rm);

	// inherited from decl
	inline void print(ostream& out=cout, int printScope=0) { PrintMembers(out,printScope); }
	inline void html(ostream& out=cout, int printScope=0) { htmlMembers(out,printScope); }
};

// ***************************************
// * res_class -- an RSL class (abstract)
// ***************************************
class res_class
{
protected:
	unsigned int typeID;
	RWCString name, derivedFrom;

	res_class_decl *inner;

	void Init(void);
	virtual Resource *spawn(RWCString nm);
	void rslConstructor(ResStructure *rsp, ResList *constructor_args, ResContext *constructor_context);

	RWTPtrSlist<Resource> freeList;

public:
	enum {noResolveScope=0, ResolveScope=1,
		methodNotFound, rslMethodFound, checkForCppMethod,
		memberNotFound, dataMemberFound };

	enum impl_t { imp_unknown, imp_cpp, imp_rsl, imp_D };

	RWCString shares;

	res_class(void);
	res_class(RWCString nm);

	// meta information get and set
	
	void SetName(RWCString nm);
	void SetParent(RWCString pa);
	inline RWCString Name(void) const { return name; }
	inline RWCString DerivedFrom(void) const { return derivedFrom; }
	res_class *DerivedFromClass(void);
	res_class *SharedClass(void);
	inline unsigned int TypeID(void) const { return typeID; }
	int HierarchyContains(RWCString& classname);
	int HierarchyContains(unsigned int type_id);
	virtual void print(ostream& out=cout);
	virtual void html(ostream& out=cout);
	int DataDeclEntries(void);
	inline int isNull(void) { return (inner == NULL); }
	inline int hasCPPImplementation() { return (implementation == imp_cpp); }
	int hasParentCPPImplementation();

	Resource *localVarNames();

	// data setting (used by RSL parser and system ONLY)
	
	static res_class *LinkToResClass(res_class_decl *rcd, const char *nm);
	inline int LinkImplementation(rslMethod *rm)
		{ return inner? (inner->LinkImplementation(rm)) : 0; }
	void SetInner(res_class_decl *in, impl_t imp=imp_unknown);

	// Memory management
	
	virtual Resource *New(RWCString nm, ResList *constructor_args=NULL,
		ResContext *constructor_context=NULL);

	virtual void Delete(Resource *rp);
	virtual void AddFree(Resource *r);
	inline size_t FreeListEntries() { return freeList.entries(); }

	// Data extraction
	
	virtual int Resolve(int method, ResList& arglist, rslMethod *& rslm, Resource *& resToUse);
	virtual int ResolveDataMember(RWCString& member, ResStructure *rs, Resource *& returned);
	virtual int InstallDeclarations(ResContext *context);
	virtual method_decl *GetMethod(void);
	inline res_class_decl *getImplementation() { return inner; }

	// rsl method execution	
	event *executeRSLMethod(RWCString methodName, ResList& arglist, ResObj& inContext);

	// utility functions for RogueWave hashing

	inline int operator==(const res_class& rc) { return ( name == rc.Name()); }
	static unsigned hash(const res_class& rc);

	friend class runtimeRSL;
	
private:
	impl_t implementation;
	res_class *parent_class, *shared_class;	// cached pointers.
	int data_decl_entries;	//cached value: total from this class + super hierarchy
	int has_base_cpp_imp;	//cached value: whether this or any parent has cpp impl.
};


extern RWTPtrHashSet<res_class> ResClasses;

#endif



