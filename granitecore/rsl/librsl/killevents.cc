// $Id: killevents.cc,v 1.4 1999/01/12 14:50:20 toddm Exp $
#include "b.h"
#include <rw/tpslist.h>

RWTPtrSlist<event> KillerEvents;

event *Remember(event *e)
{
	if (e && !e->isA(event::programCodeKind))
		KillerEvents.append(e);

	return e;
}
