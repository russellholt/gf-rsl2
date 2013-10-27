// StatMessages.h
// $Id: StatMessages.h,v 1.1 1998/11/17 23:48:38 toddm Exp $
// Jan 17, 1997 -- changed GNU to Rogue Wave libraries
#ifndef _STATMESSAGES_H_
#define _STATMESSAGES_H_

#include <rw/cstring.h>
#include <rw/tvhdict.h>

#define STATUS_UNSET_TAG "*"

class statusMessage {
public:
	enum { statusUnset = -1 };
	int severity;
	RWCString message, tag;
	statusMessage(void)
	{
		severity = statusUnset;
		message = tag = STATUS_UNSET_TAG;
	}
};

class StatusMessageMap {
	RWTValHashDictionary<int, statusMessage> dict;

public:

	StatusMessageMap(int thesize=RWDEFAULT_CAPACITY);
	~StatusMessageMap();

	void AddMessage(int key, int severity, RWCString tag, RWCString msg);
	statusMessage Find(int num);
	RWCString operator[](int num);
	void LoadMessages(const char *filename);

	static unsigned hash(const int& v);
};

#endif
