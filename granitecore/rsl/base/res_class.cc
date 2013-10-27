// res_class.cc
//  an RSL class
// $Id: res_class.cc,v 1.3 1998/12/01 20:10:26 holtrf Exp $

#include <fstream.h>
#include <rw/tpslist.h>
#include <rw/tphasht.h>

#include "res_class.h"
#include "runtime.h"
#include "Resource.h"
#include "rslMethod.h"
#include "rsldefaults.h"
#include "slog.h"
#include "destiny.h"	// logf subsystem defines
#include "rsl_html.h"

#include "R_String.h"

#include "R_D.h"

static char rcsid[] = "$Id: res_class.cc,v 1.3 1998/12/01 20:10:26 holtrf Exp $";

extern ofstream rslerr;	// rslmain.cc
extern ofstream *statsFile, *freelistFile;	// runtime.cc
extern void freelistStats(ostream &out);	// runtime.cc

// some statistics declared in runtime.cc
extern int nFreelistAdds, nFreelistRemoves, actualResCreations;
extern bool useFreelist;

#define DEFAULT_SUPERCLASS "Object"
#define DCOMPOSITE "Composite"

// hash - just calls the static hash function of Resource.
// Uses the class name only because it needs to be unique, by name.
unsigned res_class::hash(const res_class& rc)
{
	return (rc.Name()).hash();
}

void res_class::Init(void)
{
	inner = NULL;
	implementation = imp_unknown;	// c++ implementation by default
	parent_class = shared_class = NULL;
	data_decl_entries = -1;
	has_base_cpp_imp = -1;
}

res_class::res_class(RWCString nm)
{
	Init();
	shares = RC_NONAME;	// uninitialized value
	derivedFrom = DEFAULT_SUPERCLASS;	// rfh, 3/31/1998
	SetName(nm);
}

res_class::res_class(void)
{
	Init();
	name = RC_NONAME;	// uninitialized value
	shares = RC_NONAME;	// uninitialized value
	derivedFrom = DEFAULT_SUPERCLASS;	// rfh, 3/31/1998
	typeID = 0;
}

void res_class::SetName(RWCString nm)
{
	name = nm;
	typeID = Resource::theIDHash(nm);

	// the class for the default superclass shouldn't
	// inherit from the default superclass (itself)!
	// rfh, 3/31/1998 (yow! I'm coding!)
	if (nm == DEFAULT_SUPERCLASS)
		derivedFrom = RC_NONAME;	// uninitialized value
}

// spawn
// the default resource creator -- creates ResObj Resources,
// those which are RSL implemented Resources.
Resource *res_class::spawn(RWCString nm)
{
#ifdef RSLERR
	rslerr << "res_class::spawn() : create a new ResObj for `" << nm << "'\n";
#endif
	
	// at this point we know that the implementation is rsl, so set it.
	implementation = imp_rsl;

	return new ResObj(nm, this);
}


res_class *res_class::DerivedFromClass(void)
{
	if (!parent_class && derivedFrom != RC_NONAME)
	{
		res_class findrc(derivedFrom);
		parent_class = ResClasses.find(&findrc);

		if (!parent_class)
		{
			logf->info(LOGAPPENV) << "class `" << name << "' extends a class `"
				<< derivedFrom
				<< "' which cannot be found." << endline;

			if (derivedFrom != "Object")
			{
				derivedFrom = "Object";
				logf->info(LOGAPPENV)
					<< "\tattempting to extend `Object' to try to continue."
					<< endline;

				return DerivedFromClass();
			}
		}
	}

	return parent_class;
}

res_class *res_class::SharedClass(void)
{
	if (!shared_class && shares != RC_NONAME)
	{
		res_class findrc(shares);
		shared_class = ResClasses.find(&findrc);
	}

	return shared_class;
}

int res_class::hasParentCPPImplementation()
{
	if (has_base_cpp_imp < 0)
	{
		if (implementation == imp_cpp)
			has_base_cpp_imp = 1;
		else
			has_base_cpp_imp = DerivedFromClass() ? parent_class->hasParentCPPImplementation() : 0;
	}
	return has_base_cpp_imp;
}

void res_class::SetParent(RWCString pa)
{
	if (derivedFrom == RC_NONAME || derivedFrom == DEFAULT_SUPERCLASS || derivedFrom == DCOMPOSITE)
		derivedFrom = pa;
}

Resource *res_class::localVarNames()
{
	R_List *rl = R_List::New("");

	if (inner && inner->totalDataDecl > 0)
	{
		RWTPtrSlistIterator<data_decl> iter(inner->data);
		data_decl *dd=NULL;
		while(iter())
		{
			dd = iter.key();
			if (dd)
			{
				RWTValSlistIterator<RWCString> nameiter(dd->varnames);
				while (nameiter())
					rl->append(R_String::New(nameiter.key(), nameiter.key()));
			}
		}
	}

	return rl;
}

// DataDeclEntries()
// Find the total number of data declarations in an instance
// of this class - that includes this class and all parent
// classes combined. Since we have to do multiple lookups,
// cache this value for future reference.
int res_class::DataDeclEntries(void)
{
	// if cached value is not set..
	if (data_decl_entries < 0)
	{
		// number of declarations for this class
		data_decl_entries = inner? inner->totalDataDecl : 0;
		
		// add to this the total number of declarations in
		// our super-hierarchy
		if (DerivedFromClass())
			data_decl_entries += parent_class->DataDeclEntries();
	}

	return data_decl_entries;
}

// res_class::HierarchyContains(classname)
// Find out whether the inheritance hierarchy
// contains the named class. Recursively calls parent classes.
int res_class::HierarchyContains(RWCString& classname)
{
	if (name == classname)
		return 1;
	
	res_class *dfc = DerivedFromClass();
	if (dfc == NULL)	//  no parent class to check
		return 0;

	// recurse through parent classes.
	return dfc->HierarchyContains(classname);
}

// res_class::HierarchyContains(classID)
// Same as above, but using integer comparison on hash values instead of
// string comparison (better).
int res_class::HierarchyContains(unsigned int type_id)
{
	if (type_id != 0 && typeID == type_id)
		return 1;

	res_class *dfc = DerivedFromClass();
	if (dfc == NULL || dfc == this)	//  no parent class to check
		return 0;
		
	// recurse through parent classes.
	return dfc->HierarchyContains(type_id);
}

int res_class::InstallDeclarations(ResContext *context)
{

	if (!context)
	{
#ifdef RSL_DEBUG_CREATE
		rslerr << "install declarations for class `" << name << "': null context\n";
#endif
		return 1;
	}

	if (inner && !inner->data.isEmpty())
	{
#ifdef RSL_DEBUG_CREATE
		rslerr << "Instantiating data members of class `" << name
			<< "' in context `" << (context->Name()) << "'...\n";
#endif

		// Step through the list of data declarations, and have
		// each declaration installed into our object context.
		RWTPtrSlistIterator<data_decl> iter(inner->data);
		data_decl *dd;
		while(iter())
		{
			dd = iter.key();
			if (dd)
				dd->install(context);
		}
	}
		
	// Inherit datamembers from superclass
	if (derivedFrom != RC_NONAME)
	{
#ifdef RSLERR
			rslerr << "\tChecking superclass `" << derivedFrom << "'.\n";
#endif

		res_class *rc = DerivedFromClass();
		if (rc && rc != this)	// recurse with superclass
		{
			rc->InstallDeclarations(context);
			
			// see if the parent class has a c++ implementation.
			// if so, and if *we* don't have cpp implementation,
			// then create a sub-object for it.
			if (!hasCPPImplementation() && rc->hasCPPImplementation())
			{
#ifdef RSL_DEBUG_CREATE
				rslerr << "RSL class `" << name << "' extends the C++ Resource `"
					<< derivedFrom << "': Creating private subobject.\n";
#endif
				Resource *r = rc->spawn("");
				context->AddReferenceTo(CPP_SUBOBJECT_NAME, r, vPrivate);
			}
		}
#ifdef RSLERR
			else
				rslerr << "Unable to find class `" << derivedFrom << "'\n";
#endif
	}
	return 0;
}



void res_class_decl::AddDecl(decl *dc)
{
	if (!dc)
		return;

	// 1. Add to the mixed-list of declarations
	AddMember(dc);

	// 2. Add to the individual lists as well
	if (dc->hasFlag(decl::data))
	{
		totalDataDecl += ((data_decl *) dc)->entries();
		AddData((data_decl *) dc);
	}
	else
	if (dc->hasFlag(decl::method))
		AddMethod((method_decl *) dc);
#ifdef RSLERR
	else
		rslerr << "res_class::AddDecl(): undefined declaration kind\n" << flush;
#endif
}


// ClassifyDecl
// Attach the class name to the methods.
// Typically called from within LinkToResClass() when
// the res_class_decl is built by the parser and added
// to a res_class.
void res_class_decl::ClassifyDecl(const char *nm)
{
RWTPtrSlistIterator<method_decl> iter(methods);
method_decl *md=NULL;
	while(++iter == TRUE)
	{
		md = iter.key();
		if (md)
			md->memberOf = nm;
	}
}

void res_class::print(ostream& out)
{
	if (inner && inner->description.length() > 0)
		out << "/**" << inner->description << "*/\n";

	out << "class " << name;
	if (derivedFrom != RC_NONAME)
		out << " extends " << derivedFrom;
	out << " {\n";

	if (inner)
		inner->PrintMembers(out, noResolveScope);

	out << "}\n";
}

// print as html. Later, could merge with a template instead
// of hardcoding the formatting here.
void res_class::html(ostream& out)
{
	// class name and inheritance

	out << "<html><head><title>RSL: " << name << "</title></head><body>\n";
	out << "<a href=class_index.html>Resource Class Index</a><hr>\n";

	out << "\n<P><H2>" << name << "</H2>\n";
	if (derivedFrom != RC_NONAME )
	{
		out << "<UL><P><b>extends ";
		linkClassName(derivedFrom, out);
		out << "</b></P></UL>\n";
	}


	out << "<UL><P>";

	if (inner && inner->description.length() > 0)
		out << inner->description << endl;
	else
		out << "<i>no class description</i>\n";

	out << "</P></UL>\n";

	// print class declarations
	if (inner)
	{
		out << "<center><table border=0 width=80%>\n";

		//inner->htmlMembers(out, noResolveScope);

		out << "<tr><td colspan=2><H3>Data Members</H3></td></tr>\n";
		inner->htmlData(out);

		out << "<tr><td colspan=2><H3>Methods</H3></td></tr>\n";
		inner->htmlMethods(out);

		out << "</table></center>\n";
	}
	else
		out << "<i>no class members</i>\n";
}

// ***************************************
// * Print the list of methods
// * --> May go away! But seems useful..
// * (later: group by visibility)
// ***************************************
void res_class_decl::PrintMethods(ostream& out)
{
	RWTPtrSlistIterator<method_decl> methIter(methods);
	method_decl *mp = NULL;

	while(++methIter == TRUE)
	{
		mp = methIter.key();
		if (mp)
			mp->print(out);
		out << "\n\t";
	}
}

void res_class_decl::htmlMethods(ostream& out)
{
	RWTPtrSlistIterator<method_decl> methIter(methods);
	method_decl *mp = NULL;

	while(++methIter == TRUE)
	{
		mp = methIter.key();
		if (mp)
			mp->html(out);
	}
}

// *******************************************
// * Print the list of data declarations
// * --> May go away! But seems useful..
// * (later: group by visibility)
// *******************************************
void res_class_decl::PrintData(ostream& out)
{
	RWTPtrSlistIterator<data_decl> dataIter(data);
	data_decl *dp = NULL;

	while(++dataIter == TRUE)
	{
		dp = dataIter.key();
		if (dp)
			dp->print(out);
		out << "\n\t";
	}
}

// *******************************************
// * Print the list of data declarations
// * --> May go away! But seems useful..
// * (later: group by visibility)
// *******************************************
void res_class_decl::htmlData(ostream& out)
{
	RWTPtrSlistIterator<data_decl> dataIter(data);
	data_decl *dp = NULL;

	while(++dataIter == TRUE)
	{
		dp = dataIter.key();
		if (dp)
			dp->html(out);
	}
}
// ***************************************
// * Print the list of class members
// *  (declarations)
// * Later: group by visibility)
// ***************************************
void res_class_decl::PrintMembers(ostream& out, int printScope)
{
	RWTPtrSlistIterator<decl> membersIter(members);
	decl *mp = NULL;

	while(++membersIter == TRUE)
	{
		mp = membersIter.key();
		out << "\t";
		if (mp)
			mp->print(out, printScope);
		out << endl;
	}
}

void res_class_decl::htmlMembers(ostream& out, int printScope)
{
	RWTPtrSlistIterator<decl> membersIter(members);
	decl *mp = NULL;

	while(membersIter())
	{
		mp = membersIter.key();

		if (mp)
			mp->html(out, printScope);
	}
}


// New
// Get a "new" object of this type, either from the free list,
// or from spawn()
Resource *res_class::New(RWCString nm, ResList *constructor_args, ResContext *constructor_context)
{
	Resource *rp = NULL;	// the new object -- either from a freelist or spawn()
	bool fromFreelist = FALSE;	// whether rp was from the freelist or not

#ifdef RSL_DEBUG_CREATE
	rslerr << "res_class::New()" << endl << flush;
#endif

	// Record HUGE amount of statistics!!
	if (freelistFile)	// runtime.cc
		freelistStats(*freelistFile);	// runtime.cc

	if (!freeList.isEmpty())
	{

#ifdef RSL_DEBUG_CREATE
		rslerr << "\tGetting from the freelist...\n";
#endif

		if (useFreelist)
			rp = freeList.get();

#ifdef RSL_DEBUG_CREATE
		rslerr << "MEM>freelist-" << name << ": " << freeList.entries() << endl;
#endif

		if (rp)
		{
#ifdef RSL_DEBUG_CREATE
			if (rp->memberOf() == this)
				rslerr << "\tresource from freelist has same class as it's freelist (good)\n";
			else
				rslerr << "\tresource from freelist does NOT have same class as it's freelist (bad)!\n";
#endif

			fromFreelist = TRUE;
			nFreelistRemoves++;	// statistics -- from runtime.cc

			// Name the "new" Resource..
			// ResStructures and its subclasses override this to also name
			// their local data contexts.
			rp->SetName(nm);
		}
		else
		{
			fromFreelist = FALSE;	// rp from freelist was null (unlikely case)

#ifdef RSL_DEBUG_CREATE
			rslerr << "Got nothing from the freelist! Creating a new one..\n";
#endif
		}

	}
	
	if (!rp)
	{
#ifdef RSL_DEBUG_CREATE
		rslerr << "\tCreating a new `" << name << "'...\n";
#endif

		// Call the spawn() virtual function
		// each res_class subclass knows how to create its respective
		// resources, eg class rc_String::spawn() creates an R_String, etc.
		// By default, res_class::spawn() creates a ResObj, which is for
		// every RSL class (no C++ resource-specific implementations)
		rp = spawn(nm);
		actualResCreations++;
	}
	
	if (rp)
	{
//		rp->NewReference();

		// *****************************************
		// * If an RSL class (ResStruct or ResObj)
		// * gotten from the freelist, then we need
		// * to install data declarations
		// *****************************************
		if (fromFreelist == TRUE && rp->isRSLStruct())	// either ResStruct or ResObj
		{
			if (inner)
			{

#ifdef RSL_DEBUG_CREATE
				rslerr << "res_class::New(): declaring data in ResStruct from freelist\n";
#endif
				ResContext& thecontext = ((ResStructure *) rp)->GetLocalContext();
				InstallDeclarations(&thecontext);
			}

#ifdef RSL_DEBUG_CREATE
			else
				rslerr << "res_class::New(): no data declarations.\n";
#endif
		}

		// *****************************************
		// * Furthermore, if it is an RSL object,
		// * then run an RSL constructor.
		// *****************************************
		if (rp->InternalType() == Resource::resObjType)
		{
			rslConstructor(((ResStructure *) rp), constructor_args, constructor_context);
		}

		// *****************************************
		// Invoke the C++ initializer
		// *****************************************
		rp->cpp_Init(constructor_args, constructor_context);

	}

#ifdef RSL_DEBUG_CREATE
	else
		rslerr << "res_class::New(): failed to create object of type `"
			<< name << "'\n";
#endif

	return rp;
}

//	rslConstructor()
//	Send the message "Init" to the object rsp, possibly with arguments and
//	possibly with a specific context.
void res_class::rslConstructor(ResStructure *rsp, ResList *constructor_args, ResContext *constructor_context)
{
	// execute superclass constructor first
	if (DerivedFromClass() && parent_class != this)
			parent_class->rslConstructor(rsp, NULL, constructor_context);	// don't relay constructor args.

//	static ResList *rl = new ResList(0);
	ResList *rl = (constructor_args? constructor_args : new ResList(0));
	rslMethod *themethod=NULL;
	ResContext& thecontext = rsp->GetLocalContext();

	
#ifdef RSL_DEBUG_CREATE
	rslerr << "Invoking RSL constructor `Init' for class `" << name << "'\n";
#endif

	Resource *rspToUse = rsp;
	// Locate constructor RSL method with zero arguments
	int findMethodStatus = 
		Resolve(Resource::theIDHash("Init"), *rl, themethod, rspToUse);
//				Resolve(Resource::theIDHash("Init"),
//					(constructor_args? (*constructor_args) : rl), themethod);

	// execute the constructor
	if (themethod)
	{
#ifdef RSL_DEBUG_CREATE
		rslerr << "\tfound constructor.. executing..\n";
#endif

		// Add access to other data sources
		if (constructor_context)	// incoming context
			thecontext.AddContext(constructor_context);
//		else	// give access to system globals
		thecontext.AddContext(runtimeStuff.SysGlobals);

		// Do it
		EventStatus et = ((ResObj *) rsp)->RSLexecute(themethod, *rl, &thecontext);


		// remove any added contexts
		if (constructor_context)
			thecontext.RemoveContext(constructor_context);
//		else
		thecontext.RemoveContext(runtimeStuff.SysGlobals);
		
	}
	else
	{
#ifdef RSL_DEBUG_CREATE
		Logf.debug(LOGRSL)
			<< "No constructor found for class `" << name << "'" << endline;
		rslerr << "No constructor found for class `" << name << "'\n";
#endif
	}

	// if no args were passed in, then a ResList was allocated
	// so clean it up.
	if (!constructor_args)
		delete rl;
}

// Delete
// Get rid of the resource `rp'.
// Takes no action based on the refcount (assumes that the caller
// has checked this appropriately) but will log a message on a
// delete of a resource with refcount > 0.
// Uses a free list instead of deleting when NO_FREELIST is not defined.
void res_class::Delete(Resource *rp)
{
	if (!rp)
		return;

#ifdef RSL_DEBUG_MEMORY
	rslerr << "res_class::Delete() on `" << (rp->Name()) << "', `";
	rp->print(rslerr);
	rslerr << "'\n";
#endif
		
	// ResReferences should always be dealt with by value
	if (rp->InternalType() == Resource::resRefType)
	{
#ifdef RSL_DEBUG_MEMORY
		rslerr << "WARNING: res_class::Delete(): found ResReference...?\n";
#endif
//		delete ((ResReference *) rp);
		return;
	}

#ifdef RSL_DEBUG_MEMORY
	// no-action refcount check
	if (rp->RefCount() > 0)
		rslerr << "res_class::Delete() on Resource `" << rp->Name()
			<< "' with refcount " << rp->RefCount() << endl;
#endif

//#ifndef NO_FREELIST
	// Must be absolutely sure we add the object to the right free list.
	res_class *rc = rp->memberOf();
	if (rc)
	{
//		if (rc->FreeListEntries() < FREELIST_MAX)
//		{

#ifdef RSL_DEBUG_MEMORY
			rslerr << "\tClearing & adding to freelist of class `"	<< (rc->Name()) << "'\n";
#endif

//			// add to freelist or delete.
//			// there should be a function for dynamically determining the
//			// freelist max for each res_class based on usage.
//			if (rc->FreeListEntries() > 400)	// temporary hack
//			{
//				// maybe call rp->Clear() ?
//				delete rp;
//			}
//			else
//			{
				rp->Clear();
				rc->AddFree(rp);
//			}			
			
#ifdef RSL_DEBUG_MEMORY
			rslerr << "MEM>freelist+" << name << ": " << freeList.entries() << endl;
#endif
			
			return;
//		}
	}

#ifdef RSL_DEBUG_MEMORY
	else
		rslerr << "res_class::Delete(): Warning: no res_class for `" << (rp->Name()) << "'\n";
#endif


//#else	// NO_FREELIST


#ifdef RSL_DEBUG_MEMORY
	rslerr << "MEM> would del object `" << name << "', but the FREELIST IS DISABLED.\n";
#endif


//#endif	// NO_FREELIST

//	delete rp;
}

// LinkToResClass (static)
// Links class declarations to an existing res_class, or creates a new
// one as necessary.
res_class *res_class::LinkToResClass(res_class_decl *rcd, const char *nm)
{
res_class *rc=NULL;

	if (rcd)
		rcd->ClassifyDecl(nm);

#ifdef RSL_DEBUG_LINK
	rslerr << "// Linking RSL class declarations with native implementations...\n";
#endif

res_class lookup(nm);	// object to find

	rc = ResClasses.find(&lookup);
	if (rc)
	{
#ifdef RSL_DEBUG_LINK
		rslerr << "// Found existing res_class for \"" << nm << "\"\n";
#endif
		rc->SetInner(rcd, imp_cpp);	// call it a c++ implementation.
		return rc;
	}

	// not found; create new.
#ifdef RSL_DEBUG_LINK
	rslerr << "// No existing res_class for \"" << nm << "\" found; creating new.\n";
#endif

	// Sept 25, 1998: native class == R_D magic with D classes
	if (rcd && rcd->hasFlag(decl::vnative))
	{
		logf->debug(LOGRSL)
			<< "Creating an R_D class for `native' RSL class `"
			<< nm << "'." << endline;

		rc = new rc_D(nm);
		rc->SetInner(rcd, imp_cpp);

		// Automatically extend Composite if there are instance variables.
		// Now.. if the rsl class has data declarations it should
		// be a composite and we need to make sure that it "extends Composite".
		// Note that res_class::SetParent() will fail unless
		// the class name is DEFAULT_SUPERCLASS or RC_NONAME!
		if (rcd->totalDataDecl > 0)
		{
			if (rc->DerivedFrom() == DEFAULT_SUPERCLASS && rc->Name() != DCOMPOSITE)
			{
				logf->info(LOGRSL) << "Setting the parent of R_D class `"
					<< rc->Name()
					<< "' to `" << DCOMPOSITE << "' because it has instance variables."
					<< endline;
				rc->SetParent(DCOMPOSITE);
			}
		}
	}
	else
	{
		rc = new res_class(nm);
		rc->SetInner(rcd, imp_rsl);
	}

	ResClasses.insert(rc);

	return rc;
}

void res_class::SetInner(res_class_decl *in, impl_t imp)
{
	if (inner == NULL)
		inner = in;
		
	if (implementation == imp_unknown)	// don't reset
		implementation = imp;
}

// AddFree
// simply add the resource to the free list and twiddle some statistics
void res_class::AddFree(Resource *r)
{
	freeList.insert(r);

	nFreelistAdds++;
}

method_decl *res_class::GetMethod(void)
{
	if (inner && !inner->methods.isEmpty())
			return inner->methods.get();
	return (method_decl *) NULL;
}

// res_class::ResolveDataMember()
// Find the named data member in the data declarations.
// If it exists, then find the named resource in rs's context,
// and return it in `returned'.
//
// Return values:
//	res_class::dataMemberFound on success
//	res_class::memberNotFound on failure.
int res_class::ResolveDataMember(RWCString& member, ResStructure *rs, Resource *& returned)
{
	if (inner && rs)
	{
		RWTPtrSlistIterator<data_decl> iter(inner->data);
		data_decl *dd = NULL;
		
		while(iter())
		{
			dd = iter.key();
			if (dd && dd->Match(member) == TRUE)
			{
				ResReference ref = rs->GetDataMember(member);

				if (ref.isValid())
				{
					returned = ref();
					return res_class::dataMemberFound;
				}
				else
				{
					returned = NULL;
					return res_class::memberNotFound;
				}
			}
		}
	}

	// check superclass
	if (DerivedFromClass())
		return parent_class->ResolveDataMember(member, rs, returned);

	returned = NULL;
	return res_class::memberNotFound;
}


// res_class::Resolve()
// Calls res_class_decl::Resolve() to find an RSL method matching the
// method and argument list. If this fails to find a method, then
// call res_class::Resolve() on the superclass, if it exists.
// Does NOT do inheritance cycle detection: eg, it is legal (but
// disasterous) to declare classes which mutually extend (inherit
// from) each other. It could be detected by asking if an object's
// ancestry contains itself.
//
// resToUse is a pointer to the Resource for which the method is being
// invoked. If it is an RSL class (ResObj) which inherits from a Resource
// with a C++ implementation [ generally Resource or ResStruct, but more
// specifically -- a Resource whose res_class overrides res_class::spawn() ]
// then resToUse is set to the sub object.
int res_class::Resolve(int method, ResList& arglist, rslMethod *& rslm, Resource *& resToUse)
{
int findMethodStatus = methodNotFound;

#ifdef RSL_EXEDETAIL
	rslerr << "res_class::Resolve()\n";
#endif

	if (inner)	// if there are declarations for this class..
	{
		findMethodStatus = inner->Resolve(method, arglist, rslm);

		if (findMethodStatus != res_class::methodNotFound)
		{
#ifdef RSL_EXEDETAIL
			rslerr << "Class `" << name << "': ";
			if (findMethodStatus == res_class::checkForCppMethod)
				rslerr << "Found prototype; may be C++ method.\n";
			else
				rslerr << "found RSL method.\n";
#endif

			return findMethodStatus;
		}
	}
	else
		logf->error(LOGRSL) << "Error: no declarations for class `"
			<< name << "'" << endline;

	// ****************************************
	// * Check superclass
	// ****************************************
			
	// At this point, we have not found a matching method declaration,
	// so we inherit methods from the superclass (if there is one).
	if (DerivedFromClass())	// sets private member `parent_class'
	{
#ifdef RSL_DEBUG_EXECUTE
		rslerr << "\t" << name << ": Checking superclass `" << derivedFrom << "'.\n";
#endif

		if (parent_class != this)
		{
			// bounce the message to the superclass.
			// if it's not found, then we have to check the
			// shared class below, otherwise return this status.
			// note that if a superclass shares a class, then
			// that will take precedence over locally shared classes.. sheesh.
			int super_result = parent_class->Resolve(method, arglist, rslm, resToUse);
			if (super_result != res_class::methodNotFound)
			{
				// Check to see if we need to use a special sub object.
				// if so, then RESET THE ARGUMENT resToUse TO THE SUBOBJECT !
				if (parent_class->hasCPPImplementation() && resToUse->isRSLStruct())
				{
					// The object named by CPP_SUBOBJECT_NAME is created and installed in
					// res_class::InstallDeclarations()
					ResReference ref = ((ResStructure *) resToUse)->GetDataMember(CPP_SUBOBJECT_NAME);
					if (ref.isValid())
						resToUse = ref();
				}

				return super_result;
			}
		}
		else	// named superclass not found
			logf->error(LOGRSL) << "Error: Unable to find class `"
				<< derivedFrom << "', parent of class `" << name << "'" << endline;

	}
#ifdef RSL_DEBUG_EXECUTE
	else
		rslerr << "\t" << name << ": No superclass.\n";
#endif


	// must not be found in superclass, so check shared class
	// this may become a list of shared classes, to be searched in
	// order, like multiple inheritance. However, it brings all
	// the problems of MI as well. It may be a good idea to introduce
	// limited versions of share -- maybe full share (methods + data),
	// share the methods, or the share data -- as three separate
	// models.
	
	if (SharedClass())	// sets private member `shared_class'
	{
#ifdef RSL_DEBUG_EXECUTE
		rslerr << "\t" << name << ": Checking shared class `" << shares << "'.\n";
#endif

		if (shared_class != this)	// recurse with superclass
			return shared_class->Resolve(method, arglist, rslm, resToUse);
		else
			logf->error(LOGRSL) << "Error: Unable to find class `" << shares << "'" << endline;
	}
#ifdef RSL_DEBUG_EXECUTE
	else
		rslerr << "\t" << name << ": No shared class.\n";
#endif

	return res_class::methodNotFound;
}


// res_class_decl::Resolve()
// Given a method and a list of arguments,
// check for a match against:
//		- method prototypes
//		- data member declarations
// Methods take precedence over data members, so that a method
// "Name()" will mask the data member "Name" (a good thing).
// Return values are from the res_class enum:
//		methodNotFound - if there is no declaration matching `member' and the arglist.
//		rslMethodFound - a declaration was found plus an rslMethod implementation
//		checkForCPPMethod - declaration found, but no rslMethod implementation.
//		isDataMember - no method found, but data declaration found.
//	return the RSL method implementation by matching wiht the method prototype.
int res_class_decl::Resolve(int member, ResList& arglist, rslMethod *& rslm)
{
RWTPtrSlistIterator<method_decl> iter(methods);
method_decl *md=NULL;

#ifdef RSL_EXEDETAIL
	rslerr << "res_class_decl::Resolve(), arglist:\n";
	arglist.print(rslerr);
#endif

	while (iter())
	{
		md = iter.key();

#ifdef RSL_EXEDETAIL
		if (md)
			rslerr << md->name << ".. ";
#endif

		if (md && md->Match(member, arglist))
		{
#ifdef RSL_DEBUG_EXECUTE
			rslerr << "\nMatched method:\n\t";
			md->print(rslerr);
#endif

			if (md->implementation)
			{
#ifdef RSLERR
				rslerr << '\n';
				md->implementation->print(rslerr);
#endif

				rslm = md->implementation;
				return res_class::rslMethodFound;
			}
			// here we've matched a method by prototype, but it has no RSL
			// implementation, so either it is implemented in C++ or there
			// is no implementation (an error).
#ifdef RSL_EXEDETAIL
			rslerr << "No RSL implementation found (must be C++).\n";
#endif

			rslm = NULL;
			return res_class::checkForCppMethod;
		}
	}
	
	// No method found. Check data members.

	rslm = NULL;
	return res_class::methodNotFound;
}

event *res_class::executeRSLMethod(RWCString methodName, ResList& arglist, ResObj& inContext)
{
	logf->error(LOGRSL) << "executeRSLMethod() not implemented." << endline;
//	rslMethod *themethod=NULL;
//
//	findMethodStatus = 
//		Resolve(Resource::theIDHash(methodName.data()), arglist, themethod, resToUse);
//
//	if (findMethodStatus == res_class::methodNotFound)
//	{
//		return NULL;
//	}
	return NULL;
}


// LinkImplementation
// 2 things: find the indicated method declaration in
// our decl list (of methods), and set a pointer from
// the method_decl to the method implementation (rslMethod).
// Then, set the pointer in the rslMethod to the method_decl
// in this list by first deleting the method_decl that the
// rslMethod owns, as they should be duplicates. We only need
// one prototype.
int res_class_decl::LinkImplementation(rslMethod *rm)
{
method_decl *impl_decl = rm->proto(), *this_decl=NULL;

#ifdef RSL_DEBUG_LINK
	rslerr << "// res_class_decl::LinkImplementation(rslMethod *) for\n";
	rm->print(rslerr);
	rslerr << endl;

	rslerr << "// looking for declaration ";
	impl_decl->print(rslerr, 1);	// 1 = print scope
	rslerr << endl;
#endif
	
	this_decl = methods.find(impl_decl); // uses method_decl::operator==
	if (this_decl == NULL)
		return 0;	// not found!

#ifdef RSL_DEBUG_LINK
	rslerr << "// LinkImplementation: found `";
	this_decl->print(rslerr);
	rslerr << "'\n";
#endif

	this_decl->implementation = rm;

	if (rm->description.length() > 0)
		this_decl->description = rm->description;

	// Get rid of extra declaration
	delete rm->proto(); impl_decl = NULL;
	rm->proto() = this_decl;

	return 1;
}

