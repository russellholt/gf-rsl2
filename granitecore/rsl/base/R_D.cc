// *******************************************************************
// R_D.cc
//
// automatically generated from D.rsl and the template:
// $Id: R_D.cc,v 1.3 1998/12/04 16:41:19 holtrf Exp holtrf $
// *******************************************************************

#include "destiny.h"
#include "slog.h"
#include "R_D.h"

// D stuff
#include "D.h"
#include "DList.h"
#include "DString.h"
#include "DInt.h"
#include "DBool.h"
#include "DClass.h"

#include "DECI_OStream.h"

// Resources
#include "R_String.h"
#include "R_Integer.h"
#include "R_Boolean.h"

#define errlog logf->debug(LOGRSL)

extern void listcallback(DR_List dl, R_List *rl);
extern DR_List rlist2dlist(R_List *rl);
extern DR_Composite makeDComposite(ResReference ref);

// Method hash codes
// These are the methods defined in Object.rsl and
// implemented for basic RSL objects in class Resource
// and ResStructure.
//
// not all are used here.

#define _hCPPCLASSNAME 1097887317	// cppClassName

// basics (from Resource)
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

// more complex (from ResStructure)
#define _hADD 6382692   // add
#define _hADDPRIVATE 1734873649 // addPrivate
#define _hREMOVE 67136879   // remove
#define _hCONTAINS 33947655 // contains
#define _hDOESNOTCONTAIN 738223441  // doesNotContain
#define _hFIND 1718185572   // find
#define _hOpDiv 47  // /

#define _hISVALID 85602913	// isValid

// R_D static member
rc_D *R_D::Dmaster = new rc_D("D");

DR_ClassGroup rc_D::dclasses;

extern "C" res_class *Create_D_RC()
{
	if (!R_D::Dmaster)
		R_D::Dmaster = new rc_D("D");

	return R_D::Dmaster;
}

//// NOT IN USE
//// interface function for D classes to use
//// without compiling in knowledge of R_D, Resource, etc.
//rc_D_add(const char *n, DR_Library& lib )
//{
//	rc_D::dclasses->addClass(n, lib);
//}


// =======================================================================
// =======================================================================


// Spawn - create a new resource of this type (R_D)
// Called by res_class::New() if there is no object
// to pull off the free list.
Resource* rc_D::spawn(RWCString nm)
{
	return new R_D(nm, this);	// or other constructor
}



Resource *rc_D::New(RWCString nm, ResList *constructor_args,
	ResContext *constructor_context)
{
	errlog << "rc_D::New() for `" << Name() << "', create R_D instance."
		<< endline;

	// Get or create an R_D instance.
	Resource *r =
		res_class::New(nm, constructor_args, constructor_context);

	// for the top abstract class D, just return
	// the object.
	if (Name() == "D")
		return r;

	// Get or create the D class guts
	if (r && r->TypeID() == R_D_ID)
	{
		((R_D *) r)->owner = this;

		errlog << "get new instance from ClassGroup" << endline;

		DR_String nm = Name();
		((R_D *) r)->dref = rc_D::dclasses->create(nm);

		if (((R_D *) r)->dref.unsafe_get())
			errlog << "\n\t new instance: seems ok.." << endline;
		else
			errlog << "\n\t new instance: didn't work, it is null." << endline;
	}
	else
		errlog << "failed.. hmm.." << endline;
		
	return r;
}

void rc_D::Delete(Resource *rp)
{
	errlog << "\trc_D::Delete()!" << endline;
	res_class::Delete(rp);
}

void rc_D::AddFree(Resource *r)
{
	errlog << "\trc_D::AddFree()!" << endline;
	res_class::AddFree(r);
}


// =======================================================================
// =======================================================================



// New
// static convenience function
// See R_String::New(), R_Integer::New(), etc.
// If no functionality beyond res_class::New() is required
// (ie, no special values to be set conveniently), then this
// function simply eliminates the need to cast the result
// of res_class::New().
R_D *R_D::New(RWCString n, rc_D *own)
{
	Resource *r= own ? own->New(n) : Dmaster->New(n);

	return (R_D *) r;
}

// R_D constructor
R_D::R_D(RWCString nm, rc_D *rc)
	: Resource(nm)
{
	owner = rc;

	// when non-native declarations are allowed, some version of
	// rc->InstallDeclarations( ); should be called.
}

void R_D::cpp_Init(ResList *constructor_args, ResContext *constructor_context)
{
	// invoke superclass initialization first!
	Resource::cpp_Init(constructor_args, constructor_context);
	
	errlog << "\nR_D::cpp_Init()" << endline;
	
	dref.dump();
}

Resource* R_D::clone(void)
{
	logf->info(LOGRSL) << "R_D::clone() is a shallow copy for now." << endline;

	// shallow copy for now.
	R_D *rd = R_D::New(name);
	if (rd)
		rd->dref = dref;
	return rd;
}

// StrValue()
// Return a String version of this Resource, only if applicable.
RWCString R_D::StrValue(void)
{
	if (dref.unsafe_get())
		return dref.toString().val();
	return RWCString("");
}

// LogicalValue()
// Evaluate the "trueness" of this Resource (1 for true, 0 for false)
// Used in logical comparisons.
int R_D::LogicalValue()
{
	/*
	if (dref.isValid())
		return dref.toBoolean();

	return 0;
	*/

	return dref.isValid();
}

// IsEqual()
// Test for equality with another Resource.
// (ResStructure provides a default version)
int R_D::IsEqual(Resource *r)
{
	return (dref.compare(RtoD(r)) == c_equal);
}

// SetFromInline
// Given a list of Resources, match with a data member of the same
// name and assign. eg, in RSL, "myclass { a:1, b:2, /* etc */ }"
// an object of type `myclass' is created and SetFromInline() is called
// for the list of resources enclosed in { }.
// (ResStructure provides a default version)
void R_D::SetFromInline(RWTPtrSlist<Resource>& inliner)
{
	logf->error(LOGRSL)
		<< "R_D::SetFromInline() is not implemented." << endline;
}

// Assign
// set this resource equal to r.
// (ResStructure provides a default version)
void R_D::Assign(Resource *r)
{
	dref.assign(RtoD(r));
}

// Clear()
// Memory management - called to restore an object
// to a "just created" state, for free-list management.
// (ResStructure provides a default version)
void R_D::Clear()
{
#ifdef DMEMORY
	cerr << "R_D::Clear() : dumping the dref, from refcount ";

	if (dref.isValid())
		cerr << dref.unsafe_get()->refCount() << endl;
	else
		cerr << "(invalid dref)\n";
#endif

	dref.dump();
}

// print()
// ECI syntax
// (ResStructure provides a default version)
void R_D::print(ostream &out)
{
	if (dref.isValid())
	{
		DR_ECI_OStream eo;
		out << eo->stream(dref);
	}
}

// rslprint()
// Printing from within RSL
// (ResStructure provides a default version)
void R_D::rslprint(ostream &out)
{
	out << dref.toString();
}




// *************************************************************
// execute()
// This is the interface between RSL and C++ Resources.
//
// Automatically generated from "D.rsl"
// DO NOT MODIFY !
// *************************************************************
#define _hSET 5465460
#define _hSEND 1936027236   // send

ResStatus R_D::execute(int method, ResList& arglist)
{
	switch(method)
	{

		case _hOpDiv:
			logf->debug(LOGRSL) << "R_D: '/' reuest" << endline;

			if (arglist.theRequest && arglist.theRequest->isA(event::biRequestKind))
			{
				event *right = ((BinaryRequest *) arglist.theRequest)->right;
				
				if (right && right->isA(event::objReqArgKind))
				{
					return ResStatus(ResStatus::rslOk,
						DtoR(
							memberAccess(
								arglist.theRequest->method, // a slash
								((ObjRequestArg *) right)->Object() // name of the object
							) // memberAccess()
						  ) // DtoR()
						);
				}
			}

			errlog << "R_D: invalid '/' request" << endline;

		case _hCPPCLASSNAME:	// "cppClassName"
			return rsl_cppClassName(arglist);

        case _hISEQUAL: // "isEqual"
        case _hOpEQ2:   // "=="
		    return ResStatus(ResStatus::rslOk,
				R_Boolean::New("", IsEqual(arglist.get(0))));

		case _hLOCALVARNAMES:	// List localVarNames()
			// FOR NOW! Until D gets its own class interface representation!
			// copied from Resource::execute() because it is so short.
			return ResStatus(ResStatus::rslOk,
				memberOf()? memberOf()->localVarNames() : (Resource *) 0);

		case _hASSIGN:  // "assign"
		case _hOpASSIGN:   // ":="
			Assign(arglist.get(0));
			break;

		/*
		case _hCONTAINS: // contains
		case _hDOESNOTCONTAIN: // doesNotContain
		*/

		case _hSET:	// "Set"
			{
			  	DR_String drs(dref);
				drs = arglist[0].StrValue();
			}
			return ResStatus(ResStatus::rslOk,NULL);
			
		case _hSEND:
			return ResStatus(ResStatus::rslOk,NULL);

		case _hISVALID: // isValid
			return ResStatus(ResStatus::rslOk, R_Boolean::New("", dref.isValid()) );

		default: ;
	}
	
	return ResStatus(ResStatus::rslOk, DtoR(D_Route(arglist)));
}


// RSL method "cppClassName"
ResStatus R_D::rsl_cppClassName(const ResList& arglist)
{
	// testing...
	
	return ResStatus(ResStatus::rslOk,
	
		R_String::New("", DR_String(dref).val())
	);
}


// *******************************************
// D_Route()
// message passing interface to the D classes.
//
// Probably returns an R_D that wraps the
// response to this message.
// *******************************************
DRef R_D::D_Route(const ResList& arglist)
{
	// 0. check validity of dref
	// 1. Construct DR_Message from arglist
	// 2. dref.route(drmessage);
	// 3. analyze return value, create R_D if necessary
	//		(remember to compare with this)
	// 4. return

#ifdef DMEMORY
	if (!arglist.theRequest)
	{
		errlog << "R_D:D_Route(): no request." << endline;
		return DR_null;
	}
	else
		errlog << "R_D: routing `" << arglist.theRequest->method << "'" << endline;

	cerr << "R_D: Building message from RSL to D.." << endl;
#endif

	DR_Message m;
	m->messageCode = Resource::theIDHash(arglist.theRequest->method.data());

	RWCString arg_name;
	for(int len = arglist.entries(), i=0; i<len; i++)
	{
		arg_name = arglist[i].Name();

		// Turn the argument Resource into a D.
		// If the resource is null, that is if the
		// ResReference object holds a NULL Resource *,
		// then this should result be the object DR_null.
		// This is good; we still need an entry in the argument
		// dictionary for it.
		m->add(arg_name.data(), RtoD(arglist[i]));
	}

	// Create the message, give it the argument dictionary
	// and message name.
	m->message = arglist.theRequest->method;
	
	errlog << "\tRouting message." << endline;

	// Route the message!
	return dref.route(m);
}

// *******************************
//   member access
//   the "/" operator
// *******************************
DRef R_D::memberAccess(RWCString methodname, RWCString arg)
{
	if (!dref.isValid())
		return DR_null;

#ifdef DMEMORY
	errlog << "R_D::memberAccess: " << methodname << " ( objectName: "
		<< arg << ")" << endline;
#endif
	
	DR_Message m;
	m->messageCode = theIDHash(methodname.data());
	m->message = methodname;
	m->add("objectName", DR_String(arg));

	return dref.route(m);
}

Resource *_DtoR(DRef dr)
{
	return R_D::DtoR(dr);
}

// DtoR()
// Given a DRef, try to make a Resource.
// There are three basic cases at this time (more to come!)
// String, List, and the general case.
//
// Note! as of Oct 12, 1998 it is not quite clear what to do
// with the other primitive types: Integer & Boolean;
// these classes may simply be marked 'native' in their rsl
// declarations which will create R_D objects for them natively
// and allow them to pass between D stuff and RSL2 imperceptibly.
// This is the evolutionary strategy; whether it can be done yet
// for rsl2 is what is not clear. -russell
Resource *R_D::DtoR(DRef dr)
{
	errlog << "DtoR()";

	if (dr.isValid())
	{
		DR_Class cl = dr.Class();

		if (!cl.isValid())
		{
			errlog << "no class object; halfheartedly returning null." << endline;
			//return (Resource *) 0;
			return Dmaster->New("null");
		}

		unsigned int hc = theIDHash(cl->className());
		errlog << "\tclass object `" << cl->className() << "', code " << (int)hc << endline;

		// it is better to 
		//		switch (dr.dtypeid())
		// and only get the class object when it is not a built-in
		// type like String, Integer, Boolean or List
		switch (hc)
		{
			case _D_String_ID:
			{
				R_String *rd = R_String::New("", "");
				rd->safe_set(dr.unsafe_get());
				return rd;
			}

			case _D_List_ID:
				{
					Resource *x = R_D::refToList(dr);
					if (!x)
						return Dmaster->New("null");
					return x;
				}

			case _D_Int_ID:
				return R_Integer::New("", DR_Int(dr).i());

			case _D_Bool_ID:
				return R_Boolean::New("", DR_Bool(dr).b());

			default:
			{
				errlog << "R_D: basic implementation for arbitrary return values." << endline;
				errlog << "Looking for class `" << cl->className() << "'" << endline;

				res_class *rc = runtimeStuff.FindClass(cl->className());
				if (rc)
				{
					errlog << "\tFound res_class for class name `" << rc->Name() << "'" << endline;
					Resource *theres = rc->New("d");
					if (theres && theres->TypeID() == R_D_ID)
					{
						errlog << "\tnew resource is an R_D of type `"
							<< theres->ClassName() << "'!" << endline;

						((R_D *) theres)->dref = dr;

						if (!theres)
							return Dmaster->New("null");

						return theres;
					}
					else
						errlog << "New resource is null or is not an R_D!" << endline;
				}
				else
					errlog << "Can't find res_class!" << endline;
			}
		}
	}
	else
		errlog << "\treturn value is invalid: returning an R_D that points to NULL." << endline;

	//return NULL;

	return Dmaster->New("null");
}

DRef R_D::RtoD(ResReference ref)
{
	if (!ref.isValid())
		return DR_null;
	
	switch(ref.TypeID())
	{
		case R_D_ID:	// the best one
			return ((R_D *) ref())->dref;

		case R_List_ID:
			return rlist2dlist((R_List *) ref()); 
		
		case R_Integer_ID:
		{
			DR_Int xi;
			xi() = ((R_Integer *) ref())->intval();
			return xi;
		}

		case R_Boolean_ID:
		{
			DR_Bool xb;
			xb() = ref.LogicalValue();
			return xb;
		}

		case R_String_ID:
			// ref() points to an R_String
		  	// an R_String is a DR_String
			// and since RtoD() returns a DRef,
			// this will actually return a new DRef object
			// which points to the same DO_String that this DR_String does.
			return (DR_String) ( *( (R_String *) ref()) );

		default: ;
	}

	if (ref->isRSLStruct())
		return makeDComposite(ref);

	return DR_String(ref.StrValue());
}

R_List *R_D::refToList(DRef dr)
{
#ifdef DMEMORY
	cerr << "\t\trefToList()\n";
#endif

	R_List *rl = R_List::New("dr_list");

	listcallback(DR_List(dr), rl);
	
	return rl;
}

ResIterator *R_D::elements()
{
	return (ResIterator *)0;
}


//// *************
////  R_DIterator
//// *************
//R_DIterator::R_DIterator(DR_Collection dc)
//{
//	dre.New(dc);
//}
//int R_DIterator::hasMore()
//{
//	
//}
//ResReference R_DIterator::nextElement()
//{
//	
//}

