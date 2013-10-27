// iteration.h
// RSL iteration: foreach, ..
// $Id: iteration.h,v 1.1 1998/11/17 23:55:48 toddm Exp $
// *******************
// * System Includes *
// *******************
// ******************
// * Local Includes *
// ******************
#include "rslEvents.h"

#ifndef _RSLITERATION_H_
#define _RSLITERATION_H_

class foreach : public event {
	event *expression, *body;
	RWCString iterID;

public:
	enum loop_t { cContinue, cBreak, cReturn };

	foreach(RWCString id, event *exp, event *bod);
	virtual ~foreach();
	event *execute(ResContext *context);
	void print(ostream& out=cout);
	
private:

	loop_t innerLoop(Resource *iterRes,
		ResContext& iterContext, event *& resultevt);
};
#endif



