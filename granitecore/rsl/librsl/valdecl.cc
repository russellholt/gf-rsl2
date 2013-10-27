//#include <typeinfo>

#include "valdecl.h"

valdecl::valdecl(RWCString cl, RWCString obj, event *expr)
{
	ClassName = cl;
	ObjectName = obj;
	expression = expr;
}

event *valdecl::execute(ResContext *context)
{
	if (!expression || !context)
	{
//		cerr << "valdecl: null expression or context.\n";
		return NULL;
	}

	event *result = expression->execute(context);
	if (result && result->isA(event::resArgKind))
	{
		ResReference ref = ((ResArg *) result)->ref;

		if (!ref.isValid())
			return NULL;

		// Must copy program literals!
		if (result->isA(event::programCodeKind))
			context->AddReferenceTo(ObjectName, ref->clone());
		else
			context->AddReferenceTo(ObjectName, ref());
	}
//	else
//		cerr << "errors in valdedl::execute()...\n";
	
	return NULL;
}

void valdecl::print(ostream& out)
{
	out << ClassName << " " << ObjectName << " <- ";
	expression->print(out);
	out << ";";
}
