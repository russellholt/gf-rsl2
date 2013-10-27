// rslMethod.cc
// methods implemented in RSL.
// $Id: rslMethod.cc,v 1.2 1998/12/14 15:28:23 holtrf Exp $

#include <iostream.h>
#include <fstream.h>
#include <strstream.h>
#include <strings.h>
#include "rslMethod.h"
#include "runtime.h"
#include "rsldefaults.h"
#include "slog.h"
#include "destiny.h"

extern ofstream rslerr;
//static ofstream rslerr("rslerr");

#include "killevents.h"

int rslMethod::traceMode = 0;

rslMethod::rslMethod(void)
{
	prototype = NULL;
	body = NULL;
}

rslMethod::rslMethod(method_decl *md, event *bod, char *doc)
{
	prototype = md;
	body = bod;
	if (!body)
		rslerr << "rslMethod::rslMethod() --> null body\n";

	if (!prototype)
		rslerr << "rslMethod::rslMethod() --> null prototype\n";
	
	if (doc)
		description = doc;
}

rslMethod::~rslMethod()
{
	delete prototype;
	delete body;
}

// rslMethod::execute()
// Run an RSL-implemented method. (Arguments are already in
// the supplied context)
// Creates a local context so that the variable search order is
// 1) locals, 2) suplied context
// Usually, #2 means:
//   2.1) method arguments,
//   2.2) this object member variables,
//   2.3) enclosing context (optional)
event *rslMethod::execute(ResContext *context)
{
static char obuf[256];	// (trace mode) for the method prototype
static char rbuf[16384];	// (trace mode) for each argument (ECI syntax)
event *retev = NULL;	

	// Trace mode -- log each method prototype, and optionally, each argument.
	if (traceMode)
	{
		bzero((void *) obuf, (size_t) 256);
		ostrstream xstr(obuf, sizeof(obuf));
		prototype->print(xstr, 1);	// 2nd arg == print scope
		logf->debug(LOGAPPENV) << "Entering " << obuf << endline;

		// if 
		if (traceMode == 2 && context)
		{
			bzero((void *) rbuf, (size_t) 16384);
			RWTValHashSet<ResReference> *mlocals = context->GetLocals();
			if (mlocals)
			{
				RWTValHashTableIterator<ResReference> iter(*mlocals);
				ResReference ref;
				logf->debug(LOGAPPENV) << "  Arguments:" << endline;

				while(iter())
				{
					ref = iter.key();
					if (ref.isValid())
					{
						ostrstream rstr(rbuf, sizeof(rbuf));
						ref()->print(rstr);
						logf->debug(LOGAPPENV) << '\t' << ref.Name() << ": "
							<< rbuf << endline;
					}
				}
			}
		}
	}

#ifdef RSL_DEBUG_EXECUTE
	rslerr << "rslMethod::execute() for ";
	prototype->print(rslerr);
	rslerr << endl;
#endif

	if (!body)
	{
#ifdef RSL_DEBUG_EXECUTE
		rslerr << "Method body is empty.\n";
#endif
		return NULL;
	}

	ResContext methodlocals(prototype->name, BUCKETS_IN_METHOD_CONTEXT);	
	methodlocals.AddContext(context);
	
	if ( ! ((body->kind) & event::elistKind))
	{
#ifdef RSL_DEBUG_EXECUTE
		rslerr << "Method body is not an elist.\n";
#endif
		return NULL;
	}
		
	retev = body->execute(&methodlocals);
	
#ifdef RSL_DEBUG_EXECUTE
	rslerr << "rslMethod::execute(): done with body->execute()\n" << flush;
#endif


	if (retev && retev->isA(event::controlReqKind))
	{
		elist *returnList = NULL;
		event *cReqResult = NULL;
		controlRequest *retControlReq = (controlRequest *) retev;

		if ( retControlReq->hasFlag(controlRequest::argsResolved))
		{
			// argsResolved means that `arguments' already contains the result
			// of the executed request (that is, it's arguments evaluated)
			cReqResult = retControlReq->arguments;
			retControlReq->arguments = NULL;
			delete retControlReq;
		}	
		// Otherwise, execute the control request (eg, evaluate arguments, etc).
		else
			cReqResult= retev->execute(context);

		// If the result of a controlRequest execution is
		// an elist, want to add its *contents* to the
		// list of return values, not the list itself.
		if (cReqResult && cReqResult->isA(event::elistKind))
		{
			// hack
			if (((elist *) cReqResult)->entries() == 1)
				return ((elist *) cReqResult)->evtl.get();

			returnList = (elistArg *) Remember(new elistArg);

#ifdef RSL_DEBUG_EXECUTE
			rslerr << "rslmethod: transfer args `";
			cReqResult->print(rslerr);
			rslerr << "' from control request..\n";
#endif
			returnList->transferContentsFrom((elist *) cReqResult);
			
			// if the result was dynamically allocated (not program code), then delete it.
			if (! ((cReqResult->kind) & event::programCodeKind))
			{
#ifdef RSL_DEBUG_MEMORY
#ifdef RSL_DEBUG_EXECUTE
				rslerr << "rslMethod::execute(): DELETE a ControlRequest elist.\n" << flush;
#endif
#endif

				// obsoleted by Remember()
				//delete cReqResult;
				cReqResult=NULL;
			}
		}
//		else
//			returnList->add(cReqResult);	// can't I just return cReqResult?

		if (returnList)
			return returnList;

		return cReqResult;
	}

#ifdef RSL_DEBUG_EXECUTE
	rslerr << "\n\n";
#endif
	return retev;
}


void rslMethod::print(ostream& out)
{
	if (prototype)
	{
		if (description.length() > 0)
			out << "\n/**" << description << "*/\n";

		prototype->print(out, 1);
		
		out << "\n{\n";
		if (body)	// can have a null method body
			body->print(out);
		out << "\n}\n";
	}
}

void rslMethod::html(ostream& out)
{
	if (prototype)
	{
		// later: instead of just printing the description, should
		// interpret it like javadoc, substituting tags for hyperlinks
		// like @see
		if (description.length() > 0)
			out << "<font color=00ff00>/**" << description << "*/</font><br>\n";

//		prototype->html(out, 1);
		out << "<b>";
		prototype->print(out, 1);
		out << "</b>";

		out << "\n{\n";
		if (body)	// can have a null method body
//			body->html(out);
			body->print(out);
		out << "\n}\n";
	}
	
} 

// Comparing methods == comparing method declarations (prototypes).
int rslMethod::operator==(const rslMethod& m)
{
	if (!prototype || !m.prototype)
		return 0;

	// Comparison precedence:
	// class name
	if (prototype->memberOf != m.prototype->memberOf)
		return 0;
	if (prototype->name != m.prototype->name)
		return 0;
	if (prototype->returnType != m.prototype->returnType)
		return 0;
}

/*
int rslMethod::operator<(const rslMethod& m)
{

}
*/



