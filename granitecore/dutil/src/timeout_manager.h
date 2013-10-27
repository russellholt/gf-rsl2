#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include "slog.h"
#include "destiny.h"
#include "Resource.h"
#include "b.h"

#ifndef TO_MGR
#define TO_MGR

// class timeout_manager;

/** timeout manager node **/

class timeout_node {
	timeout_node *next, *prev;
	time_t last_touch;
	int consecutive_timeouts;

public:
	// enum cmp { lessthan, equal, greaterthan };
	time_t max_idle;
	RWCString session;
	RWCString message;
	ResReference obj;	// may be replaced with object name
	bool active;

	timeout_node(Resource * o, RWCString mesg="Timeout");
	timeout_node(ResReference ref, RWCString mesg="Timeout");

	// int compare(timeout_node *t, cmp how);
	int LessIdle(timeout_node *t);
	friend class timeout_manager;
	
	// methods for hashing
	int operator ==(const timeout_node& t);
	static unsigned hash(const timeout_node& t);
	
	inline void touch() { last_touch = time(0); }
};

/** timeout manager **/

class timeout_manager {
	time_t maxIdleTime;	// default, not absolute max.
	int consecutive_allowed;
	timeout_node *old_end, *young_end, *counter;
	void AddYoungNode(timeout_node *& node);
	void InsertYoungNode(timeout_node *& node);

public:
	timeout_manager(int seconds=0, int consecutive=0);
	
	inline void SetIdleTime(int seconds) { maxIdleTime = (time_t) seconds; }
	inline time_t MaxIdle(void) { return maxIdleTime; }

	inline void SetConsecutiveTO(int c) { consecutive_allowed = c; }
	inline int ConsecutiveTO(void) { return consecutive_allowed; }

	time_t GreatestIdle(void);
	time_t SecsToTimeout(void);
//	timeout_node *AddObject(Resource *r, time_t mi);
//	timeout_node *AddObject(Resource *r) { return AddObject(r, -1); }

	timeout_node *RegisterResource(Resource *r, RWCString session,
		time_t timeout, RWCString method);

	void CutNode(timeout_node *& tn);
	void Touch(timeout_node *& t);
	
	int timeoutReady();

	AuditRequest *SendTimeouts(void);
//	event *SendOneTimeout(void);

	inline int InitCounter(void) { counter = old_end; return (counter != NULL); }
	void BeGone(timeout_node *& node);
	void print(ostream& out=cout);
	void InsertAt(timeout_node *& what, timeout_node *&where);
	void InsertAfter(timeout_node *&what, timeout_node *&where);
	
	inline int isEmpty() { return (young_end == (timeout_node *) NULL
		&& old_end == (timeout_node *) NULL); }
};


#endif

