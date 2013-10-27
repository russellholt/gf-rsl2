// Resource.cc
// $Id: Resource.cc,v 1.4 1999/01/12 15:07:18 toddm Exp $

// *******************
// * System Includes *
// *******************
#include <fstream.h>
#include <rw/tpslist.h>

// ******************
// * Local Includes *
// ******************
#include "Resource.h"

#include "R_String.h"
#include "R_Integer.h"
#include "R_Boolean.h"

#include "rslEvents.h"
#include "rslMethod.h"

#include "slog.h"
#include "destiny.h"

#include "killevents.h"

extern ofstream rslerr;
//static ofstream rslerr("rslerr");

int ResReference::count = 0;

// statistics declared in runtime.cc
extern int nResourcesCreated, nResourcesDestroyed, nResObjs,
	nResRefsCreated, nResRefsDestroyed;

ResReference ResIterator::nextElement()
{
	return ResReference((Resource *) NULL);
}

ResIterator *ResReference::elements() {
	return (that? that->elements() : new ResIterator());
}

unsigned Resource::hash(const Resource &r)
{
//	rslerr << "Resource::hash() on `" << r.Name() << "'\n";
	return (r.Name()).hash();
}

int Resource::operator==(const Resource& r)
{
#ifdef RSLERR
	rslerr << "\nResource::operator==\n" << flush;
#endif

	return (r.Name() == name);
}

Resource::Resource(void) : refcount(0)
{
	nResourcesCreated++;
}

Resource::~Resource()
{
#ifdef RSLERR
	if (memberOf())
		rslerr << "MEM>Resource::~Resource() for `" << name << "'\n" << flush;
#endif
	nResourcesDestroyed++;
}

unsigned int Resource::TypeID(void)
{
	return memberOf()? memberOf()->TypeID()
		: (unsigned int) 0;
}

int Resource::isRSLStruct(void) const
{
	// virtual functions
	return (InternalType() & (resStructType | resObjType) );
}

int Resource::isLang() const { return (InternalType() & resLangType); }

int Resource::isD() const { return (InternalType() & DType); }


RWCString Resource::ClassName(void)
{
	if (memberOf())
		return (memberOf())->Name();
	return "";
}

ResIterator *Resource::elements()
{
	return new ResIterator; /* default: does nothing. */
}


Resource::Resource(RWCString nm) : name(nm), refcount(0)
{
	nResourcesCreated++;
}

int Resource::IsEqual(Resource *r)
{
	return (r && r->Name() == name);
}

// ****** GetSet ****** //
// *** obsolescence *** //
Resource *Resource::GetSet(RWCString& themember, Resource *f)
{
	if (f)  // set
		themember = f->StrValue();
	return R_String::New("", themember);
}

Resource *Resource::GetSet(int& themember, Resource *f)
{
    if (f && f->ClassName() == "Integer")   // set
        themember = ((R_Integer *)f)->intval();
    else
        if (f && f->ClassName() == "Boolean")
            themember = f->LogicalValue();

    return R_Integer::New("", themember);    // get
}


int Resource::NewReference(void)
{
#ifdef DEBUG_REFCOUNT
	rslerr << "MEM>new ref:" << (refcount+1)  << " on `" << name << "' (or `" << (Name())
		<< "'), type `" << ClassName() << "'\n";
#endif

	return ++refcount;
}

int Resource::DecReference(void)
{
#ifdef DEBUG_REFCOUNT
	rslerr << "MEM>dec ref:" << (refcount-1)  << " on `" << name << "' (or `" << (Name())
		<< "'), type `" << ClassName() << "'\n";
#endif

	return --refcount;
}

// Resource::HierarchyContains(classname)
// Find out whether the RSL inheritance hierarchy of this resource
// contains the named class. Recursively calls parent classes.
int Resource::HierarchyContains(RWCString classname)
{
res_class *rc = memberOf();
	if (!rc)
		return 0;

	return rc->HierarchyContains(classname);
}

// Resource::HierarchyContains(classID)
// Same as above, but using integer comparison on hash values instead of
// string comparison (better).
int Resource::HierarchyContains(unsigned int typeID)
{
res_class *rc = memberOf();
	if (!rc)
		return 0;

	return rc->HierarchyContains(typeID);
}

res_class* Resource::memberOf(void) { return (res_class *) NULL; }
RWCString Resource::StrValue() { return name; }
int Resource::LogicalValue() { return 1; }
void Resource::Clear() { name = ""; }
void Resource::print(ostream &out) { out << name; }
void Resource::rslprint(ostream &out) { out << name; }

void Resource::cpp_Init(ResList *constructor_args, ResContext *constructor_context) { }

void Resource::AddOrReplace(ResReference ref) { }


// Hash codes for built-in methods.
#define _hCLASSNAME 1965162526  // className
#define _hCLEAR 292316513   // clear
#define _hCLONE 107769710   // clone
#define _hADD 6382692   // add
#define _hREMOVE 67136879   // remove
#define _hISEQUAL 470952305 // isEqual
#define _hOpEQ2 15677   // ==
#define _hASSIGN 102593385  // assign
#define _hOpASSIGN 14909    // :=
#define _hINSTANCEOF 1197871121 // instanceOf
#define _hLOCALVARNAMES 1029205878	// localVarNames

ResStatus Resource::execute(int method, ResList& arglist)
{
#ifdef DEBUG_RSL_EXECUTE
	rslerr << "Resource::execute() for `" << name << "'\n";
#endif

    switch(method)
    {
        case _hCLASSNAME:   // "className"
		    return ResStatus(ResStatus::rslOk, R_String::New("", ClassName()));

		case _hINSTANCEOF:	// instanceOf(String className)
		    return ResStatus(ResStatus::rslOk,
				R_Boolean::New("",
					HierarchyContains(theIDHash(arglist[0].StrValue()))));

		case _hLOCALVARNAMES:	// List localVarNames()
			return ResStatus(ResStatus::rslOk,
				memberOf()? memberOf()->localVarNames() : (Resource *) 0);

        case _hCLEAR:   // "clear"
			Clear();
		    return ResStatus(ResStatus::rslOk, NULL);
 
        case _hCLONE:   // "clone"
		    return ResStatus(ResStatus::rslOk, clone());
 
        case _hISEQUAL: // "isEqual"
		    return ResStatus(ResStatus::rslOk,
				R_Boolean::New("", IsEqual(arglist.get(0))));
 
        case _hOpEQ2:   // "=="
		    return ResStatus(ResStatus::rslOk,
				R_Boolean::New("", IsEqual(arglist.get(0))));
 
        case _hASSIGN:  // "assign"
			Assign(arglist.get(0));
		    return ResStatus(ResStatus::rslOk, NULL);
 
        case _hOpASSIGN:   // ":="
		    return ResStatus(ResStatus::rslOk, NULL);
  
        default: ;
    }
	
	return ResStatus(ResStatus::rslFail);
}

void Resource::SetFromInline(RWTPtrSlist<Resource>& inliner)
{
	logf->info(LOGSERVER) << "Resource class `" << ClassName()
		<< "' does not implement inline Resource specification." << endline;;
}

void Resource::Assign(Resource *r)
{
	logf->error(LOGSERVER) << "Error: Resource class `" << ClassName()
		<< "' does not implement `Assign'." << endline;
}

Resource* Resource::clone(void)
{
	if (memberOf())
	{
		Resource *r = memberOf()->New("");
		if (r)
		{
			r->Assign(this);
			return r;
		}
	}
	return NULL;
}

// ****************************************************************
// * ResStructure
// ****************************************************************

ResStructure::ResStructure(const char *nm, const char *contextname,
			size_t localsize)
	: Resource(nm), locals(contextname, 2*(1+localsize))
{
#ifdef RSLERR
	rslerr << "ResStructure::ResStructure() for `" << nm << "'\n";
#endif

	locals.SetOwner(this);

	// Allocate local RSL class variables.
	res_class *rc = memberOf();

	if (rc)	// Note: this will be false for ResObj and its subclasses.
	{
#ifdef RSLERR
		rslerr << "new ResStructure: install declarations..\n";
#endif

		rc->InstallDeclarations(&locals);
	}
}

ResStructure::~ResStructure()
{ }


// ResStructure::SetName()
// Sets the name of the resource as does Resourc::SetName(),
// but we also need to name the local context the same as the
// object. Cool!
void ResStructure::SetName(RWCString nm)
{
	Resource::SetName(nm);
	locals.SetLocalContextName(nm);
	locals.SetOwner(this);	// just in case.
}


// ResStructure::execute(method, arglist, context)
// Uses the method name to retrieve a data member, if it exists. Otherwise,
// passes it on to Resource::execute() with a hash of method.
//
// returns EventStatus::evtFound if the method or data member was found,
// otherwise EventStatus::evtFail.
EventStatus ResStructure::RSLexecute(RWCString method, ResList& arglist,
	ResContext *context)
{
#ifdef RSLERR
	rslerr << "ResStructure::execute(method, arglist, context)\n";
	// 1. find data member (not implemented yet)
	rslerr << "\t- find data member `" << method << "' : not implemented.\n"
		<< "\tfalling back on C++.\n";
#endif

	unsigned int mid = Resource::theIDHash(method.data());
	ResStatus exestat = execute(mid, arglist);
	event *new_e = NULL;

	if (exestat.status == ResStatus::rslOk)
	{
		if (exestat.r && exestat.r->InternalType() == Resource::resRefType)
			new_e = (ResArg *) Remember(new ResArg((ResReference *) (exestat.r)));
		new_e = (ResArg *) Remember(new ResArg(exestat.r));
	}

	return EventStatus(	(exestat.status==ResStatus::rslOk?
			EventStatus::evtFound : EventStatus::evtFail),
		new_e);
}


void ResStructure::AddOrReplace(ResReference ref)
{
	ResReference it = GetDataMember(ref.Name());

	if (it.isValid() && it() != ref())	// found == replace if not the same object!
		RemoveResource(ref.Name());

	locals.AddResource(ref());
}

// ************************************************************************
// macros for ResStructure::execute . . .
// ************************************************************************

#define _hADD 6382692   // add
#define _hADDPRIVATE 1734873649 // addPrivate
#define _hREMOVE 67136879   // remove
#define _hCONTAINS 33947655 // contains
#define _hDOESNOTCONTAIN 738223441  // doesNotContain
#define _hFIND 1718185572   // find
#define _hOpDiv 47  // /
#define _hSETDYNAMICSPACE 34284377	// setDynamicSpace

// ************************************************************************
// ResStructure::execute
// Basic built-in methods for RSL, accessible from every instance
// of ResStructure. Implies that this object either declares these
// methods in rsl, or inherits from the class Object (in RSL).
// This means that the rsl class Object reflects class Resource,
// ResStructure, and ResObj, without explicitly doing so, and that
// C++ subclasses of Resource, ReStructure and ResObj insert a
// call to their respective superclass's execute() in the default
// case of their own execute().
// ************************************************************************
// Note that methods like add() and remove() change the object model
// in that they merge the concepts of aggregation and composition. Adding
// or removing from an object creates an inheritance exception...
// Suddenly, every object can have stuff added to it or removed from it.
// Subclasses should intercept these messages if they don't want to allow
// such behavior.
// ************************************************************************
// RFH April 1998
// ************************************************************************

ResStatus ResStructure::execute(int method, ResList& arglist)
{
    switch(method)
    {
        case _hCLASSNAME:   // "className"
		    return ResStatus(ResStatus::rslOk, R_String::New("", ClassName()));
 
        case _hCLEAR:   // "clear"
			Clear();
		    break;
 
        case _hCLONE:   // "clone"
		    return ResStatus(ResStatus::rslOk, clone());
 
        case _hADD:    // add(String name , x)
			locals.RemoveResource(arglist[0].StrValue());
			locals.AddReferenceTo(arglist[0].StrValue(), arglist.get(1));
		    break;

        case _hADDPRIVATE:    // addPrivate(String name , x)
			locals.RemoveResource(arglist[0].StrValue());
			locals.AddReferenceTo(arglist[0].StrValue(), arglist.get(1), vPrivate);
		    break;
		
		case _hSETDYNAMICSPACE: // setDynamicSpace
			locals.ResizeLocalSpace(R_Integer::Int(arglist.get(0)));
			break;

		case _hREMOVE:	// remove(String s)
			RemoveResource(arglist[0].StrValue());			
		    break;
 
        case _hISEQUAL: // "isEqual"
        case _hOpEQ2:   // "=="
		    return ResStatus(ResStatus::rslOk,
				R_Boolean::New("", IsEqual(arglist.get(0))));

		case _hASSIGN:  // "assign"
		case _hOpASSIGN:   // ":="
			Assign(arglist.get(0));
			break;
			
		case _hCONTAINS: // contains(String objectName)
			return ResStatus(ResStatus::rslOk,
				R_Boolean::New("",
					GetDataMember(arglist[0].StrValue()).isValid() ));

		case _hOpDiv:	// Sept 24, 1998
				if (arglist.theRequest && arglist.theRequest->isA(event::biRequestKind))
				{
					event *right = ((BinaryRequest *) arglist.theRequest)->right;
					
					if (right && right->isA(event::objReqArgKind))
					{
						return ResStatus(ResStatus::rslOk,
						(GetDataMember(((ObjRequestArg *) right)->Object())).RealObject());
					}
				}
				// FALL THROUGH
			
		case _hFIND:
		    return ResStatus(ResStatus::rslOk,
				(GetDataMember(arglist[0].StrValue()).RealObject()));

		case _hDOESNOTCONTAIN: // contains(String objectName)
			return ResStatus(ResStatus::rslOk,
				R_Boolean::New("",
					!(GetDataMember(arglist[0].StrValue()).isValid()) ));
			
		default: // Bounce to superclass.
			return Resource::execute(method, arglist);
    }

    return ResStatus(ResStatus::rslOk, NULL);	
}


// ResStructure::SetFromInline
// Given this list of resources, for each Resource r in the
// list, match r with a local object o. Set the value of o
// to equal that of r.
void ResStructure::SetFromInline(RWTPtrSlist<Resource>& inliner)
{
#ifdef RSLERR
	rslerr << "ResStructure::SetFromInline()\n";
#endif
	
	RWTPtrSlistIterator<Resource> iter(inliner);
	Resource *r=NULL;

	while(iter())
	{
		r = iter.key();
		ResStatus stat = locals.Find(r->Name());
		if (stat.status == ResStatus::rslOk)
		{
			// Calling Assign is better than removing
			// r_found and adding ref(), because r_found
			// might be pointed to by other structures and while
			// memory management would be fine, the old value
			// of r_found would still be around and lead to hard
			// to find bugs.
			stat.r->Assign(r);
		}
		else
			logf->error(LOGRSL) << "Error in inline Resource specification: `"
				<< r->Name() << "' is not a data member of class `"
				<< ClassName() << "'." << endline;
	}
}

// ResStructure::EqualOrAssign()
// Determine the equality with, or make assignment to, the given Resource.
// These are rolled into one protected method called from
// both IsEqual() and Assign() in ResStructure, because all this code
// would be duplicated almost verbatim, except for the very innermost block.
int ResStructure::EqualOrAssign(EoA_t eqOrAssign, Resource *r)
{
	if (!r)
		return 0;
	
	// Check type correctness
	
	if (eqOrAssign == equality)
		if (TypeID() != r->TypeID())
		{
			logf->error(LOGRSL)
			<< "For equality, the left-hand class `" << ClassName()
			<< "' must be identical to the right-hand class `" << r->ClassName()
			<< "'" << endline;
			return 0;
		}
	
	// class "_transfer_" is a special aggregate used for network
	// assignment to a destination object of unknown type, so we
	// want to be able to assign to it no matter what our type is.
	if (!HierarchyContains(r->TypeID()) && r->ClassName() != "_transfer_")
	{
		logf->error(LOGRSL)
			<< "In assignment, the left-hand class `" << ClassName()
			<< "' must be identical to, or a parent class of,"
			<< " the right-hand class `" << r->ClassName() << '\'' << endline;

		return 0;
	}

	if (r->InternalType() == Resource::resStructType
		|| r->InternalType() == Resource::resObjType)
	{
		ResContext& argContext = ((ResStructure *) r)->GetLocalContext();
		RWTValHashSet<ResReference>* locallist = argContext.GetLocals();
		
		// For each resource in the r's context, find a resource by
		// the same name in our context and make an assignment.
		if (locallist)
		{
			RWTValHashTableIterator<ResReference> iter(*locallist);
			ResReference ref;
			while(iter())
			{
				ref = iter.key();
				ResStatus stat = locals.Find(ref.Name());
				if (stat.status == ResStatus::rslOk)
				{
#ifdef RSLERR
					rslerr << "\tFound data member `" << ref.Name() << "'\n";
#endif

					if (eqOrAssign == equality)
					{
#ifdef RSLERR
						rslerr << "\tcalling IsEqual()..\n";
#endif
						if ( !(stat.r->IsEqual(r)) )
							return 0;
					}
					else
					if (eqOrAssign == assignment)	// just be sure...
					{
#ifdef RSLERR
						rslerr << "\tcalling Assign()..\n";
#endif
						stat.r->Assign(ref());
					}
					else
					{
#ifdef RSLERR
						rslerr << "ResStructure::EqualOrAssign() -- unknown value "
							<< eqOrAssign << endl;
#endif
						return 0;
					}
				}
				else
					logf->debug(LOGRSL) << "data member `" << ref.Name()
						<< "' not found in class `" << ClassName() << '\'' << endline;
			}
		}
	}

#ifdef RSLERR
	else
		rslerr << "Error: ResStructure::Assign(): argument is class `"
			<< r->ClassName() << "'\n";
#endif

}

void ResStructure::Clear(void)
{
	locals.Clear();		// object oriented programming at its coolest.
}

void ResStructure::RemoveResource(RWCString nm)
{
	locals.RemoveResource(nm);
}


// GetDataMember
// Find the named RSL data member. Returns a ResReference
// to emphasize memory management safety. Since returning a
// Resource * would require type checking and casting anyway,
// this is not much extra effort and it is safer. Eg,
// ResReference ref = GetDataMember("thatone");
// if (ref.isType(R_Whatever_ID))
//		((R_Whatever *) ref())->somefunc();
ResReference ResStructure::GetDataMember(RWCString theName)
{
	ResStatus stat = locals.Find(theName);
	return ResReference(stat.r);
}

// ResStructure::print
// Print an RSL-syntax formatted RSL Resource.
void ResStructure::print(ostream &out)
{
	out << ClassName() << " { ";
	RWTValHashSet<ResReference> *thedata = locals.GetLocals();
	if (thedata)
	{
		RWTValHashTableIterator<ResReference> iter(*thedata);
		ResReference ref;
		int prev=0;
		while (iter())
		{
			ref = iter.key();
			
			// only print members that have public access specifiers!
			if (ref.isPublic())
			{	// and only print a comma if there is something to print
				if (prev)
					out << ", ";
				else
					prev=1;
	
				if ((ref.Name()).length() > 0)
					out << ref.Name() << ": ";
				ref.print(out);
			}
		}
	}
	out << " }";
}

// ResStructure::rslprint
// Printing from within RSL.. this will run
// an RSL method eventually. Currently similar to ResObj::print(),
// but calls sub-data members with rslprint() instead of print(),
// and with newlines instead of commas and no brackets.
void ResStructure::rslprint(ostream &out)
{
	RWTValHashSet<ResReference> *thedata = locals.GetLocals();
	if (thedata)
	{
		RWTValHashTableIterator<ResReference> iter(*thedata);
		ResReference ref;
		int prev=0;
		while (iter())
		{
			if (prev)
				out << "\n";
			else
				prev=1;

			ref = iter.key();
			if ((ref.Name()).length() > 0)
				out << ref.Name() << ": ";
			ref.rslprint(out);
		}
	}
}

ResIterator *ResStructure::elements()
{
	return locals.elements();
}

// ****************************************************************
// * ResObj
// ****************************************************************

// constructor
// Gets the total number of data declarations in the res_class
// to size the ResContext hash table in ResStructure exactly.
// Note that the way this is invoked *requires* that rc be non-NULL!!!
// This is fine, since it should *only* be invoked from within
// the default res_class::spawn().
ResObj::ResObj(const char *nm, res_class *rc)
	: ResStructure(nm, nm, rc->DataDeclEntries())
{
#ifdef RSL_DEBUG_CREATE
	rslerr << "Construct ResObj for `" << nm << "' from class: \n";
	rc->print(rslerr);
#endif
	
	owner = rc;
	
	if (rc)
	{
		
#ifdef RSL_DEBUG_CREATE
		rslerr << "new ResObj -- install declarations\n";
#endif

		rc->InstallDeclarations(&locals);

	}

#ifdef RSLERR
	locals.print(rslerr);
#endif

	nResObjs++;
}

ResObj::~ResObj()
{ }


Resource *ResObj::clone()
{
	Resource *r = owner->New("");
	if (r)
		r->Assign(this);	// copy the data members

	return r;
}


ResStatus ResObj::execute(int method, ResList& arglist)
{
	return ResStructure::execute(method, arglist);	
}



// ResObj::execute(method, arglist, context)
// Run an rslMethod if it exists, or run Resource::execute(method, arglist)
//   if not.
// 1. Find the method implementation for the argument list by matching
//   against available class methods by prototype.
//
// 2. execute the RSL method if found, OR attempt to run an internal method
//   (Resource::execute())
//
//   A context is supplied to the method so that variable search order
// is as follows: 1. arguments, 2. object member variables, 3. supplied
// context (and all its sub-contexts)
//
// #3 might be removed: ie, `context' might actually be dangerous to have in
// the search list.
EventStatus ResObj::RSLexecute(rslMethod *themethod, ResList& arglist, ResContext *context)
{
#ifdef RSL_DEBUG_EXECUTE
	rslerr << "ResObj::execute(method, arglist, context)\n";
#endif

	if (!themethod)
	{
#ifdef RSL_DEBUG_EXECUTE
		rslerr << "\tnull method.\n";
#endif
		return EventStatus(EventStatus::evtFail);
	}

#ifdef RSL_DEBUG_EXECUTE
	rslerr << "found rsl method:\n";
	themethod->print(rslerr);

	rslerr << "\tAdding argument context to the execution context..\n";
#endif
	
	// Create argument ResContext - hash table exactly the size
	// of the arguments list.
	ResContext arc("arguments", arglist.entries());

	arc.AddResListContents(arglist);	// Search #1 - arguments
	arc.AddContext(&locals);				// Search #2 - method locals
	arc.AddContext(context);		// Search #3 - other contexts

#ifdef RSLERR
	arc.printContextInfo(rslerr);
#endif

	return EventStatus(EventStatus::evtFound,
		themethod->execute(&arc)	// do it
		);
}

// ResObj::rslprint()
// printing from within RSL -- just calls the ResStructure
// version for now; it will call an rsl method eventually.
void ResObj::rslprint(ostream &out)
{
	ResStructure::rslprint(out);
}

// ****************************************************************
// * ResReference
// ****************************************************************

#ifdef DEBUG_REFCOUNT
static void logRefState(char *msg, int num, RWCString name, Resource *that)
{
	rslerr << "MEM> " << msg << " #" << num;

	if (that)
		rslerr << " for `" << name << "': " << that->RefCount();
	else
		rslerr << " (null)";

	rslerr << endl;
}
#endif

ResReference::ResReference(void) : Resource("")
{
	nResRefsCreated++;
	visibility = vPublic;
	that = NULL;

#ifdef DEBUG_REFCOUNT
	logRefState("ResReference()", num, Name(), that);
#endif
}


ResReference::ResReference(Resource *thatone, access_t vis)
	: Resource("")
{
	that = thatone;
	nResRefsCreated++;
	visibility = vis;

	if (that)	// just in case...
		that->NewReference();	// increment reference count

#ifdef DEBUG_REFCOUNT
	logRefState("newref -> ResReference(R)", num, Name(), that);
#endif
}

ResReference::ResReference(RWCString nm, access_t vis)
	: Resource(nm)
{
	that = NULL;
	nResRefsCreated++;
	visibility = vis;

#ifdef DEBUG_REFCOUNT
	logRefState("ResReference(N)", num, Name(), that);
#endif
}

ResReference::ResReference(RWCString nm, Resource *thatone, access_t vis)
	: Resource(nm)
{
	that = thatone;
	nResRefsCreated++;
	visibility = vis;

	if (that)	// just in case...
		that->NewReference();	// increment reference count

#ifdef DEBUG_REFCOUNT
	logRefState("newref -> ResReference(N,R)", num, Name(), that);
#endif
}

// Copy constructor
ResReference::ResReference(const ResReference& rr)
{
	nResRefsCreated++;

//#ifdef DEBUG_REFCOUNT
//	rslerr << "MEM>Copy ResReference for `" << rr.Name()
//		<< "' from #" << rr.num << " to this #" << num;
//#endif

	visibility = rr.Visibility();

	that = rr();

	if (that)
	{
//#ifdef DEBUG_REFCOUNT
//		rslerr << "\n  ->call newref: ";
//#endif

		NewReference();

//#ifdef DEBUG_REFCOUNT
//		rslerr << " -> newref: " << that->RefCount() << endl;
//#endif

	}

//#ifdef DEBUG_REFCOUNT
//	else
//		rslerr << " (no object)\n";
//#endif

	name = rr.Name();
}

ResReference::~ResReference()
{
#ifdef DEBUG_REFCOUNT
//	rslerr << "MEM> ~ResReference() -> decref, #" << num;
//
//	if (that)
//		rslerr << " for `" << name << "': " << that->RefCount() << endl;
//	else
//		rslerr << " (null)\n";
	logRefState("~ResReference() -> decref,", num, Name(), that);
#endif

	nResRefsDestroyed++;

	DecReference();
}


RWCString ResReference::Name(void) const
{
	return (name.length() > 0) ? name
		: (that? that->Name() : RWCString(""));
}

RWCString ResReference::ClassName(void)
{
	return (that? that->ClassName() : RWCString(""));
}

unsigned int ResReference::TypeID(void)
{
	return (that? that->TypeID() : (unsigned int) 0);
}

int ResReference::IsEqual(Resource *r)
{
	return that? (that->IsEqual(r)) : 0;
}

int ResReference::isType(unsigned t)
{
	return that? (that->TypeID() == t) : 0;
}

res_class * ResReference::memberOf(void)
{
	return (that?(that->memberOf()) : (res_class*) NULL);
}

RWCString ResReference::StrValue(void)
{
	if (that != NULL)
		return that->StrValue();
	return name;
//	return that? that->StrValue() : name;
}

int ResReference::LogicalValue()
{
	return that? that->LogicalValue() : 0;
}

Resource* ResReference::clone(void)
{
	return (that?(that->clone()) : (Resource *) NULL);
}

void ResReference::Clear()
{
	if (that)
		that->Clear();
}

void ResReference::print(ostream &out)
{
	if (that)
		that->print(out);
	else
		Resource::print(out);
}

void ResReference::rslprint(ostream &out)
{
	if (that)
		that->rslprint(out);
	else
		Resource::rslprint(out);
}


void ResReference::AddOrReplace(ResReference ref)
{
	if (that)
		that->AddOrReplace(ref);
}




// Assignment
ResReference& ResReference::operator=(const ResReference& rr)
{
#ifdef DEBUG_REFCOUNT
	rslerr << "MEM>Assign ResReference for `" << rr.Name() << "'\n";
#endif

	// Set() takes care of decrementing the refcount on the
	// existing resource (if there is one), etc.

	if (this != (&rr))	// don't want to accidentally increment
	{
		// (would such a self-assignment be optimized out by the compiler?)
		Set(rr.Name(), rr.RealObject());
		visibility = rr.Visibility();
	}

#ifdef DEBUG_REFCOUNT
	rslerr << "MEM>\t(assign resref) returning *this..\n";
#endif


#ifdef DEBUG_REFCOUNT
	logRefState("Set -> ResReference::operator=()", num, Name(), that);
#endif

	return (*this);
}

// ResReference equality -- by name only
int ResReference::operator==(const ResReference& rr)
{
	return (Name() == rr.Name());
}

// ResReference::Set
// give a new name to the resource, or set a new name and a new
// resource.
// Eliminate references to references to references... etc...
void ResReference::Set(RWCString nm, Resource *thatone)
{
	name = nm;
	if (that == thatone)	// avoid unnecessarities
		return;

	DecReference();

	// extract the real resource object from a ResReference, if given.
	Resource *theres = thatone;
	while(theres && theres->InternalType() == Resource::resRefType)
		theres = ((ResReference *) theres)->RealObject();


	if (theres)
	{
		that = theres;
		that->NewReference();
	}
	else
		that = NULL;
}

ResStatus ResReference::execute(int method, ResList& arglist)
{
#ifdef RSL_DEBUG_EXECUTE
	rslerr << "ResReference::execute() on `" << name << "...";
#endif

	if (that)
	{
#ifdef RSL_DEBUG_EXECUTE
		rslerr << " -> object `" << (Name()) << ".'\n";
#endif
		return that->execute(method, arglist);
	}

#ifdef RSL_DEBUG_EXECUTE
	rslerr << " No Resource!\n";
#endif

	return ResStatus(ResStatus::rslFail);
}

// ResReference::NewReference
// Increment the object's refcount and return it. But if it is
// null, simply return 1 because ResReferences are not
// allocated dynamically.
int ResReference::NewReference(void)
{
	if (that && that != this)
	{
		int nr = that->NewReference();
		
#ifdef DEBUG_REFCOUNT
	logRefState("ResRef ref+", num, Name(), that);
#endif
		return nr;
	}

	return 1;
}

// ResReference::DecReference()
// Decrement the object's refcount and return it. But if it is null,
// must simply return 1, because ResReferences are always used by
// *value* (as a local variable, automatic allocation), not dynamically.
int ResReference::DecReference(void)
{
	if (that && that != this)
	{
		int dr = that->DecReference();

#ifdef DEBUG_REFCOUNT
	logRefState("ResRef ref-", num, Name(), that);
#endif

		if (dr <= 0)
		{
			res_class *rc = that->memberOf();
			if (rc)
			{
#ifdef DEBUG_REFCOUNT
				rslerr << " -> res_class::Delete().\n";
#endif
				rc->Delete(that);
			}
			else
			{
#ifdef DEBUG_REFCOUNT
				rslerr << "no resclass found. DELETING...\n";
#endif
				delete that;
				that = NULL;
			}
		}
#ifdef DEBUG_REFCOUNT
		else
			rslerr << "dec ref.\n";
#endif

		return dr;
	}

	return 1;
}

unsigned ResReference::hash(const ResReference &r)
{
//    rslerr << "ResReference::hash() on '" << r.Name() << "'\n";
    return(r.Name()).hash();
}

// ****************************************************************
// * ResStatus
// ****************************************************************

ResStatus::ResStatus(void)
{
	r = (Resource *) NULL;
	status = rslOk;
}

ResStatus::ResStatus(int st, Resource *res)
{
	r = res;
	status = st;
}

// ****************************************************************
// * ResList
// ****************************************************************

ResList::ResList(int sz, Request *req, ResContext *ctxt)
{
	if (sz > 0)
		items = new ResReference[sz];
	else
		items = NULL;

	size = sz;
	nextinsert=0;
	
	theRequest = req;
	enclosingContext = ctxt;
}

ResList::~ResList()
{
#ifdef RSL_DEBUG_EXECUTE
	rslerr << "ResList::~ResList()\n" << flush;
#endif

	delete[] items;

	// since we don't own theRequest or the enclosingContext
	// we can just forget about them.
}

void ResList::Add(Resource *r)
{
#ifdef RSL_DEBUG_EXECUTE
	rslerr << "ResList::Add(r)...\n";
#endif

	// it's ok for r to be NULL, but a pseudoname must be
	// given if so (not RL_NULL_RESOURCE).
	if (nextinsert >= 0 && nextinsert < size)
	{
		if (r)
		{
			// strip references to references, etc.
			Resource *ther=r;
			while(ther && ther->InternalType() == Resource::resRefType)
				ther = ((ResReference *) ther)->RealObject();

			if (ther)	// make really sure
			{
				items[nextinsert++].Set(
					ther->Name(), ther);
			}
		}
	}
}

void ResList::Add(Resource *r, RWCString pseudoname)
{
#ifdef RSL_DEBUG_EXECUTE
	rslerr << "ResList::Add(r, `" << pseudoname << "')\n";
#endif

	// it's ok for r to be NULL, but a pseudoname must be
	// given if so (not RL_NULL_RESOURCE).
	if (nextinsert >= 0 && nextinsert < size)
	{
		if (r || pseudoname != RL_NULL_RESOURCE)
		{
			// strip references to references, etc.
			Resource *ther=r;
			while(ther && ther->InternalType() == Resource::resRefType)
				ther = ((ResReference *) ther)->RealObject();

//			if (ther)	// make really sure
//				items[nextinsert++].Set(
//					(pseudoname==RL_NULL_RESOURCE? ther->Name() : pseudoname),
//					ther);

			items[nextinsert++].Set(
				(pseudoname==RL_NULL_RESOURCE? ther->Name() : pseudoname),
				ther);

		}
#ifdef RSL_DEBUG_EXECUTE
		else
			rslerr << "Invalid ResList::Add(): null resource OR no given pseudoname.\n";
#endif

	}
}

// ResList::get()
// Get the real resource by name (reference name, not resource name)
// Return the resource
Resource *ResList::get(const char *nm) const
{
int i=0;
	for (; i<size; i++)
		if (items[i].Name() == nm)
			return items[i].RealObject();
	return NULL;
}

// ResList::getref()
// Get a ResReference to a resource by reference name
// return the ResReference
Resource *ResList::getref(const char *nm) const
{
int i=0;
	for (; i<size; i++)
		if (items[i].Name() == nm)
			return items+i;
	return NULL;
}

void ResList::print(ostream& out) const
{
int i=0;
	for (; i<size; i++)
	{
		out << (items[i].ClassName()) << ",\t" << (items[i].Name()) << ",\t";
		items[i].print(out);
		out << '\n';
	}
}

// *************************************************************
// * theIDHash
// *   Hash an arbitrary string to 4 bytes of an unsigned int.
// *************************************************************
unsigned int Resource::theIDHash(const char *s0) //, unsigned int key=0)
{
char values[4] = {'\0', '\0', '\0', '\0'};

	if (!s0) return 0;
	
	int result=0, i, len = strlen(s0), pos=0;

	for(i=0; i<len; i++, (++pos) %= 4)
		if (values[pos])	// non zero
			values[pos] ^= s0[i];
		else
			values[pos] = s0[i];	// retain original value

	if (len > 4)	// 4 or less
		len = 4;

	for(i=0; i<len; i++)
	{
//		values[i] ^= (key & f);	// xor with right 8 bits of key
//		key = (key >>8);	// ready for next 8 bits
		result = (result <<8)
			+ (char) values[i];	// OR next byte value
	}
	return result;
}
