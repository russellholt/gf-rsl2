// tech/rsl/Request.cc
// $Id: Request.cc,v 1.4 1998/12/22 19:13:10 toddm Exp $

#include <fstream.h>
#include "b.h"
#include "runtime.h"
#include "R_String.h"
#include "slog.h"
#include "destiny.h"

#include "R_D.h"

#include "killevents.h"

extern ofstream rslerr;

int argsResolved = 0, argEventsDestroyed = 0;

// TEMPORARY HACKS in b.cc
//extern void elist___kill(event *& el);
//extern RWTPtrSlist<event> ResultsAccumulator;

Request::Request(const char* obj, const char* meth, elist *args)
	: objRequest(obj), method(meth)
{
	kind |= requestKind;
	arguments=args;
}

Request::~Request(void)
{
// Commented out TGM - 12/21/98
// I don't think we need to Rememeber in a destructor - TGM
//	Remember(arguments);
	//elist___kill((event *)arguments);
}

void Request::print(ostream& out)
{
	out << object << '.' << method;
	
	// print parenthesis and argument list, only
	// if the request is not a data member request.
	if (! (kind & event::dataMemberReqKind) )
	{
		out << "( ";
		if (arguments)
			arguments->print(out);
		out << " )";
	}
}

// addSimpleArgument()
// Simply creates and adds an R_String to the argument list.
// If the arglist is NULL, it creates that too.
void Request::addSimpleArgument(RWCString argName, RWCString argValue)
{
	if (arguments == NULL)
		arguments = (elist *) Remember(new elist);

	ResArg *ra = (ResArg *) Remember(new ResArg(R_String::New("", argValue.data())));
	ra->argName = argName;
	arguments->add(ra);
}

// Request::execute
// Most of the action happens here
// 1. Resolve arguments in the given context (variable lookups, etc)
// 2. Find object (ObjRequest::Resolve() )
// 3. Find rslMethod from object's memberOf() (res_class) by matching
//    with ResList returned from #1 (res_class and method_decl do this)
// 4. call Resource::execute() for the object (and note that this could
//    either be a C++ resource like R_String OR an RSL implemented method
event *Request::RExecute(ResContext *context, int reportErrors)
{
#ifdef RSL_DEBUG_EXEDETAIL
	rslerr << "\nRequest::execute() for\n\t";
	print(rslerr);
	rslerr << flush;
#endif

	if (!context)
	{
#ifdef RSL_DEBUG_EXEDETAIL
		rslerr << "\tgiven context is NULL. Done.\n";
#endif
		return NULL;
	}

	ResStatus stat = objRequest::Resolve(context);

	if (stat.status == ResStatus::rslFail || !stat.r)
	{
		Logf.debug(LOGAPPENV)
			<< "unable to find requested object \"" << object << "\"." << endline;
		return NULL;
	}

#ifdef RSL_DEBUG_EXEDETAIL
	rslerr << "Found resource `" << stat.r->Name() << "'. internal type `"
		<< stat.r->InternalType() << "', class name `" << stat.r->ClassName()
		<< "', TypeID `" << stat.r->TypeID() << "'.. :";
	stat.r->print(rslerr);
	rslerr << endl;
#endif

	
	// Is it a data member request?
	// this means we're requesting a data member (instance variable)
	// that is, a named part of a composite object.
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	// NOTE! HEY! As of 2.5, This way of asking for an instance
	// variable is *deprecated*. Instead, one sends a specific
	// request to the actual object. This is done via the "find"
	// method from within RSL or the "/" operator, as in a / b
	// is a binary request with "/" as the method name rather than
	// an "object request" which is what we have right here.
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	if (kind & event::dataMemberReqKind)
	{
#ifdef DEPRECATED_MESSAGES
		logf->info(LOGRSL) << "Use of '.' for data member request in `"
			<< this << "' is deprecated. Use '/' operator instead." << endline;
#endif
		// if it is an RSL
		if ((stat.r)->isRSLStruct())
			return resolveDataMember( (ResStructure *) (stat.r));

		
		// ********************
		// check for DComposite
		// ********************
		if ((stat.r)->isD())
		{
			R_D *dx = (R_D *) (stat.r);
			return 
				// slash ("/") is the member access operator.
				Remember(new ResArg( R_D::DtoR(dx->memberAccess("/", method)) ) );
		}
		
	}

	ResList rl(arguments? (arguments->entries()) : 0, this, context);
	ResolveArguments(context, rl);


#ifdef RSL_DEBUG_EXEDETAIL
	rslerr << "\n---- Begin ResList ----\n";
	rl.print(rslerr);
	rslerr << "----- End ResList -----\n";
#endif

	return executeResource(stat.r, rl, context, reportErrors);	
}

event *Request::resolveDataMember(ResStructure *rs)
{
	if (!rs)
		return NULL;
		
	res_class *rc = rs->memberOf();
	if (!rc)
	{
		logf->error(LOGAPPENV)
			<< "Error: no class for Resource `" << (rs->Name()) << "'!" << endline;

#ifdef RSL_DEBUG_EXEDETAIL
		rslerr << "resolveDataMember(): no class for Resource `" << (rs->Name()) << "', :";
		rs->print(rslerr);
		rslerr << endl;
#endif

		return NULL;
	}
	
//	Resource *themember=NULL;
//	int retval = rc->ResolveDataMember(method, rs, themember);

	// instead of matching with data declarations, i'm just going
	// to access the actual storage. this increases speed as well
	// as giving more flexibility for objects who have undeclared
	// data members (eg, R_Table)

	ResReference ref = rs->GetDataMember(method);
	if (ref.isValid())
		return Remember(new ResArg(ref()));

//	if (retval == res_class::dataMemberFound)
//		return new /* memory leak */ ResArg(themember);
		
	return NULL;
}

// Request::executeResource
// Send a message to one of three kinds of Resources:
// pure RSL (ResObj), RSL/C++ hybrid (ResStructure) and pure C++ (Resource)
event *Request::executeResource(Resource *r, ResList& rl, ResContext *context, int reportErrors)
{
	if (!r)
	{
		Logf.error(LOGRSL) << "Request: null Resource" << endline;
		return NULL;
	}
	
	if (r->RefCount() <= 0)
		Logf.error(LOGRSL) << "Accessing object of type `"
			<< r->ClassName() << "' that is on the freelist!" << endline;

	int findMethodStatus=0;
	res_class *rc = r->memberOf();
	
	if (!rc)
	{
		logf->error(LOGAPPENV)
			<< "Error: no class for Resource `" << (r->Name()) << "'!" << endline;

#ifdef RSLERR
		rslerr << "executeResource(): no class for Resource `" << (r->Name()) << "', :";
		r->print(rslerr);
		rslerr << "\n";
#endif

		return NULL;
	}

	rslMethod *themethod=NULL;
	Resource *resToUse = r;

	// Resolve method and arguments (prototype enforcement)
	findMethodStatus = 
		rc->Resolve(Resource::theIDHash(method.data()), rl, themethod, resToUse);

	if (findMethodStatus == res_class::methodNotFound)
	{
		//	if method not found, try to find a data member.
		// (that is, if we can -- ie we're a ResStructure)
		if (	r->InternalType() == Resource::resStructType
			||	r->InternalType() == Resource::resObjType)
		{
#ifdef RSLERR
			rslerr << "Request:executeResource() - checking for auto-assign data member..\n";
#endif

			ResReference memberRef = ((ResStructure *) r)->GetDataMember(method);
			
			if (memberRef.isValid())
			{
				memberRef()->Assign(rl.get(0));

#ifdef RSLERR
				rslerr << "Request: Auto-Assignment for datamember-as-method..\n";
				rslerr << "Post-assignment: Resource data member is: `";
				memberRef.print(rslerr);
				rslerr << endl;
#endif

				return NULL;	// good.
			}
		}

		if (reportErrors)
		{
			Logf.error(LOGAPPENV)
				<< "method `" << method << "' not found for request `"
				<< (this)
				<< "' in class `" << (rc->Name()) << "'" << endline;
		}
	}
	else	// method is not "not found"
	{
		if (findMethodStatus == res_class::checkForCppMethod)
			return executeCpp((resToUse?resToUse : r), rl);

		if (r->InternalType() == Resource::resObjType)
		{
			EventStatus et = ((ResObj *) r)->RSLexecute(themethod, rl, context);
			return et.evt;
		}
		else
		// Hybrid RSL/C++ object - eg, a C++ class derived from ResStructure so it
		// can have data members in RSL.
		if (r->InternalType() == Resource::resStructType)
		{
			EventStatus et = ((ResStructure *) r)->RSLexecute(method, rl, context);
			
			// The following is probably redundant.
			if (et.status == EventStatus::evtFound)
				return et.evt;
			else
				return executeCpp((resToUse?resToUse : r), rl, reportErrors);
		}
		else
		{
			// This branch should never occur, because a C++ method
			// *should* be taken care of above when
			// findMethodStatus == res_class::checkForCppMethod.
			
#ifdef RSLERR
			rslerr << "Request::executeResource() : fell down to C++ method,\n"
				<< "but skipped res_class::checkForCppMethod (doing it anyway).\n";
#endif

			return executeCpp((resToUse?resToUse : r), rl, reportErrors);
		}
	}
	
	return NULL;
}

// Request::executeCpp()
// Run a C++ method.
event *Request::executeCpp(Resource *r, ResList& rl, int reportErrors)
{
unsigned int mid = Resource::theIDHash(method.data());

#ifdef RSLERR
	rslerr << "Request: calling C++ object `" << r->Name()
		<< "' with method `" << method;

	rslerr << "' -> " << mid << endl;
#endif

	ResStatus exestat = r->execute(mid, rl);
	
	if (exestat.status == ResStatus::rslOk)
	{
		if (r && r->InternalType() == Resource::resRefType)
			return Remember(new ResArg((ResReference *) exestat.r));
		return Remember(new ResArg(exestat.r));
	}

	return NULL;
}



static Resource *cloneArgument(ResArg *rarg)
{
Resource *cloned = NULL, *to_clone = rarg->ref();

#ifdef RSL_DEBUG_EXEDETAIL
	rslerr << "Cloning (`" << to_clone->Name() << "', `";
	to_clone->print(rslerr);
	rslerr << "') gives ";
#endif

	cloned = to_clone->clone();
	if (cloned && cloned->Name().length() == 0)
		cloned->SetName("clone");

	return cloned;
}


// Request::ResolveArguments
// Resolves Arguments into resources by either looking them up in the
// given context
void Request::ResolveArguments(ResContext *context, ResList& rl)
{
#ifdef RSLERR
	rslerr << "Request::ResolveArguments()\n   - arguments are: ";
	if (arguments)
		arguments->print(rslerr);
	else
		rslerr << "(NULL)";

	rslerr << "\n";
#endif


	if (!arguments)
		return;

RWTPtrSlistIterator<event> iter(arguments->evtl);
event *e=NULL, *ex=NULL;
RWCString currentArgName;


#ifdef RSL_DEBUG_EXEDETAIL
	// elistArgKind
	rslerr << "Arguments: kind " << arguments->kind << endl;
	rslerr << "---------------- iterating arguments ---------------\n";
#endif

	while (iter())
	{	
#ifdef RSL_DEBUG_EXEDETAIL
		rslerr << '\t';
#endif

		e = iter.key();
		ex = NULL;

		if (e->isA(argKind))
			currentArgName = ((Argument *) e)->argName;
		else
			currentArgName = "";

//////////////////////////////////////
		if (e)
		{
			ex = e->execute(context);

#ifdef RSL_DEBUG_EXEDETAIL
			rslerr << " [evaluating argument `"; e->print(rslerr); rslerr << "': `";
			if (ex)
				ex->print(rslerr);
			else
				rslerr << "(null)";
			rslerr << "']\n";
#endif
		}
//////////////////////////////////////

		if (!ex)
		{
#ifdef RSL_DEBUG_EXEDETAIL
			rslerr << "(null event)\n";
#endif
			// argument resolution failed so add an argument placeholder;
			// method must be matched even if the arguments failed to be
			// found.
			rl.Add(NULL, currentArgName);

			continue;
		}


#ifdef RSL_DEBUG_EXEDETAIL
		rslerr << "kind " << ex->kind << '\t';
		ex->print(rslerr);
#endif

		// check for multiple return values!
		if (ex->isA(event::elistKind))
		{
//			// if there is only one event in the list, then we can try to
//			// be "smart" and use only that entry. So we need to continue
//			// processing with that one event below and delete the returned
//			// list IF it's not part of the program!
//			if (((elist *) ex)->entries() == 1)
//			{
//				// remove the first event from the list.
//				event *only_e = ((elist *) ex)->evtl.get();
//				if (!ex->isA(event::programCodeKind))
//					delete ex;
//				ex = only_e;
//			}
//			else
//			{
				Logf.error(LOGRSL)
					<< "Multiple values returned from an RSL method cannot be used as a "
					<< "single argument" << endline;
	
				rl.Add(NULL, currentArgName);
				continue;
//			}
		}

		// **********************
		// * Check argument kind
		// **********************
		
		// 1. resArgKind -- resource is already given, eg an Integer.
		// This may require that the resource be copied since it is
		// pass-by-value.
		if (ex->isA(event::resArgKind))
		{
#ifdef RSL_DEBUG_EXEDETAIL
			rslerr << "\t(resArgKind)\t name=`" << currentArgName << "'\t";
			((ResArg *) ex)->ref.print(rslerr);
#endif

			// Add to outgoing ResList
			// Reference to the resource, with the argument name.
#ifdef RSL_DEBUG_EXEDETAIL
			rslerr << "Add to arglist `" << currentArgName << "'\n";
#endif
			
			Resource *rToAdd = NULL;
			
			// part of the program means we must clone the resource..
			if ( ex->isA(event::programCodeKind) )
				rToAdd = cloneArgument((ResArg *) ex);
			else
			{
				rToAdd = ((ResArg *) ex)->ref.RealObject();	// could be null.
			}
				
#ifdef RSL_DEBUG_EXEDETAIL
			if (!rToAdd)
				rslerr << "Invalid argument.\n";
#endif

			rl.Add(rToAdd, currentArgName);
			argsResolved++;

		}
		else

		// 2. objReqArgKind -- a named object to look up.
		// truthfully, this is unlikely that an ObjRequestArg will be
		// returned by executing an event.
		if (ex->isA(event::objReqArgKind))
		{
			cerr << "result of event::execute() is an objReqArgKind: call Russell.\n";

			RWCString objectToFind = ((ObjRequestArg *) ex)->Object();
			RWCString newname = currentArgName;

			ResStatus stat = context->Find(objectToFind);

			if (stat.status == ResStatus::rslOk)
			{
				rl.Add(stat.r, newname);
				argsResolved++;
			}
			else
			{
				// Add a null ResReference to the ResList as an argument placeholder
				rl.Add(NULL, newname);

				Logf.error(LOGAPPENV) << "Resource `" << newname
					<< "' not found in context `"
					<< context->Name() << "'" << endline;
			}
		}
		else
		{
			// 3. etcetera 
			if (ex->isA(event::argKind))
			{
				rslerr << "Not a ResArg, Not an ObjRequestArg: Analyzing `";
				ex->print(rslerr);
				rslerr << endl;
				
//#ifdef RSL_DEBUG_EXEDETAIL
				if (ex->isA(event::argListKind))
					rslerr << "\t(argListKind)\t";
				else
				if (ex->isA(event::requestArgKind))
					rslerr << "\t(requestArgKind)\t";
				else
				if (ex->isA(event::elistArgKind))
					rslerr << "\t(elistArgKind)\t";
				else
				if (ex->isA(event::elistKind))
					rslerr << "\t(elistKind)\t";
				
//#endif

				if (ex != e)
				{
					// don't want to execute the same thing again!
					event *rete = ex->execute(context);
	
					// rete should now be a ResArg.
					if (rete && rete->isA(event::resArgKind))
					{
#ifdef RSL_DEBUG_EXEDETAIL
						rslerr << ".. got ResArg back, it is: ";
						rete->print(rslerr);
						rslerr << flush;
#endif
	
						// Add to outgoing ResList
						// Reference to the resource, with the argument name.
						if ( ex->isA(event::programCodeKind) )
							rl.Add(cloneArgument((ResArg *) rete), currentArgName);
						else
							rl.Add(((ResArg *) rete)->ref.RealObject(), currentArgName);
						
						argsResolved++;
						
					}
					else
					{
						rslerr << "non-ResArg argument: `";
						if (rete) rete->print(rslerr); else rslerr << "(null)";
						rslerr << "' ??? ??? Call Russell.\n";
						rl.Add(NULL, currentArgName);
					}

					/* obsoleted by Remember()
					if (rete != ex && rete != e)
					{
						argEventsDestroyed++;
						delete rete;
					}
					*/
				}
			}
		}

#ifdef RSL_DEBUG_EXEDETAIL
		rslerr << '\n';
#endif

		/* obsoleted by Remember()
		// if ex != e, this means ex was allocated by e->execute(..)
		if (ex && ex != e)
		{
			argEventsDestroyed++;
			delete ex;
			ex = NULL;
		}
		*/
	}

#ifdef RSL_DEBUG_EXEDETAIL
	rslerr << "-------------- done iterating arguments -------------\n";
#endif
}

