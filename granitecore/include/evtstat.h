// evtstat.h
// Return status object for event calls.
// $Id: evtstat.h,v 1.1 1998/11/18 00:01:14 toddm Exp $

#ifndef _EVENTSTATUS_H_
#define _EVENTSTATUS_H_

class event;

class EventStatus {
public:
	enum { evtUndefined=0, evtOk=1, evtFail, evtFound, evtNotFound };
	int status;
	event *evt;
	
	EventStatus(int stat, event *e=NULL) { status=stat, evt=e; }
};

#endif



