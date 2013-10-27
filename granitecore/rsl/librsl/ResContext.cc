#include <rw/tpslist.h>
#include <fstream.h>

#include "Resource.h"
#include "R_String.h"
#include "R_Integer.h"
#include "rsldefaults.h"



extern ofstream rslerr;


// declared in runtime.cc
extern  int nResContextAdds, nFreelistAdds, nFreelistRemoves;


// ****************************************************************
// * ResContext
// ****************************************************************

ResContext::ResContext(const char *nm, size_t nlocals)
	: contextName(nm) 
{
	if (nlocals > 0)
		locals = new RWTValHashSet<ResReference>(ResReference::hash, nlocals);
	else
		locals = NULL;
		
	contexts = NULL;
	contextOwner = NULL;
}

ResContext::~ResContext()
{
#ifdef RSLERR
	rslerr << "ResContext::~ResContext() for `" << contextName << "'\n" << flush;
#endif

	if (locals)
	{
		// delete all the value ResReferences and thus decrement the
		// refcounts on the resources, and in some cases, delete them.
		delete locals;
	}

	if (contexts)
		delete contexts;	// don't delete the ResContexts themselves!
}

// Owner
// If this context has a contextOwner, return it.
// Otherwise, search the list of contexts and call Owner on
// each one until an owner is found. Otherwise, return NULL.
Resource *ResContext::Owner(void)
{
	if (contextOwner)
		return contextOwner;


	RWTPtrSlistIterator<ResContext> iter(*contexts);
	ResContext *rct=NULL;
	Resource *rown=NULL;
	
	while (iter())
	{
		rct = iter.key();
		rown = rct->Owner();
		if (rown)
			return rown;
	}

	return NULL;
}

void ResContext::AddResource(Resource *r, access_t vis)
{
#ifdef DEBUG_REFCOUNT
// Keep track of these invocations for detailed log tracking.
// the logfile may get messy so assigning a unique number allows
// the boundaries to be seen easier, especially if there are ever
// nested calls.
static int count=0;
#endif

	if (!locals)
		locals = new RWTValHashSet<ResReference>(ResReference::hash, BUCKETS_IN_NAMESPACE);

	if (r)
	{

#ifdef DEBUG_REFCOUNT
		rslerr << "\nRCX> AddResource#" << count
			<< ": declaring local ref (N,R)\n";
#endif

		ResReference ref(r->Name(), r, vis);
////		locals->insert(ResReference(r));

	//	r->DecReference();

#ifdef DEBUG_REFCOUNT
		rslerr << "RCX> AddResource#" << count
			<< ": inserting local ref in table\n";
#endif

		locals->insert(ref);

#ifdef DEBUG_REFCOUNT
		rslerr << "RCX> AddResource#" << count
			<< ": done insert.\n\n";

	// increment the counter for the next time.
	count++;	// local static

#endif
	
	nResContextAdds++;	// runtime.cc

	}
}

// ResContext::AddReferenceTo()
// Same as AddResource(), but allows the use of a new name.
void ResContext::AddReferenceTo(RWCString newname, Resource *r, access_t vis)
{
#ifdef DEBUG_REFCOUNT
static int count=0;
#endif

	if (!locals)
		locals = new RWTValHashSet<ResReference>(ResReference::hash, BUCKETS_IN_NAMESPACE);

	if (r)
	{

#ifdef DEBUG_REFCOUNT
		rslerr << "\nRCX> AddRefTo#" << count
			<< ": declaring local ref (N,R)\n";
#endif

		ResReference ref(newname, r, vis);

	//	r->DecReference();

#ifdef DEBUG_REFCOUNT
		rslerr << "RCX> AddRefTo#" << count
			<< ": inserting local ref in table\n";
#endif

		locals->insert(ref);

#ifdef DEBUG_REFCOUNT
		rslerr << "RCX> AddResource#" << count
			<< ": done insert.\n\n";

	// increment the counter for the next time.
	count++;	// local static

#endif

		nResContextAdds++;	// runtime.cc
	}
}

// ResContext::AddResListContents()
// Add the ResReferences (not the real resources)
void ResContext::AddResListContents(ResList& rl)
{
register int i, len = rl.entries();

#ifdef DEBUG_REFCOUNT
static int count=0;
#endif

	if (locals)
	{
		for (i=0; i<len; i++)
		{
#ifdef DEBUG_REFCOUNT
		rslerr << "RCX> AddResListContents#" << count
			<< ": inserting ref from ResList into table\n";
#endif

			locals->insert( *( (ResReference *) (rl.getref(i)) ) );

#ifdef DEBUG_REFCOUNT
		rslerr << "RCX> AddResource#" << count
			<< ": done insert.\n\n";

	// increment the counter for the next time.
	count++;	// local static

#endif
			nResContextAdds++;	// runtime.cc
		}
	}

#ifdef RSLERR
	else
		rslerr << "ResContext::AddResListContents: null local context for `"
			 << contextName << "'\n";
#endif

}

void ResContext::AddContext(ResContext *c)
{
	if (!contexts)
		contexts = new RWTPtrSlist<ResContext>;
	contexts->insert(c);
}

void ResContext::PrependContext(ResContext *c)
{
	if (!contexts)
		contexts = new RWTPtrSlist<ResContext>;
	contexts->prepend(c);
}

void ResContext::RemoveContext(ResContext *c)  
{
	if (contexts)
		contexts->remove(c); 
}

void ResContext::RemoveResource(RWCString nm)
{
	if (locals)
	{
		ResReference ref(nm, NULL);
		locals->remove(ref);
	}
	
}

void ResContext::ResizeLocalSpace(int n)
{
	if (!locals)
		locals = new RWTValHashSet<ResReference>(ResReference::hash, n);
	else
		locals->resize(n);
}
		

/** GetLocals() is OBSOLETE */
RWTValHashSet<ResReference> *ResContext::GetLocals(void)
{
	return locals;
}


// ****************
// ResContext::Find
// Find the named resource. Look in locals first, then other contexts.
// *********************************************************************
ResStatus ResContext::Find(RWCString what, Resource *replaceWith)
{
ResReference lookRef(what), ref;
ResStatus stat;
RWBoolean found = FALSE;

#ifdef RSLERR
	rslerr << "ResContext::Find() ";
	rslerr << " - Looking for `" << what << "' in context `" << contextName
		<< "'\n";
//	print(rslerr);
#endif

	// **********************
	// * `self' special case.
	// **********************
	if (what=="self")
	{
#ifdef RSLERR
		rslerr << "ResContext::Find -- request for SELF found\n";
#endif

		Resource *cown = Owner();
		if (cown)
			return ResStatus(ResStatus::rslOk, cown);

#ifdef RSLERR
		rslerr << "\tno context owner..\n";
#endif

		return ResStatus(ResStatus::rslFail);
	}


	if (locals)
		found = locals->find(lookRef, ref);

	// if not found in locals (or locals doesn't exist), then search other contexts.
	if (found == FALSE)
	{
		if (contexts)
		{
			// Search through the list of other contexts in scope order-
			// inner to outer scope - linearity and order is important.
			RWTPtrSlistIterator<ResContext> iter(*contexts);
			ResContext *resco=NULL;
			ResStatus innerstat;
			
			while (++iter == TRUE)
			{
				resco = iter.key();
				if (resco && resco != this)	// watch for infinite loops!
				{
					innerstat = resco->Find(what, replaceWith);
					if (innerstat.status == ResStatus::rslOk)	// found
						return innerstat;
				}
#ifdef RSLERR
				else
					rslerr << "\t..NULL ResContext in ResContext::contexts ...\n";
#endif
			}
		}

#ifdef RSLERR
		rslerr << "  -- failed (`" << what << "' not found).\n";
#endif

		stat.status = ResStatus::rslFail;
	}
	else
	{

#ifdef RSLERR
		rslerr << "  -- found in context `" << contextName << "'.\n";
#endif

		if (replaceWith != NULL)
		{
#ifdef RSLERR
			rslerr << "\treplacing with `";
			replaceWith->print(rslerr);
			rslerr << "'\n";
#endif

			Replace(ref, replaceWith);
		}

		stat.r = ref.RealObject();
	}

	return stat;
}

// Replace
// Remove the old, add the new.
// If `what' is not actually in the table, the new one
// will be added anyway, with the same name as the "old" one.
void ResContext::Replace(ResReference& what, Resource* with)
{
	if (!locals)
		return;

	locals->remove(what);
	AddReferenceTo(what.Name(), with);
}

//	ResContext::RemoveContext
//	Removes a named context.
void ResContext::RemoveContext(const char *nm)
{
	if (!contexts)
		return;

	ResContext test(nm, 0);
	contexts->remove(&test);
}
	
unsigned ResContext::hash(const ResContext& rc)
{
	//return Resource::hash((rc.Name()).data());
	return Resource::theIDHash((rc.Name()).data());
}

// Equality by name only.
int ResContext::operator==(const ResContext& rc)
{
	return (contextName == rc.Name());
}

void ResContext::print(ostream& out)
{
	if (locals)
	{
		RWTValHashTableIterator<ResReference> iter(*locals);
		out << "ResContext `" << contextName << "' contains " << locals->entries()
			<< " Resources:\n";

		while (++iter == TRUE)
		{
			// Assignment to ResReference will decrement the refcount on
			// the existing Resource in the ResReference, assign the new
			// name and Resource, and increment its refcount.
			ResReference resref = iter.key();
			out << '\t' << resref.Name() << ": " << resref()->RefCount() << endl;
		}
		// destruction of resref will decrement its refcount.
	}

#ifdef RSLERR
	else
		rslerr << "ResContext::print() -- no locals\n";
#endif

}

// ResContext::printContextInfo
// Prints information about the context: name and number of resources.
// Rescurses for all subcontexts with indentation (wow!)
void ResContext::printContextInfo(ostream& out, RWCString indent)
{
	out << indent << "Context `" << contextName << "': ";
	if (locals)
		out << locals->entries() << " local resources. "
			<< (contexts? (contexts->entries()) : 0) << " super-contexts.\n";

	// print enclosing contexts (recurse)
	if (contexts)
	{
		RWTPtrSlistIterator<ResContext> iter(*contexts);
		ResContext *rc=NULL;
		indent += "   ";
		while(++iter == TRUE)
		{
			rc = iter.key();
			if (rc && rc != this)	// watch for infinity
				rc->printContextInfo(out, indent);
		}
	}
}


void ResContext::Clear(void)
{
	contextName="";

	if (contexts)
		delete contexts;	// doesn't kill the actual context objects
	contexts = NULL;

	// Clear the local context, but don't delete it. This means we're
	// getting rid of all the ResReference objects, which should
	// delete the Resources (in most cases), but when this object is
	// pulled off the free list (as it is when Clear() is called!)
	// we'll still need a local table of the same size that it was
	// when it was originally created.
	if (locals)
		locals->clear();

}

class ResContextIterator : public ResIterator {
  protected:
	RWTValHashSetIterator<ResReference>& iter;

  public:
	ResContextIterator(RWTValHashSet<ResReference>& table) : iter(table) { }
	int hasMoreElements() { return (int) (iter()); }
	ResReference nextElement() { return iter.key(); }
};

ResIterator *ResContext::elements()
{
	if (locals)
		return new ResContextIterator(*locals);

	return new ResIterator();
}


