// $Id: timeout_manager.cc,v 1.1 1998/11/17 23:47:33 toddm Exp $
// timeout_manager.cc
//
// RFH 2/21/96
// Copyright 1996 Destiny Software Corporation

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <rw/cstring.h>

#include "timeout_manager.h"
#include "destiny.h"
#include "slog.h"
#include "b.h"
#include "rslEvents.h"

//#include "runtime.h"

#ifdef RSLERR
extern ofstream rslerr;
#endif

extern "C" {
int getpid(void);
}

timeout_node::timeout_node(Resource * o, RWCString mesg)
{
	next = prev = NULL;
	obj.Set("", o);
	consecutive_timeouts=0;
	last_touch = time(0);
	max_idle = -1;
	message = mesg;
}

timeout_node::timeout_node(ResReference ref, RWCString mesg)
{
	next = prev = NULL;
	obj = ref;
	consecutive_timeouts=0;
	last_touch = time(0);
	max_idle = -1;
	message = mesg;
}

// node equality
// equality by address of resource
int timeout_node::operator ==(const timeout_node& t)
{
	return (obj.RealObject() == t.obj.RealObject());
}

// hash code
// use address of Resource since it's got to be the
// exact object
unsigned timeout_node::hash(const timeout_node& t)
{
	return (unsigned) (t.obj.RealObject());
}

// LessIdle
// Less idle time is greater than or equal remaining time...
int timeout_node::LessIdle(timeout_node *t)
{
	if (!t)
		return 0;
#ifdef DEBUG
	obj->print();
	cout << " >= ";
	t->obj->print();
#endif

time_t tnow = time(0);
time_t l, r;

	l = max_idle - (tnow - last_touch);
	r = t->max_idle - (tnow - t->last_touch);

#ifdef DEBUG
	if (l>=r)
		cout << ": true ...";
	else
		cout << ": false ...";
#endif

	return (l >= r);

}


// constructor
// argument is the max idle time for which a timeout
// message is sent.
timeout_manager::timeout_manager(int seconds, int consecutive)
{
	maxIdleTime = (time_t) seconds;
	consecutive_allowed = consecutive;
	young_end = old_end = counter = NULL;
}


// AddObject
// Put a resource* in the list. Creates and returns
// a new list node, which the caller should keep track
// of to use in future calls (such as Touch). However,
// timeout_manger is responsible for its destruction.
// 
timeout_node *timeout_manager::RegisterResource(Resource *r, RWCString session,
	time_t timeout, RWCString method)
{
#ifdef RSLERR
	rslerr << "timeout_manager::RegisterResource(): ";
#endif

	if (!r)
	{
		return NULL;
	}

#ifdef RSLERR
	rslerr << "session " << session << ", timeout in " << timeout
		<< " seconds, method: `" << method << "'\n";
#endif
	
	timeout_node *tn = new timeout_node(r, method);
	tn->max_idle = timeout;
	tn->session = session;

	InsertYoungNode(tn);

	return tn;

}

// AddYoungNode
// Add a node to the list at the young end, and set
// its last touch time to now.
void timeout_manager::AddYoungNode(timeout_node *& node)
{
	if (!node)
		return;


	// no nodes
	if (young_end == NULL && old_end == NULL)
	{
		old_end = young_end = node;
		return;
	}
	
	young_end->next = node;	// attach node to the end
	node->prev = young_end;
	node->next = NULL;	// it's at the end
	young_end = node;	// reset the end pointer
	
	if (!old_end)
		old_end = young_end;
}

// InsertYoungNode
// Gotta search the list & find out where this node fits in
// based on normalized idle times (ratio comparison).
void timeout_manager::InsertYoungNode(timeout_node *& node)
{
	if (!node)
		return;

	if (!young_end)
	{
		AddYoungNode(node);
		return;
	}

timeout_node *p = young_end, *p_prev = NULL;
int w=0;

	while(p)
	{
		p_prev = p->prev;	// save pointer due to re-ordering potential
		w = node->LessIdle(p);
		if (w)
		{
			InsertAfter(node, p);
			return;
		}
		p = p_prev;	// get older
	}
	
	InsertAt(node, old_end);
}


// InsertAt
// Put `what' before `where' in the list. "Before," meaning toward the
// old end of the list.
//
// -- to add a node to the end (young side) of the list,
//    use AddYoungNode().

void timeout_manager::InsertAt(timeout_node *& what, timeout_node *&where)
{
	if (!what || !where)
		return;
#ifdef DEBUG
	cout << "Insert ";
	what->obj->print();
	cout << " before ";
	where->obj->print();
	cout << '\n';
#endif

	if (where->prev)
		where->prev->next = what;

	what->next = where;
	what->prev = where->prev;	// may be NULL: ok.
	where->prev = what;	// back point

	if (old_end == where)
		old_end = what;

}

void timeout_manager::InsertAfter(timeout_node *&what, timeout_node *&where)
{
	if (!what || !where)
		return;
	
#ifdef DEBUG
	cout << "Insert ";
	what->obj->print();
	cout << " after ";
	where->obj->print();
	cout << '\n';
#endif

	if (where->next)
		where->next->prev = what;

	what->prev = where;
	what->next = where->next;	// may be NULL: ok.
	where->next = what;

	if (young_end == where)
		young_end = what;
}

// GreatestIdle
// the idle time of the oldest item.
time_t timeout_manager::GreatestIdle(void)
{
	if (old_end)
		return time(0) - old_end->last_touch;
	
	return 0;
}

// SecsToTimeout
// number of seconds until the first timeout in the list occurs
time_t timeout_manager::SecsToTimeout(void)
{
	if (old_end)
		return old_end->max_idle - GreatestIdle();
	
	return maxIdleTime;
}

int timeout_manager::timeoutReady()
{
	return (old_end && (old_end->max_idle - GreatestIdle()) <= 0);
}

// Touch
// Call to inform the manager that the object has been
// "touched" - ie, zero idle time
void timeout_manager::Touch(timeout_node *& node)
{
	if (!node)
		return;

	node->consecutive_timeouts = 0;
//	node->last_touch = time(0);	// make young
	node->touch();
	
	// simple case: one node
	if (young_end == node)
		return;	// nothing to do

	CutNode(node);
	// AddYoungNode(node);	// add to end
	InsertYoungNode(node);
}

// CutNode
// remove the given node, but don't delete it.
void timeout_manager::CutNode(timeout_node *& node)
{

	if (node->prev)
	{
		node->prev->next = node->next;
		if (node->next == NULL)
			young_end = node->prev;
	}

	if (node->next)
	{
		node->next->prev = node->prev;
		if (node->prev == NULL)
			old_end = node->next;
	}

	node->next = NULL;
	node->prev = NULL;

	if (young_end == node && old_end == node)	// last node
		young_end = old_end = NULL;
}

// SendTimeouts
// Currently only sends one timeout event and returns one result
// which may be null.
//
////////// Look through the list and send timeout events to each node
////////// whose idle time is greater than the maximum.
////////// Since the list is sorted, stop when the first element is
////////// encountered that has low idle time (< max)
AuditRequest *timeout_manager::SendTimeouts(void)
{
	// quick check.
	if (!old_end)
		return NULL;

#ifdef DEBUG
	cout << form("[%d]: SendTimeouts: greatestIdle is %d\n", getpid(),
		GreatestIdle());
	print();
#endif

	timeout_node *temp = old_end, *temp_next=NULL;
	time_t tnow = time(0), tdiff;
//	elist *resultlist = NULL;

	AuditRequest *arq =NULL;
	event *requ_results	=NULL;

	while(temp)
	{
		if (!temp)
			break;


		tdiff = tnow - temp->last_touch;

#ifdef DEBUG
		cout << "TOMgr: " << tdiff << " of " << temp->max_idle << " seconds: ";
#endif
		if (tdiff < temp->max_idle)
		{
#ifdef DEBUG
			cout << "idle time under maximum.\n";
#endif

			return arq;
		}

		// Check to see if this node is active. If not, just move it to the end of 
		// the list. An inactive node is one which should not be sent timeout events,
		// ie, it may be pending removal when safe.
		if (!temp->active)
		{
			logf->debug(LOGAPPENV) << "timeout manager -- skipping inactive node" << endline;
			temp_next = temp->next;	// save pointer
			Touch(temp);	// move inactive node to the end of the list
			temp = temp_next;	// continue with saved pointer
			continue;
		}

        // ***************************************************
        // * Check to see if this is a session level timeout *
        // *                                                 *
        // * MAY TAKE THIS CHECK OUT WHEN AUDITREQUEST IS    *
        // * FIXED TO SEND TO A OBJECT NAME INSTEAD OF A     *
        // * OBJECT TYPE.                                    *
        // ***************************************************
        if (temp->session == temp->obj.Name())
        {
    		// ************************
    		// * Send the timeout event
    		// ************************
    		logf->notice(LOGAPPENV) << "TimeoutManager: sending event `"
    			<< temp->message << "' to Resource `"
    			<< temp->obj.ClassName() << "' in session "
    			<< temp->session << endline;


    		arq = new AuditRequest (
    			(char *) (temp->obj.ClassName().data()),
    			(char *) (temp->session.data()),
    			(char *) (temp->message.data()),
    			(char *) "_timeout_" );
        }
        else
        {
    		// ************************
    		// * Send the timeout event
    		// ************************
    		logf->notice(LOGAPPENV) << "TimeoutManager: sending event `"
    			<< temp->message << "' to Resource `"
    			<< temp->obj.Name() << "' in session "
    			<< temp->session << endline;


    		arq = new AuditRequest (
    			(char *) (temp->obj.Name().data()),
    			(char *) (temp->session.data()),
    			(char *) (temp->message.data()),
    			(char *) "_timeout_" );
        }

		requ_results = arq->execute(NULL);
		
		if (requ_results)
		{
#ifdef RSLERR
			rslerr << "TM: got result back: `";
			requ_results->print(rslerr);
			rslerr << "'\n";
#endif
		}

//		// create a return elist if it doesn't exist yet
//		if (requ_results && resultlist==NULL)
//			resultlist = new elist;

		if (requ_results)
		{
#ifdef RSLERR
			rslerr << "-- adding returned event to outgoing list, which is now:\n";
#endif
			// we don't actually return the results themselves. they
			// become the "arguments" in the ECI response which mirrors
			// the incoming request.
			if (requ_results->isA(event::elistArgKind))
			{
#ifdef RSLERR
				rslerr << "This is a elistArgKind\n";
#endif
				arq->SetArgs((elistArg *) requ_results);
				requ_results = NULL;
//				return arq;
//				resultlist->add(arq);
			}
//			else
//			{
//				rslerr << "Not a list arg..\n";
//				resultlist->add(requ_results);
//			}

		}
		else
		{
			// if there were no results returned, we don't want (or do we?)
			// to send a message to the client.
			delete arq;
			arq = NULL;
		}

		
		// Save next pointer (may rearrange the list via Touch() if it's timed out)
		temp_next = temp->next;

		/* Check consecutive timeouts

		 * consecutive timeout functionality reintroduced 3/21/96
		 * to handle the "spin-cycle" problem when an object seems to
		 * ignore the timeout event.
		 *
		 * Re-thinking the timeout philosophy has led me to believe that one
		 * timeout event should be sent, and then it should be reset - the
		 * destination object shouldn't be repsonsible for resetting; it can
		 * ignore the event in which case another will not be sent (or only
		 * the number specified by the consecutive allowed timeout value.
		 */

		temp->consecutive_timeouts++;
		if (temp->obj.isValid())
		{			
			logf->debug(LOGAPPENV)
				<< "TM: object " << temp->obj.StrValue()
				<< ": timeout #" << temp->consecutive_timeouts << "..."
				<< endline;

			if (temp->consecutive_timeouts >= consecutive_allowed)
			{
				Logf.info(LOGAPPENV)
					<< "TimeoutManager: Auto ResetTimer() on object `"
					<< temp->obj.StrValue() << "'" << endline;
				Touch(temp);	// reset idle time (moves to the young end of the list)
				temp->consecutive_timeouts = 0;	// reset.
			}
		}
		else	// no obj -- touch anyway?
			Touch(temp);

		// Advance with saved pointer.
		//temp = temp_next;

		return arq;
	}
	
//	if (resultlist)
//	{
//		rslerr << "TM: returning elist\n\t";
//		resultlist->print(rslerr);
//		rslerr << endl;
//	}
//	else
//		rslerr << "TM: no events to return.\n";
//
//	return resultlist;


	/*-------------------*/

//	if (arq)
		return arq;
//	else
//		return requ_results;

}

//// SendOneTimeout
//// Same as above, but incremental. Slower.
//event *timeout_manager::SendOneTimeout(void)
//{
//	if (!counter)
//		counter = old_end;
//	if (!counter)	// still not...
//		return 0;
//
//	// Save the next pointer in case timeout nodes
//	// are re-arranged as a result of a timeout handler
//	timeout_node *counter_next = counter->next;
//
//	time_t tnow = time(0), tdiff;
////	SLList<resource *> no_args;
////	String tos = "Timeout";
//	ResList arglist(0);
//	Resource *r=NULL;
//
//	tdiff = tnow - counter->last_touch;
//
//	/*
//	cout << "TOMgr: " << tdiff << " of " << maxIdleTime << " seconds: ";
//	*/
//	if (tdiff < maxIdleTime)
//	{
//		/*
//		cout << counter->obj->Value() << ": Idle time under maximum.\n";
//		*/
//		return 0;
//	}
//
//	counter->consecutive_timeouts++;
//
//#ifdef DEBUG
//	cout << " -- Timeout event for object ";
//	if (counter->obj)
//		cout << counter->obj->Value() << '\n';
//#endif
//	
//	unsigned int mid = Resource::theIDHash(counter->message);
//	ResStatus stat = counter->obj->execute(mid, arglist);
//
//	// counter = counter->next;
//	counter = counter_next;	// use the saved pointer
//
//	if (counter)
//		return 1;
//	
//	return 0;
//}



// BeGone
// Informs the manager that the object in the node will
// soon go away. This is where all nodes are destroyed.
void timeout_manager::BeGone(timeout_node *& node)
{
	if (node)
	{
		CutNode(node);
		delete node;
		node = NULL;
	}
}


// print
// a test routine
void timeout_manager::print(ostream& out)
{
	out << "Old: ";
	timeout_node *temp = NULL;
	time_t tdiff, tnow=time(0);
	for(temp = old_end; temp; temp=temp->next)
	{
		temp->obj.print(out);
		tdiff = tnow - temp->last_touch;
		out << '(' << tdiff << '/' << (temp->max_idle) << ')';
	}
	out << '\n';
}
