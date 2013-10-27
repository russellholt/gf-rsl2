// rslEvents.cc
// $Id: rslEvents.cc,v 1.5 1998/12/22 19:18:04 toddm Exp $

#include <fstream.h>

#include "rslEvents.h"
#include "rsldefaults.h"
#include "slog.h"
#include "destiny.h"
#include "runtime.h"

#include "R_Integer.h"
#include "R_Boolean.h"
#include "R_String.h"

#include "rslMethod.h"

#include "killevents.h"

extern ofstream rslerr;
//extern RWTPtrSlist<event> ResultsAccumulator;	// b.cc

// note the the following function is an ugly hack
// extern void elist___kill(event *& el);	// b.cc

int ifStatements=0, ifContexts=0;

// ****************************************************************
// * BinaryRequest
// ****************************************************************

BinaryRequest::BinaryRequest(void) : Request("", "")
{
	left=right=NULL;
	brKind = biUninitialized;
	kind |= event::biRequestKind;
}

BinaryRequest::BinaryRequest(event *l, const char *op, event *r, int br, int groupExpr)
	: Request("", "")
{
	kind |= event::biRequestKind;
	method = op;

	left = groupExpr ? new eventGroup(l) : l;
	right = groupExpr ? new eventGroup(r) : r;
}

BinaryRequest::~BinaryRequest()
{
#ifdef RSLERR
	rslerr << "BinaryRequest::~BinaryRequest()\n";
#endif
}


// BinaryRequest::execute()
// Evaluate a binary expression. That is, execute the left and right.
// The output of the right becomes the arguments to the output
// of the left, with method `method' !
// eg, (left) + (right) becomes leftoutput.+(rightoutput);
//
// Future: logical comparisons (and, or) performed here instead
// of in the resources. eg, for `and' (&&), if the left is false,
// there is no need to test (and execute) the right. But for
// `or' (||), if the left is true then there is no need to test
// (and execute) the right. This fits the C/++/Perl model.
event *BinaryRequest::execute(ResContext *context)
{
#ifdef RSL_DEBUG_EXEDETAIL//-----------------------------------------------------
	rslerr << "BinaryRequest::execute()\n";
#endif//------------------------------------------------------------

	if (!left)
	{
//#ifdef RSL_DEBUG_EXEDETAIL//-----------------------------------------------------
//		rslerr << "\tleft is NULL?\n";
//#endif//------------------------------------------------------------

		return NULL;
	}

//#ifdef RSL_DEBUG_EXEDETAIL//-----------------------------------------------------
//	rslerr << "BinaryRequest: executing left..\n";
//#endif//------------------------------------------------------------

	event *leftresult= left->execute(context);
	event *rightresult = NULL;
	
	if (!leftresult)
	{
//#ifdef RSL_DEBUG_EXEDETAIL//-----------------------------------------------------
//		rslerr << "BinaryRequest: left execution returned NULL..\n";
//#endif//------------------------------------------------------------

		return NULL;
	}

	if (right)
	{
#ifdef RSL_DEBUG_EXEDETAIL//-----------------------------------------------------
		rslerr << "BinaryRequest: executing right: kind " << right->kind << endl;
		right->print(rslerr);
		rslerr << endl;
#endif//------------------------------------------------------------

		rightresult= right->execute(context);
//		if (!rightresult->isA(event::programCodeKind))
//			ResultsAccumulator.insert(rightresult);
	}

//#ifdef RSL_DEBUG_EXEDETAIL//-----------------------------------------------------
//	else
//		rslerr << "BinaryRequest: right is NULL...\n";
//#endif//------------------------------------------------------------


	// leftresult should be a ResArg, though it may be an elist (??)
	if ((leftresult->kind) & event::resArgKind)
	{
		elist *thearg_elist = (elist *) Remember(new elist);

		if (rightresult)
		{
			// Check type of right result so to correctly create arglist.
			if ((rightresult->kind) & event::resArgKind)
			{
				
//#ifdef RSLERR//-----------------------------------------------------
//				rslerr << "Rightresult is a ResArg --- `";
//				rightresult->print(rslerr);
//				rslerr << "'\n";
//#endif//------------------------------------------------------------
				

				thearg_elist->add(rightresult);
			}
			else
			if ((rightresult->kind) & event::elistKind)
			{
				Logf.debug(LOGRSL)
					<< "BinaryRequest: right result is an elist!" << endline;

//#ifdef RSL_DEBUG_EXEDETAIL//----------------------------------------
//				rslerr << "BinaryRequest: right result is an elist!\n";
//#endif//------------------------------------------------------------

				// rightresult, being an elist, was allocated in elist::execute()
				// and no other place (since currently, C++ Resources can only
				// return single Resources). Thus we need to transfer its events
				// and destroy it.

				elist *therightelist = (elist *) rightresult;
				if (therightelist->isA(event::programCodeKind))
				{
					// since the right is part of the program, we
					// want to preserve it!
					RWTPtrSlistIterator<event> iter(therightelist->evtl);
					while (iter())
						thearg_elist->add(iter.key());
				}
				else
				{
					//cerr << "right elist is NOT part of the program\n";
					// transferContentsFrom() removes each element
					// from the right elist
					thearg_elist->transferContentsFrom(therightelist);
				}

//#ifdef RSL_DEBUG_EXEDETAIL
//				rslerr << "BinaryRequest: DELETE right elist.\n" << flush;
//#endif

			}

//#ifdef RSL_DEBUG_EXEDETAIL
//			else
//				rslerr << "RightResult is neither ResArg nor elist...??\n";
//#endif

		}

//#ifdef RSL_DEBUG_EXEDETAIL
//		else
//			rslerr << "Right result is NULL.\n";
//#endif

		ResList arglist(thearg_elist->entries(), this, context);
		arguments = thearg_elist;	// set the arguments for this Request

		if (thearg_elist->entries() > 0)
			ResolveArguments(context, arglist);

			
#ifdef RSL_DEBUG_EXEDETAIL
		rslerr << " [bireq-executing for left `"; leftresult->print(rslerr);
		rslerr << "']\n";
#endif

		event *outgoing = executeResource( ((ResArg *) leftresult)->ref.RealObject(),
			arglist, context);

		// Some cleanup
		arguments = NULL;	// clear arguments

		
#ifdef RSL_DEBUG_EXEDETAIL
		if (leftresult && leftresult->isA(programCodeKind))
			rslerr << "Binary: left result is part of the program\n";
	
		if (rightresult && rightresult->isA(programCodeKind))
			rslerr << "Binary: right result is part of the program\n";
		else
			rslerr << "Binary: right result NOT part of the program\n";
#endif

		// result
#ifdef RSL_DEBUG_EXEDETAIL
		rslerr << "Done with successful BinaryRequest::execute()\n";
#endif

		// Add the left result to the event accumulator list to
		// be deleted later. this is an intermediate result (temporary)
		// that would otherwise be dropped. for example, in the
		// expression 2 * 3 + 4, the order of evaluation is
		// (2 * 3) + 4. in this case, the leftresult is 2*3, the
		// value 5. This 5 then gets the argument 4 via its method "+",
		// returning the new value 9. where is the 5? right here...

		// Remember() is done around operator new so this should be unnecessary
        // if (leftresult && !leftresult->isA(programCodeKind)) 
        //     ResultsAccumulator.insert(leftresult);           

		// kill any intermediate arguments. this function kills only
		// non-program events. so, for example, in 1 + 2 + 3,
		// the evaluation order is 1 + ( 2 + 3), so the intermediate
		// value 5 is the argument to the + method of 1. where
		// does it go? right here..
		/* Remember() is done around operator new
		   so this should be unnecessary
		elist___kill((event *) thearg_elist);	// THIS IS A HACK!
		*/

		return outgoing;
	}

#ifdef RSL_DEBUG_EXEDETAIL
	else
	if ((leftresult->kind) & event::elistKind)
		rslerr << "BinaryRequest: left result is an elist!\n";
#endif

	return NULL;
}

void BinaryRequest::print(ostream& out)
{
	if (left)
		left->print(out);

	out << ' ' << method << ' ';

	if (right)
		right->print(out);
}

// ****************************************************************
// * BiRefRequest
// ****************************************************************

BiRefRequest::BiRefRequest(event *l, event *r, int groupExpr)
	: BinaryRequest(l, "<-", r, BinaryRequest::biRefAssign, groupExpr)
{

}

BiRefRequest::~BiRefRequest()
{	}

// execute()
// Evaluate the reference assignment
event *BiRefRequest::execute(ResContext *context)
{
	if (!left)
	{
		Logf.error(LOGRSL) << "Unable to resolve left side of `<-' in `"
			<< this << "'" << endline;
		return NULL;
	}

	// ******************************************
	// * There are two cases for the left side
	// * an object request, o <- expr
	// * and a subobject request, c.o <- expr
	// ******************************************

	if (left->isA(event::objReqArgKind)
		|| left->isA(event::requestArgKind))
	{
		// Evaluate the expression on the right..
		event *rightresult = right->execute(context);
		
		if (!rightresult)
		{
			// this may not strictly be an error.
			logf->info(LOGRSL) << "Unable to resolve right side of `<-' in `"
				<< this << "'" << endline;
			return NULL;
		}

		// accumulate result event for delayed deletion if it is not part of
		// the program. this same check is done later, but this should be
		// done here since 

		// Remember() is done around operator new so this should be unnecessary - TGM
        // Commented out TGM - 12/21/98
		// if (!rightresult->isA(event::programCodeKind))
		//     ResultsAccumulator.insert(rightresult);

		// Right has to have a Resource..
		if (rightresult->isA( event::resArgKind))
		{
			ResContext *theContext = context;	// context that we'll search
			RWCString OldName;	// name to replace
			Resource *NewResource = ((ResArg *) rightresult)->ref.RealObject();

			// two legal lvalue types: 1) named object, 2) container.object
			if (left->isA(event::objReqArgKind))
				OldName = ((ObjRequestArg *) left)->Object();
			else
			{	// instance variable request
				// we have the form:  container.object <- rightresult
				// so we expect `container' to be a ResStructure
				// in which we can replace `object' with `rightresult'.
				
				// left should be a Request object, which is an objRequest
				// object, on which we call execute() to only resolve its
				// object request (`container'), not the method (`object').
				
				// container.object form is a RequestArg object
				// which has a pointer to a Request...
				if (left->isA(requestArgKind))
				{
					Request *leftRequest = (Request *) (((RequestArg *) left)->req);
					OldName = leftRequest->method;

					// Find the container object in the current context
					ResStatus stat = context->Find(leftRequest->object);

					if (stat.status == ResStatus::rslOk)	// found
					{
						Resource *cContext = stat.r;	// the object returned

						// cContext is the `container' described above
						if (!cContext)
						{
							Logf.error(LOGRSL) << "Unable to resolve container of `"
								<< this << "' (NULL)" << endline;
							return NULL;
						}
						
						// Hopefully, it's a ResStructure from which we can
						// grab the ResContext.
						if (cContext->isRSLStruct())
							theContext = &((ResStructure *) cContext)->GetLocalContext();
						else
						{
							Logf.error(LOGRSL) << "Left side of `<-' in `"
								<< this << "' must be an RSL implemented object." << endline;
							return NULL;
						}
					}
					else
					{
						Logf.error(LOGRSL) << "Unable to resolve container of `"
							<< this << "'" << endline;
						return NULL;
					}
				}
				else
				{
					Logf.error(LOGRSL) << "Unable to resolve left side of `<-' in `"
						<< this << "'" << endline;
					return NULL;
				}
			}

			// ****************************************************
			// * Perform the substitution (reference assignment)  *
			// ****************************************************
			// if the event is part of the program, its object must
			// be copied!
			if (rightresult->isA(event::programCodeKind))
				theContext->Find( OldName, NewResource->clone());
			else
				theContext->Find( OldName, NewResource);

			return NULL;
		}

	}

	return NULL;
}

// ****************************************************************
// * IfRequest
// ****************************************************************

IfRequest::IfRequest(event *exp, event *t, event *f) : Request("", "")
{
	expression = exp;
	truebranch = t;
	falsebranch = f;
	kind |= ifRequestKind;
}

IfRequest::~IfRequest()
{
#ifdef RSLERR
	rslerr << "IfRequest::~IfRequest()\n" << flush;
#endif

	/*
	if (expression)
		delete expression;
	if (truebranch)
		delete truebranch;
	if (falsebranch)
		delete falsebranch;
	*/
}

event *IfRequest::execute(ResContext *context)
{
#ifdef RSLERR
	rslerr << "IfRequest::execute()\n";
#endif

	// *****************************
	// * execute the if-expression.
	// *****************************

	event *exprResult=NULL;

	if (expression)
		exprResult = expression->execute(context);
	else
	{
		Logf.error(LOGAPPENV) << "`if': Invalid boolean expression" << endline;

#ifdef RSLERR
		cerr << "`if': Invalid boolean expression\n";
#endif

		return NULL;
	}
	
	// accumulate intermediates for delayed deletion
	// Remember() is done around operator new so this should be unnecessary - TGM
    // Commented out TGM - 12/21/98
	// if (exprResult && !exprResult->isA(programCodeKind))
	//     ResultsAccumulator.insert(exprResult);

	// *****************************
	// * Determine which branch
	// *****************************

	event *theBranch=NULL;
	int proceedWithCaution = 0;

	if (!exprResult)
	{
		if (rslMethod::traceMode)
			Logf.info(LOGAPPENV) << "\tif (" << expression << "): false, but did not evaluate to a Boolean (perhaps a method or object was not found)." << endline;

		proceedWithCaution = 1;

		// return NULL;
	}

	if (proceedWithCaution)
		theBranch = falsebranch;
	else
	if ((exprResult->kind) & event::resArgKind)
	{

#ifdef RSLERR
		rslerr << "\tif: expr result is resArgKind\n";
#endif

		Resource *ther = ((ResArg *) exprResult)->ref();

		// theBranch = (ther && ther->LogicalValue() ) ? truebranch : falsebranch;
		if (ther && ther->LogicalValue() )
		{
			if (rslMethod::traceMode)
					logf->debug(LOGAPPENV) << "\tif ("
						<< expression << "): true" << endline;

			theBranch = truebranch;
		}
		else
		{
			if (rslMethod::traceMode)
			{
				logf->debug(LOGAPPENV) << "\tif (" << expression << "): false";

				if (!falsebranch)
					Logf << " (no branch)";

				Logf << endline;
			}
	
			theBranch = falsebranch;
		}
	}
	else
		theBranch = NULL;
		// return NULL;

	if (!theBranch)
		return NULL;


	// *****************************
	// * Execute a branch
	// *****************************

	// Create a context (local scope) for new variable declarations in
	// the statement block. BUT, in the future this should check to
	// see that a) it is an elist and not a single statement, and
	// b) that there are actually variable declarations (localDecl)
	// in the elist -- and should size the hash table exactly with
	// the total number of declarations instead of the default.
	
	ifStatements++;

	if (theBranch->isA(event::elistKind) && ((elist *) theBranch)->useOwnContext())
	{
		ResContext iflocals("ifLocals", BUCKETS_IN_METHOD_CONTEXT);
		iflocals.AddContext(context);
		
		ifContexts++;
	
		return theBranch->execute(&iflocals);
	}
	
	
	return theBranch->execute(context);
}

void IfRequest::print(ostream& out)
{
	out << "if (";
	if (expression)
		expression->print(out);
	else
		out << " (NULL expr) ";
	out << ")\n";

	if (truebranch)
		printBranch(truebranch, out);
	else
		out << "{ (NULL stmt) }";

	if (falsebranch)
	{
		out << "else\n";
		printBranch(falsebranch, out);
	}

	// it's ok for false branch to be null
}

// printBranch
// prints brackets if the branch is an elist of length > 1
void IfRequest::printBranch(event *branch, ostream& out, RWCString margin)
{
	int paren = ((branch->kind) & event::elistKind && ((elist *) branch)->entries() > 1);
	if (paren)
		out << margin << "{\n";

	branch->print(out);		// , margin + "    ");

	if (paren)
		out << margin << "}\n";
}

// ****************************************************************
// * LocalDecl
// ****************************************************************

// execute
// create the named resources for the given type
event *LocalDecl::execute(ResContext *context)
{

#ifdef RSLERR
	rslerr << "LocalDecl::execute() for\n";
	print(rslerr);
#endif

	if (!local)
	{

#ifdef RSLERR
		rslerr << "\t(no data_decl??)\n";
#endif

		return NULL;
	}

	if (!context)
	{

#ifdef RSLERR
		rslerr << "\t(no context)\n";
#endif

		return NULL;
	}

	local->install(context);

#ifdef RSLERR
	rslerr << "After LocalDecl::execute(), context is now:\n";
	context->print(rslerr);
	rslerr << '\n';
#endif

	// nothing to return, though it should be an EventStatus based
	// on the return value from data_decl::install()
	return NULL;
}



// ****************************************************************
// * hijackContext
// ****************************************************************

hijackContext::hijackContext(RWCString name)
{
	named_object = name;
}

event *hijackContext::execute(ResContext *context)
{
#ifdef RSLERR
	rslerr << "hijackContext::execute()\n";
#endif

	if (!context)
		return NULL;

	// 1. find context owner
	
	Resource *own = context->Owner();
	if (!own)
		return NULL;
	
	// 2. find named object

	ResStatus stat = context->Find(named_object);
	if (stat.status == ResStatus::rslFail)
	{
		Logf.error(LOGAPPENV) << "share: unable to find object `"
			<< named_object << "'" << endline;

#ifdef RSLERR
		cerr << "share: unable to find object `" << named_object << "'\n";
#endif

		return NULL;
	}

	Resource *r = stat.r;

	res_class *rc = own->memberOf();

	if (!rc)
	{
		Logf.error(LOGAPPENV) << "share: object `"
			<< own->Name() << "' has no class?!\n";
		
#ifdef RSLERR
		cerr << "share: object `" << own->Name() << "' has no class?!\n";
#endif
		
		return NULL;
	}
	
	// *********************
	// * share the methods *
	// *********************
	if (rc->shares == RC_NONAME)
	{
		rc->shares = r->ClassName();

		Logf.notice(LOGAPPENV) << "\tclass `" << rc->Name()
			<< "' shares methods of class `"
			<< rc->shares << "'" << endline;

#ifdef RSLERR
		rslerr << "\tclass `" << rc->Name() << "' shares methods of class `"
			<< rc->shares << "'\n";
#endif

	}
	
	// **************************
	// * share the actual data  *
	// * (if it's a ResStruct)  *
	// **************************
	
	if (r->isRSLStruct() && own->isRSLStruct())
	{
		ResContext &ownrc = ((ResStructure *) own)->GetLocalContext();

		// as long as data is being shared, there has to be an
		// object, otherwise the context pointer will go stale.
		ownrc.AddReferenceTo(REFNAME_TO_SHARED_OBJ, r, vPrivate);
		
#ifdef RSLERR
		rslerr << "Current owner context is:/n";
		ownrc.print(rslerr);
		rslerr << endl;
#endif
		
		ownrc.AddContext(& ( ((ResStructure *) r)->GetLocalContext()) );

#ifdef RSLERR
		rslerr << "Shared context is now:\n=======(begin shared context)\n";
		ownrc.printContextInfo(rslerr, "  ");
		ownrc.print(rslerr);
		rslerr << "\n=======(end shared context)\n";
#endif

	}
	
	return NULL;
}

void hijackContext::print(ostream& out)
{
	out << "share " << named_object;
}


// ****************************************************************
// * Argument
// ****************************************************************

Argument::Argument(void)
{
	kind=argKind;
	argName = UNNAMED_ARG;
}

event *Argument::execute(ResContext *context)
{
#ifdef RSLERR
	rslerr << "Argument::execute()\n";
#endif

	return NULL;
}

// ****************************************************************
// * elistArg
// ****************************************************************

elistArg::elistArg(void)
{
	kind |= elistArgKind;
}

elistArg::~elistArg()
{
#ifdef RSLERR
	rslerr << "  elistArg::~elistArg()\n" << flush;
#endif
}

// print a comma separated list of argument-events
void elistArg::print(ostream& out)
{
RWTPtrSlistIterator<event> ei(evtl);
event *e=NULL;
int elast=0;

	while(++ei == TRUE)
	{
		if (elast)
			out << ", ";
		e = ei.key();
		if (e)
			e->print(out);
		elast = 1;
	}
}

event *elistArg::execute(ResContext *context)
{
#ifdef RSLERR
	rslerr << "elistArg::execute()\n";
#endif

	return this;
}

// ****************************************************************
// * ObjRequestArg
// ****************************************************************

ObjRequestArg::ObjRequestArg(const char* nm)
{
	object = nm;
	kind |= objReqArgKind;
	if (object == "self")
		kind |= event::selfReqKind;
}

void ObjRequestArg::print(ostream& out)
{
	if (argName != UNNAMED_ARG)
		out << argName << ": ";
	out << object;
}

event *ObjRequestArg::execute(ResContext *context)
{
#ifdef RSLERR
	rslerr << "ObjRequestArg::execute(): find `" << object << "'\n";
#endif

	ResStatus stat = context->Find(object);
	if (stat.status == ResStatus::rslOk)	// found
	{
		return Remember(new ResArg(stat.r));
	}

	return NULL;
}


// ****************************************************************
// * ResArg
// ****************************************************************

ResArg::ResArg(Resource *res)
{
	kind |= resArgKind;
	ref.Set("", res);
}

// ResArg with a named (referenced) resource
ResArg::ResArg(ResReference *aref)
{
	kind |= resArgKind;
	if (aref)
		ref = *aref;
}

// ResArg copy constructor
ResArg::ResArg(const ResArg &ra)
{
	kind = ra.kind;
	argName = ra.argName;
	ref = ra.ref;
}

void ResArg::print(ostream& out)
{
	if (argName != UNNAMED_ARG)
		out << argName << ": ";
	ref.print(out);
}

event *ResArg::execute(ResContext *context)
{
#ifdef RSLERR
	rslerr << "ResArg::execute() !\n";
#endif

	// just return this, since execute is used to resolve or work
	// out something, and ResArg is already resolved -- it carries
	// a Resource *
	return this;
}

// ****************************************************************
// * ListArg
// ****************************************************************

ListArg::ListArg(const char* type, elist *evl)
{
	kind |= argListKind;
	argType = type;
	events = evl;	// may be NULL (and that's ok)
}


void ListArg::print(ostream& out)
{
	if (argName != UNNAMED_ARG)
		out << argName << ": ";
	out << argType << " { ";
	if (events)
		events->print(out);
	out << " } ";
}

// ListArg::execute
// Create a new Resource
event *ListArg::execute(ResContext *context)
{
#ifdef RSLERR
	rslerr << "ListArg::execute()\n";
#endif

// The following won't allow myclass{} to specify a "blank" object
//	if (!events)
//		return NULL;

	// *************************************************************************
	// Simple cases for fundamental types, eg
	//    Integer{1}
	// the only element in the list should be a ResArg, so
	// we just return that.
	// HOWEVER: this doesn't take into account errors like Integer{"hello"}.
	// Perhaps it should create a new object of the type, as done below for
	// non-fundamental types, and call Assign() with the evtl.first() resource.
	// *************************************************************************

//	if (argType == "Integer"
//			|| argType == "String"
//			|| argType == "Boolean")
//		return events->evtl.first();

	switch(Resource::theIDHash(argType.data()))
	{
		case R_Integer_ID:
		case R_String_ID:
		case R_Boolean_ID:
			return events->evtl.first();

		default:
			break;
	}
	
res_class rc(argType), *therc;

//	therc = ResClasses.find(&rc);
	if (! (therc = runtimeStuff.FindClass(argType)))
	{
		therc = runtimeStuff.FindClass("Table");	// don't use R_Table directly.
	}
	
	if (therc) // therc might still be null, so don't put in an else.
	{
		// Iterate through each event (should be Argument subclasses)
		// and resolve (execute) each one. Each result should be
		// a ResArg, that is an Argument which contains a Resource *.

		RWTPtrSlist<Resource> results;

		if (events)
		{
			RWTPtrSlistIterator<event> e_iter(events->evtl);
			event *ev=NULL, *resultev=NULL;
			Resource *resultr=NULL;

			while (e_iter())
			{
				ev = e_iter.key();
	
				// Resolve the argument ...
				resultev = ev->execute(context);
				if (resultev && resultev->isA(event::resArgKind))
				{
					resultr = ((ResArg *) resultev)->ref();
					if ((ev->kind) & event::argKind)
						resultr->SetName( ((Argument *) ev)->argName);
	
					results.append( resultr);
					// delete the ResArg ???
					// Or add it to a big list somewhere to be deleted later?
				}
			}
		}

		// Results now become the data members of a new Resource.
		Resource *newr = therc->New(argName);
		if (newr)
		{
			// SetFromInline only if non-empty
			// eg, it's ok to say myclass{}.
			if (!results.isEmpty())
				newr->SetFromInline(results);
			return Remember(new ResArg(newr));
		}
		
	}

	Logf.error(LOGSERVER) << "Error: unable to create Resource from `"
		<< this << "'" << endline;

#ifdef RSLERR
	rslerr << "Error: unable to create Resource with `";
	print(rslerr);
	rslerr << "'\n";
#endif

	return NULL;
}

// ****************************************************************
// * RequestArg
// ****************************************************************

void RequestArg::print(ostream& out)
{
	if (req)
	{
		if (argName != UNNAMED_ARG)
			out << argName << ": ";
		req->print(out);
	}
}

event *RequestArg::execute(ResContext *context)
{
#ifdef RSLERR
	rslerr << "RequestArg::execute()\n";
#endif

	if (req)
		return req->execute(context);
	return NULL;
}

// ****************************************************************
// * controlEvent
// ****************************************************************

controlEvent::controlEvent(int k, event *ev)
{
	kind = controlKind;
	ctKind = k;
	what = ev;
}

controlEvent::~controlEvent()
{
//	if (what)
	//	delete what;

    // I don't think we need to Rememeber in a destructor - TGM
    // Commented out TGM - 12/21/98
    // Remember(what);
}

void controlEvent::print(ostream& out)
{
	switch(ctKind) {
		case ctBreak: out << "break "; break;
		case ctContinue: out << "continue "; break;
		default: ;
	}
	if (what)
		what->print(out);
}


event *controlEvent::execute(ResContext *context)
{
#ifdef RSLERR
	rslerr << "controlEvent::execute()\n";
#endif

//	return NULL;
	return this;
}

// ****************************************************************
// * controlRequest
// ****************************************************************

controlRequest::controlRequest(int ck, elist *args)
	: Request("", "", args)
{
	kind |= controlReqKind;
	crFlags = ck;
}

controlRequest::controlRequest(const controlRequest& cR)
	: Request("", "", cR.arguments)
{
	kind |= controlReqKind;
	crFlags = cR.CRFlags();
}

// controlRequest::execute()
event *controlRequest::execute(ResContext *context)
{
#ifdef RSL_DEBUG_EXECUTE
	rslerr << "ControlRequest::execute() for `";
	print(rslerr);
	rslerr << "'\n";
#endif

	if (hasFlag(crReturn))
	{

#ifdef RSL_DEBUG_EXECUTE
			rslerr << "crReturn: `";
#endif
			if (arguments)
			{

#ifdef RSLERR
				arguments->print(rslerr);
				rslerr << "'\n";
#endif
				return ResolveArgsInPlace(context);
			}
#ifdef RSLERR
			else
				rslerr << "(no args)'\n";
#endif

//#ifdef RSL_DEBUG_EXECUTE
//			else
//				rslerr << "(null)";
//#endif

	}
	else
	if (hasFlag(crException))
	{

#ifdef RSL_DEBUG_EXECUTE
			rslerr << "crException";
#endif

	}

//#ifdef RSL_DEBUG_EXECUTE
	else
			rslerr << "Error: unknown controlRequest, flags " << crFlags;

	rslerr << endl;
//#endif

	return NULL;
}

// controlRequest::ResolveArgsInPlace()
// Transform Request::arguments into an elist of ResArgs.
// That is, do Request::ResolveArguments() as Request::execute() does,
// then take each Resource in the resulting ResList and simply add it
// as a ResArg to an elist.
elist *controlRequest::ResolveArgsInPlace(ResContext *context)
{
	// Create a ResList from the arguments

	ResList rl(arguments? (arguments->entries()) : 0);
	ResolveArguments(context, rl);

#ifdef RSLERR
	rslerr << "ControlRequest::ResolveArgsInPlace().. new ResList:\n";
	rl.print(rslerr);
	rslerr << "\n---------\n";
#endif

	elist *outgoingArgs = (elistArg *) Remember(new elistArg);
 
	// Step through the ResList and create an equivalent arglist 
	int i=0, len=rl.entries();
	ResArg *ra=NULL;
	for(; i<len; i++)
	{
		ra = (ResArg *) Remember(new ResArg( (ResReference *) (rl.getref(i)) ));
		ra->argName = (rl.getref(i))->Name();
		outgoingArgs->add(ra);
	}

	return outgoingArgs;
}


void controlRequest::print(ostream& out)
{
	if (hasFlag(crReturn))
		out << "return ";
	else
	if (hasFlag(crException))
		out << "/* exception */ ";	// not implemented
	else
		out << "/* unknown controlRequest kind */ ";

	if (arguments)
		arguments->print(out);
	out << ";";
}

