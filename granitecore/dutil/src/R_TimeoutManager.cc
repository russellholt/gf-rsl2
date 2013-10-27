// R_TimeoutManager.cc
// $Id: R_TimeoutManager.cc,v 1.1 1998/11/17 23:47:09 toddm Exp $

#include "R_TimeoutManager.h"
#include "slog.h"
#include "rsldefaults.h"
#include "b.h"
#include "destiny.h"
#include "R_Integer.h"

#define _hINIT 1231972724	// Init
#define _hREGISTER 554762779	// Register
#define _hRESETTIMER 1128471048 // ResetTimer
#define _hREMOVE 604007791  // Remove

#define DEFAULTMETHOD "defaultMethod"
#define DEFAULTTIMEOUT "defaultTimeout"

static char rcsid[] = "$Id: R_TimeoutManager.cc,v 1.1 1998/11/17 23:47:09 toddm Exp $";

#ifdef RSLERR
extern ofstream rslerr;
#endif

// R_TimeoutManager static member
rc_TimeoutManager R_TimeoutManager::rslType("TimeoutManager");


extern "C" res_class *Create_TimeoutManager_RC()
{
	return &(R_TimeoutManager::rslType);
}


// Spawn - create a new resource of this type (R_TimeoutManager)
// Called by res_class::New() if there is no object
// to pull off the free list.
Resource* rc_TimeoutManager::spawn(RWCString nm)
{
	return new R_TimeoutManager(nm);	// or other constructor
}

// New
// static convenience function
// See R_String::New(), R_Integer::New(), etc.
// If no functionality beyond res_class::New() is required
// (ie, no special values to be set conveniently), then this
// function simply eliminates the need to cast the result
// of res_class::New().
R_TimeoutManager *R_TimeoutManager::New(RWCString n)
{
	Resource *r= R_TimeoutManager::rslType.New(n);
	return (R_TimeoutManager *) r;
}

// R_TimeoutManager constructor
R_TimeoutManager::R_TimeoutManager(RWCString n)
	: ResObj(n, &rslType), nodes(timeout_node::hash, BUCKETS_FOR_SESSIONS)
{

}

// StrValue()
// Return a String version of this Resource, only if applicable.
RWCString R_TimeoutManager::StrValue(void)
{
	return "TimeoutManager";
}

// LogicalValue()
// Evaluate the "trueness" of this Resource (1 for TRUE, 0 for FALSE)
// Used in logical comparisons.
int R_TimeoutManager::LogicalValue()
{
	return (nodes.entries() > 0);
}

//// Assign
//// set this resource equal to r.
//// (ResStructure provides a default version)
//void R_TimeoutManager::Assign(Resource *r)
//{
//	/* modify this */
//}

// Clear()
// Memory management - called to restore an object
// to a "just created" state, for free-list management.
// (ResStructure provides a default version)
void R_TimeoutManager::Clear()
{
	// iterate timeout_node hash set and cut each node
	
#ifdef RSLERR
	rslerr << "R_TimeoutManager::Clear() -- not implemented\n";
#endif
}

//// print()
//// ECI syntax
//// (ResStructure provides a default version)
//void R_TimeoutManager::print(ostream &out)
//{
//
//}

// rslprint()
// Printing from within RSL
// (ResStructure provides a default version)
void R_TimeoutManager::rslprint(ostream &out)
{
	tm.print(out);
}



// FindNode()
// Given a Resource, find the corresponding timeout_node in the hash table.
// remember this is memory address comparison of the resources since
// it must be the exact resource.
timeout_node *R_TimeoutManager::FindNode(ResReference ref)
{
	timeout_node lookuptm(ref);
	return nodes.find(&lookuptm);
}

//	timeoutReady()
//	check to see if there are any pending timeout events.
//	If NOT, then look in the "removable" list for any resource pending removal.
//
//	Not clear whether this method should really be in the interior class
//	timeout_manager, since the problem is introduced by R_TimeoutManager:
//	rsl code executed for a timeout event calls Remove while the timeout
//	list is already being walked (list is munged while it's being walked).
int R_TimeoutManager::timeoutReady()
{
	int tmready = tm.timeoutReady();

	// "removable" is a list of resources to be removed from
	// the timeout manager's care.
	if (!tmready && removable.entries() > 0)
	{
		RWTValSlistIterator<ResReference> iter(removable);
		
		while (iter())
		{
			timeout_node *realtm = FindNode(iter.key());
		
			if (realtm)
			{
				Logf.info(LOGAPPENV) << "TimeoutManager: final remove for `"
					<< ((iter.key())()->Name()) << "', in session `"
					<< realtm->session << "'." << endline;

				// Remove the object from the hash table
				nodes.remove(realtm);

				// Remove the node from the timeout manager's list
				// (it will be deleted)
				tm.BeGone(realtm);
			}
			else
			{
				Logf.error(LOGAPPENV)
					<< "TimeoutManager::Remove(\""
					<< (iter.key()).Name() << "\"): object not registered." << endline;		
			}
		}
		removable.clear();
	}

	return tmready;
}

ResStatus R_TimeoutManager::execute(int method, ResList& arglist)
{
	switch(method)
	{
		case _hREGISTER:	// "Register"
			return rsl_Register(arglist);
 
        case _hRESETTIMER:   // "ResetTimer"
            return rsl_Touch(arglist);
 
        case _hREMOVE:  // "Remove"
            return rsl_Remove(arglist);

		default: ;
	}

	return ResStatus(ResStatus::rslFail);
}

// RSL method "Register"
//	Register(object, String Session);
//	Register(object, String Session, Integer timeoutSeconds, String methodName);	
ResStatus R_TimeoutManager::rsl_Register(const ResList& arglist)
{
#ifdef RSLERR
	rslerr << "R_TimeoutManager::rsl_Register()\n";
#endif

    ResReference TheObject = arglist[0];
    timeout_node *tn=NULL;
    RWCString thesession = arglist[1].StrValue();
	RWCString theTMvalue, theMethod;

	// first form:
	//	native Register(object, String Session);
	// use default method and timeout value as given in the rsl class
	// declaration.
	if (arglist.entries() < 4)
	{
#ifdef RSLERR
		rslerr << "TimeoutManager:Getting default timeout and method..\n";
#endif
		
		// Get default method and timeout value.
		RWCString defaultMethod = GetDataMember(DEFAULTMETHOD).StrValue();
		int defaultTimeout = R_Integer::Int((GetDataMember(DEFAULTTIMEOUT))());
		
		if (defaultMethod.length() <= 0)
			defaultMethod = "Timeout";

		if (defaultTimeout <= 0)
		{
			Logf.error(LOGAPPENV)
				<< "TimeoutManager: Unable to set a timeout value: none given"
				<< " and no default found, attempt to Register object `"
				<< TheObject.Name() << "'" << endline;

			return ResStatus(ResStatus::rslOk);
		}

		tn = tm.RegisterResource(TheObject(), thesession,
			defaultTimeout, defaultMethod);
	}
	else
	{
		// Second form:
		//	native Register(object, Integer timeoutSeconds, String methodName);	
		// Use given values for method and timeout.
		
		ResReference timeoutval = arglist[2], timeoutmeth = arglist[3];
		
		tn = tm.RegisterResource(TheObject(), thesession,
			(time_t) R_Integer::Int(timeoutval()), timeoutmeth.StrValue());
	}

	if (!tn)
	{
		Logf.error(LOGAPPENV)
			<< "TimeoutManager: unable to create internal node for `"
			<< TheObject.Name() << "'" << endline;
		return ResStatus(ResStatus::rslOk, NULL);
	}
	
	tn->active = TRUE;

	// add node to hash set so it can be looked up by name easily
	nodes.insert(tn);

	return ResStatus(ResStatus::rslOk, NULL);
}


 
// RSL method "Touch"
//	native Touch(object);
ResStatus R_TimeoutManager::rsl_Touch(const ResList& arglist)
{
	// find timeout node
    timeout_node *realtm = FindNode(arglist[0]);	
	
	// will find a timeoutnode that contains the exact resource (by address)
	
	if (realtm)
	{
		Logf.info(LOGAPPENV)
			<< "TimeoutManager::ResetTimer(\"" << arglist[0].Name()
			<< "\");" << endline;
		tm.Touch(realtm);
	}
	else
	{
		Logf.error(LOGAPPENV)
			<< "TimeoutManager::ResetTimer(): object `"
			<< arglist[0].Name() << "' is not registered." << endline;
	}

    return ResStatus(ResStatus::rslOk);
}

// RSL method "Remove"
//	native Remove(object);	
ResStatus R_TimeoutManager::rsl_Remove(const ResList& arglist)
{
	ResReference ref = arglist[0];

	Logf.info(LOGAPPENV) << "TimeoutManager: delayed remove for Resource `"
		<< ref()->Name() << "'." << endline;

	timeout_node *tm_node = FindNode(ref);

	if (tm_node)
	{
		tm_node->active = FALSE;
		removable.insert(ref);
	}

	return ResStatus(ResStatus::rslOk, NULL);
}
