// $Id: R_List.cc,v 1.1 1998/11/17 23:53:23 toddm Exp $

#include <fstream.h>
#include <rw/tpslist.h>

#include "R_List.h"
#include "R_Integer.h"
#include "R_String.h"
#include "rslEvents.h"
#include "destiny.h"
#include "runtime.h"
#include "slog.h"

extern ofstream rslerr;

extern "C" res_class *Create_List_RC()
{
	return &(R_List::rslType);
}

// R_List method name hash definitions
#define _hAPPEND 252997733	// append
#define _hAPPENDCOPY 2137862922	// appendCopy
#define _hPREPEND 354156912	// prepend
#define _hPREPENDCOPY 2053929011	// prependCopy
#define _hFRONT 309489518	// front
#define _hREMOVEFRONT 1802395421	// removeFront
#define _hTAIL 1952541036	// tail
#define _hREMOVETAIL 1835809038	// removeTail
#define _hFINDBYVALUE 1214602501	// findByValue
#define _hOpBrack 23389	// []
#define _hDELETEBYNAME 1583432569	// deleteByName
#define _hDELETEBYINDEX 560876153	// deleteByIndex
#define _hDELETEBYVALUE 593576553	// deleteByValue
#define _hOpASSIGN 14909    // :=
#define _hOpEQ 61	// =	(OBSOLETE)
#define _hLENGTH 403533415  // length
#define _hCLEAR 292316513   // clear
#define _hOpMult 42 // *
#define _hTRANSFORM 1779699228  // transform
#define _hDISTRIBUTE 1650790657	// distribute
#define _hDISTRIBUTEBYTYPE 907813661	// distributeByType
#define _hBEFORE 268461679  // before
#define _hAFTER 325481573   // after
#define _hSUBLIST 436606572 // sublist

class ListResIterator : public ResIterator {
  RWTValSlistIterator<ResReference> iter;

  public:
	ListResIterator(RWTValSlist<ResReference>& rl) : iter(rl) { }

	virtual int hasMoreElements() { return (int) (iter()); }
	virtual ResReference nextElement() { return iter.key(); }
};

ResIterator *R_List::elements()
{
	return new ListResIterator(locals);
}

rc_List R_List::rslType("List");

Resource *rc_List::spawn(RWCString aname)
{
	return new R_List(aname);
}

R_List::R_List()
{ }

R_List::R_List(RWCString n) : Resource(n)
{ }

R_List::~R_List()
{
	Clear();
}

void R_List::Clear()
{
	locals.clear();
}

void R_List::print(ostream &out)
{
RWTValSlistIterator<ResReference> iter(locals);
ResReference ref;
int prev=0;

	out << "List " << "{ ";
	while(++iter == TRUE)
	{
		if (prev)
			out << ", ";
		else
			prev=1;

		ref = iter.key();
		ref.print(out);
	}
	out << '}';
}

void R_List::rslprint(ostream &out)
{
RWTValSlistIterator<ResReference> iter(locals);
ResReference ref;

	while(++iter == TRUE)
	{
		ref = iter.key();
		ref.rslprint(out);
		if (ref.TypeID() == R_String_ID)
			out << endl;
	}
}

Resource *R_List::clone(void)
{
//	Resource *r = new R_List();
	R_List *r = R_List::New("clone");
	r->GetList() = locals;
	return (Resource *) r;
}

void R_List::append(Resource *r)
{
	if (r)
	{
		ResReference aref(r->Name(), r);
		locals.insert(aref);
	}
}

// SetFromInline
// Clear and append
void R_List::SetFromInline(RWTPtrSlist<Resource>& inliner)
{
	RWTPtrSlistIterator<Resource> iter(inliner);
	locals.clear();
	while(iter())
		append(iter.key());
}

void R_List::Assign(Resource *r)
{
	if (r && r->TypeID() == R_List_ID)
		locals = ((R_List *) r)->GetList();
}


ResStatus R_List::execute(int method, ResList& arglist)
{
	switch(method)
	{
		case _hAPPEND:	// "append"
			return rsl_append(arglist);

		case _hAPPENDCOPY:	// "appendCopy"
			return insertCopy(arglist, /* atHead: */ FALSE);

		case _hPREPEND:	// "prepend"
			return rsl_prepend(arglist);

		case _hPREPENDCOPY:	// "prependCopy"
			return insertCopy(arglist, /* atHead: */ TRUE);

		case _hFRONT:	// "front"
			return rsl_front(arglist);

		case _hREMOVEFRONT:	// "removeFront"
			return rsl_removeFront(arglist);

		case _hTAIL:	// "tail"
			return rsl_tail(arglist);

		case _hREMOVETAIL:	// "removeTail"
			return rsl_removeTail(arglist);

		case _hFINDBYVALUE:	// "findByValue"
			return rsl_findByValue(arglist);

		case _hOpBrack:	// "[]"
			return OpBrack(arglist);

		case _hDELETEBYNAME:	// "deleteByName"
			return rsl_deleteByName(arglist);

		case _hDELETEBYINDEX:	// "deleteByIndex"
			return rsl_deleteByIndex(arglist);

		case _hDELETEBYVALUE:	// "deleteByValue"
			return rsl_deleteByValue(arglist);

		case _hOpASSIGN:	// ":="
		case _hOpEQ:	// "="	(OBSOLETE)
			return OpEQ(arglist);

		case _hLENGTH:	// "length"
			return rsl_length(arglist);

		case _hCLEAR:	// "clear"
			return rsl_clear(arglist);

		case _hOpMult:	// "*"
		case _hTRANSFORM:   // "transform"
			 return rsl_transform(arglist);

		case _hDISTRIBUTE:	// "distribute"
			return rsl_distribute(arglist);

		case _hDISTRIBUTEBYTYPE:	// "distributeByType"
			return rsl_distributeByType(arglist);			
		 
        case _hBEFORE:  // "before"
            return rsl_before(arglist);
 
        case _hAFTER:   // "after"
            return rsl_after(arglist);
 
        case _hSUBLIST: // "sublist"
            return rsl_sublist(arglist);
		
		default: ;
	}

	return ResStatus(ResStatus::rslFail);
}

ResStatus R_List::rsl_append(const ResList& arglist)
{
register int i=0, len=arglist.entries();

	for (; i<len; i++)
		locals.append(arglist[i]);

	return ResStatus(ResStatus::rslOk);
}

// rsl_insertCopy()
// add a copy of each argument to either the head or
// the tail of the list, depending on the value of the
// atHead boolean.
//
// To fully understand how the copy works, look up
// Resource::clone(). Consider the difference between cloning
// a Resource and cloning a ResObj. Also consider what happens
// if the ResObj extends (in RSL) a Resource like R_Transaction.
ResStatus R_List::insertCopy(const ResList& arglist, bool atHead)
{
register int i=0, len=arglist.entries();

	// iterate through each argument.
	for (; i<len; i++)
	{
		ResReference ref = arglist[i];

		if (ref.isValid())
		{
			Resource *newobj = ref->clone();	// ResReference::operator->()

			if (newobj)
			{
				if (atHead)
					locals.prepend(ResReference(newobj));
				else
					locals.append(ResReference(newobj));
			}
			else
				Logf.error(LOGAPPENV)
					<< "List::" << (atHead? "prepend" : "append")
			  		<< "Copy(): attempt to clone argument failed."
			  		<< endline;
		}
	}

	return ResStatus(ResStatus::rslOk);
}

ResStatus R_List::rsl_prepend(const ResList& arglist)
{
register int i=0, len=arglist.entries();

	for (; i<len; i++)
		locals.prepend(arglist[i]);

	return ResStatus(ResStatus::rslOk, NULL);
}


ResStatus R_List::rsl_front(const ResList& arglist)
{
	Resource *retval = NULL;
	if (!locals.isEmpty())
		retval = (locals.first()).RealObject();
	return ResStatus(ResStatus::rslOk, retval);
}


ResStatus R_List::rsl_removeFront(const ResList& arglist)
{
	Resource *retval = NULL;

	if (!locals.isEmpty())
		retval = locals.removeFirst().RealObject();

	return ResStatus(ResStatus::rslOk, retval);
}


ResStatus R_List::rsl_tail(const ResList& arglist)
{
	Resource *retval = locals.isEmpty()?
		(Resource *) NULL : (locals.last().RealObject());
	return ResStatus(ResStatus::rslOk, retval);
}


ResStatus R_List::rsl_removeTail(const ResList& arglist)
{
	Resource *retval = NULL;

	if (!locals.isEmpty())
		retval = locals.removeLast().RealObject();

	return ResStatus(ResStatus::rslOk, retval);
}

ResStatus R_List::rsl_findByValue(const ResList& arglist)
{
#ifdef DEBUG
	rslerr << "findByValue\n";
#endif

	ResReference equalTo, ref;
	unsigned methodHash;

	equalTo = arglist[1];

	// methodHash = Resource::theIDHash(method.StrValue());

	RWTValSlistIterator<ResReference> iter(locals);
	Request theReq("", arglist[0].StrValue());
	ResList args(0);
	ResContext rc("List_findByValue", 0);
	event *retev=NULL;
	Resource *compareWith=NULL;

	while(iter())
	{
		ref = iter.key();
		retev = theReq.executeResource(ref(), args, &rc);

		if (retev)
		{
#ifdef DEBUG
			rslerr << "findByValaue: got retev `";
			retev->print(rslerr);
			rslerr << "'. Comparing with `" << equalTo.StrValue() << "':";
#endif

			if ((retev->kind) & event::resArgKind)
			{
#ifdef DEBUG
				rslerr << " It's a ResArg...";
#endif

				compareWith = ((ResArg *) retev)->ref();

				if (compareWith != NULL)
				{
#ifdef DEBUG
					rslerr << " it's not NULL...";
#endif

					if (compareWith->IsEqual(equalTo()))
					{
#ifdef DEBUG
						rslerr << " Got it.\n";
#endif
						return ResStatus(ResStatus::rslOk, ref());
					}

#ifdef DEBUG
					else
						rslerr < " Nope.\n";
#endif

				}
#ifdef DEBUG
				else
					rslerr << " but it's NULL.\n";
#endif

			}
		}

#ifdef DEBUG
		else
			rslerr << "\tretev is NULL\n";
#endif

	}

	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "[]"
ResStatus R_List::OpBrack(const ResList& arglist)
{
	int loc = R_Integer::IntFromResource(arglist.get(0));
	Resource *theobj=NULL;

	if (loc >= 0 && loc < locals.entries())
	{
		ResReference &ref = locals[loc];
		theobj = ref.RealObject();
	}

	return ResStatus(ResStatus::rslOk, theobj);
}

ResStatus R_List::rsl_deleteByName(const ResList& arglist)
{
	Logf.error(LOGRSL) << "List::deleteByName() not implemented." << endline;
	cerr << "List::deleteByName() not implemented.\n";
	return ResStatus(ResStatus::rslOk, NULL);
}

ResStatus R_List::rsl_deleteByIndex(const ResList& arglist)
{
	Logf.error(LOGRSL) << "List::deleteByIndex() not implemented." << endline;
	cerr << "List::deleteByIndex() not implemented.\n";
	return ResStatus(ResStatus::rslOk, NULL);
}

ResStatus R_List::rsl_deleteByValue(const ResList& arglist)
{
	Logf.error(LOGRSL) << "List::deleteByValue() not implemented." << endline;
	cerr << "List::deleteByValue() not implemented.\n";
	return ResStatus(ResStatus::rslOk, NULL);
}

ResStatus R_List::OpEQ(const ResList& arglist)
{
	Assign(arglist.get(0));
	return ResStatus(ResStatus::rslOk, NULL);
}

ResStatus R_List::rsl_length(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_Integer::New("tempInt", locals.entries())
	);
}

ResStatus R_List::rsl_clear(const ResList& arglist)
{
	locals.clear();
	return ResStatus(ResStatus::rslOk);
}

// rsl method
// List distribute(String method);
ResStatus R_List::rsl_distribute(const ResList& arglist)
{	
	if (locals.entries() <= 0)
		return ResStatus(ResStatus::rslOk, NULL);

	ResList newargs(arglist.entries()-1);
	if (arglist.entries() > 1)
	{
		int i;
		for (i=1; i<arglist.entries(); i++)
			newargs.Add(arglist.get(i));
	}

	Request r("", arglist[0].StrValue());
//	R_List *outlist = new R_List;
	R_List *outlist = R_List::New("dist");
	RWTValSlistIterator<ResReference> iter(locals);
	ResReference ref;

	while(iter())
	{
		ref = iter.key();
		event *retev = r.executeResource(ref(), newargs, NULL, 0);
		if (!retev)
			continue;
		if (retev->isA(event::resArgKind))
			outlist->append( ((ResArg *) retev)->ref.RealObject() );
		else
		{
			Logf.error(LOGRSL)
				<< "List::distribute(): unimplemented return event- check stderr"
				<< endline;

			cerr << "List::distribute(): unimplemented return event: `";
			retev->print(cerr);
			cerr << "'\n";
		}
		delete retev;
	}
	
	return ResStatus(ResStatus::rslOk, outlist);
}

ResStatus R_List::rsl_distributeByType(const ResList& arglist)
{
	Logf.error(LOGAPPENV)
		<< "List::distributeByType() not implemented." << endline;

	return ResStatus(ResStatus::rslOk, NULL);
}

// transform
// Construct a new list of objects of the given type,
// Create each of these objects by transforming each
// of the elements of self, eg, passing it as an
// argument to the constructor of the given type.
ResStatus R_List::rsl_transform(const ResList& arglist)
{
	RWCString type = arglist[0].StrValue();
	R_List *thenewlist = NULL;
	res_class to_find(type);
	res_class *rc = ResClasses.find(&to_find);
	
	if (rc)
	{
		thenewlist = R_List::New("tempList");
		
		if (!locals.isEmpty())	// skip if empty, but still allocate new R_List.
		{
			RWTValSlistIterator<ResReference> iter(locals);
			Resource *thenewobject = NULL;
			
			while(iter())
			{
				ResList args(1);
				args.Add(iter.key().RealObject(), UNNAMED_ARG);
				
				// Create a new object of this type, and pass
				// the list element to the constructor.
				thenewobject = rc->New("", &args);
				if (thenewobject)
					thenewlist->append(thenewobject);
			}
		}
	}
	else
	{
		Logf.error(LOGAPPENV)
			<< "List::transform(): class `" << type << "' not found."
			<< endline;
	}

	return ResStatus(ResStatus::rslOk, thenewlist);
}

// RSL method "before"
//    List before(Integer index);
ResStatus R_List::rsl_before(const ResList& arglist)
{
	int index = R_Integer::Int(arglist.get(0));
	
	// check range
	if (index >= 0 && index < locals.entries())
	{
		R_List *nlist = R_List::New("");
		int i=0;
		RWTValSlistIterator<ResReference> iter(locals);
		while (iter())
		{
//			cerr << "\nconsidering `" << iter.key().StrValue() << "', pos " << i;
			if (i < index)	// before, not up-to-and-including
			{
//				cerr << ".. adding.\n";
				nlist->append(iter.key().RealObject());
			}
			else
				break;
			
			i++;
		}
		return ResStatus(ResStatus::rslOk, nlist); 
	}
	else
		Logf.error(LOGAPPENV) << "List::before(): index "
			<< index << " is out of range.\n";

	return ResStatus(ResStatus::rslOk, NULL); 
}
 
// RSL method "after"
//    List after(Integer index);
ResStatus R_List::rsl_after(const ResList& arglist)
{
	int index = R_Integer::Int(arglist.get(0));
	
	// check range
	if (index >= 0 && index < locals.entries())
	{
		R_List *nlist = R_List::New("");
		int i=0;
		RWTValSlistIterator<ResReference> iter(locals);

		for( ; iter(); i++)
		{
			if (i > index)	// after, not at-and-after
				nlist->append(iter.key().RealObject());
		}
		return ResStatus(ResStatus::rslOk, nlist); 
	}
	else
		Logf.error(LOGAPPENV) << "List::after(): index "
			<< index << " is out of range." << endline;

	return ResStatus(ResStatus::rslOk, NULL);
}
 
// RSL method "sublist"
//    List sublist(Integer start, Integer end);
ResStatus R_List::rsl_sublist(const ResList& arglist)
{
	int from = R_Integer::Int(arglist.get(0)),
		to = R_Integer::Int(arglist.get(1));
	
	// check range. compare <= 0 so that -0 or 0
	// can be used as the 2nd argument.
	if (to <= 0)
		// - 1 so that (1, -1) cuts off the first
		// and last elements, for symmetry...
		to = locals.entries() + to - 1;

	if (from >= 0 && from < locals.entries() && to >= 0
		&& to <= locals.entries())
	{
		R_List *nlist = R_List::New("");
		int i=0;
		RWTValSlistIterator<ResReference> iter(locals);

		while(iter())
		{
			if (i >= from && i <= to)
				nlist->append(iter.key().RealObject());
			i++;
		}
		return ResStatus(ResStatus::rslOk, nlist); 
	}
	else
		Logf.error(LOGAPPENV) << "List::sublist(): sublist (" << from << ", " << to
			<< ") is out of range" << endline;

    return ResStatus(ResStatus::rslOk, NULL);
}


