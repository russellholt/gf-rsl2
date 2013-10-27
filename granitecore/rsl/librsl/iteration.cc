// iteration.cc
// RSL iteration: foreach, ..
// $Id: iteration.cc,v 1.2 1998/11/24 19:28:10 toddm Exp $

static char rcsid[] = "$Id: iteration.cc,v 1.2 1998/11/24 19:28:10 toddm Exp $";

#include "iteration.h"
#include "R_List.h"
#include "R_String.h"
#include "rsldefaults.h"
#include "slog.h"
#include "destiny.h"

// for 
#include "rslMethod.h"

extern ofstream rslerr;

// ****************************************************************
// * foreach
// ****************************************************************

foreach::foreach(RWCString id, event *exp, event *bod)
{
	expression = exp;
	body = bod;
	iterID = id;
}

foreach::~foreach()
{
//	delete expression;
//	delete body;
}

event *foreach::execute(ResContext *context)
{
#ifdef RSL_DEBUG_EXECUTE
	rslerr << "foreach::execute()\n";
#endif


	if (!expression)
	{
		Logf.error(LOGRSL) << "foreach: no in-expression to evaluate." << endline;
		return NULL;
	}
	
	if (iterID.length() <= 0)
	{
		Logf.error(LOGRSL) << "foreach: no iteration identifier." << endline;
		return NULL;
	}
	
	if (!body)
	{
		Logf.error(LOGRSL) << "foreach: no statement body to execute." << endline;
		return NULL;
	}

	event *toIterate = expression->execute(context);
	
	// toIterate is not the "container" resource across which
	// we'll iterate, so it must be a ResArg.
	
	if (toIterate && toIterate->isA(event::resArgKind))
	{
		if (rslMethod::traceMode)
			logf->debug(LOGAPPENV) << "\tEntering foreach " << iterID << " in ("
				<< expression << ")" << endline;

		ResReference ref = ((ResArg *) toIterate)->ref;
		ResContext iterateContext("foreach_", BUCKETS_IN_METHOD_CONTEXT);
		
		// give access to the enclosing context!
		iterateContext.AddContext(context);
		
		if (!ref.isValid())
		{
			Logf.error(LOGRSL) << "foreach: unsuccessful in-expression evaluation."
				<< endline;
			return NULL;
		}

		if (ref->HierarchyContains(R_List_ID))
		{
#ifdef RSLERR
			rslerr << "foreach: Beginning iteration on a List..\n";
#endif				
			R_String *rs = R_String::New("", "");
			iterateContext.AddReferenceTo(iterID + "_name", rs);

			RWTValSlistIterator<ResReference>
				iter(((R_List *) ref())->GetList());
			event *resultevt = NULL;
			loop_t lt;
			while(iter())
			{
				// set the name of the object's reference!
				rs->Set(iter.key().Name());

				lt = innerLoop(iter.key().RealObject(), iterateContext, resultevt);
				
				if (lt == cContinue)
					continue;
				
				if (lt == cBreak)
					break;
					
				if (lt == cReturn)
					return resultevt;
			}
		}
		else
		if (ref->isRSLStruct())
		{
#ifdef RSLERR
			rslerr << "foreach: Beginning iteration on a ResStruct..\n";
#endif				

			ResContext &rct = ((ResStructure *) ref())->GetLocalContext();

			if (!rct.GetLocals())
				return NULL;

			RWTValHashSetIterator<ResReference> iter(*(rct.GetLocals()));
			event *resultevt = NULL;
			loop_t lt;

#ifndef NO_ITER_REF
			// To the iteration context, add a variable containing
			// the name of the reference to the iteration variable
			// in its container. This variable is named the same
			// as the iteration variable, with a suffix of "_name".
			// For example, when iterating over a Table or a ResObj,
			// we may want to know the name of this object!
			// Doing this is necessary because this name is not an
			// intrinsic property of the object, so nothing about the
			// object itself will give us this information.
			// RFH apr 1, 1998
			R_String *rs = R_String::New("", "");
			iterateContext.AddReferenceTo(iterID + "_name", rs);
#endif

			while(iter())
			{
#ifndef NO_ITER_REF
				// set the name of the object's reference!
				rs->Set(iter.key().Name());
#endif

				// do a loop iteration!
				lt = innerLoop(iter.key().RealObject(), iterateContext, resultevt);
				
				if (lt == cContinue)
					continue;
				
				if (lt == cBreak)
					break;
					
				if (lt == cReturn)
					return resultevt;
			}
		}
//		else
//		if (ref->HierarchyContains(R_Integer_ID))
//		{
//
//		}
		else
			Logf.error(LOGRSL) << "foreach: iteration across objects of class `"
				<< ref.ClassName() << "' unsupported." << endline;
		
	}
	else
	{
		Logf.error(LOGRSL) << "foreach: unsuccessful in-expression evaluation."
			<< endline;
		return NULL;
	}

	return NULL;
}

foreach::loop_t foreach::innerLoop(Resource* iterRes,
		ResContext& iterContext, event *& resultevt)
{
	if (!iterRes)
	{
		resultevt = NULL;
		return cBreak;
	}

	// update the iteration identifier
	// this can be done more efficiently, though the central
	// difficulty here is that ResReferences are always dealt
	// with by value, which neccessitates a remove/add instead
	// of an update, effectively the equivalent of
	// "x <- something" in RSL but faster.

	iterContext.RemoveResource(iterID);
	iterContext.AddReferenceTo(iterID, iterRes);
	
	resultevt = body->execute(&iterContext);
	
	// check for "break" and "return".
	
	if (!resultevt)
		return cContinue;

#ifdef RSLERR
	rslerr << "foreach: got result from one body execution: `";
	resultevt->print(rslerr); rslerr << "'\n";
#endif

	if (resultevt->isA(event::controlKind))
	{
		// assume break
#ifdef RSLERR
		rslerr << "foreach: breaking out of loop with `break'.\n";
#endif				

		delete resultevt;
		resultevt = NULL;

		return cBreak;
	}
	else
	if (resultevt->isA(event::controlReqKind))
	{

#ifdef RSLERR
		rslerr << "foreach: got control request. returning.\n";
#endif				

		return cReturn;
	}

}

void foreach::print(ostream& out)
{
	out << "foreach " << iterID << " in (";
	if (expression)
		expression->print(out);

	out << ")\n";

	if (body)
		body->print(out);
}

