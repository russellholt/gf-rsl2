
#include "rslEvents.h"

#ifndef _VALDECL_H_
#define _VALDECL_H_

class valdecl : public event {
protected:

  RWCString ClassName, ObjectName;
  event *expression;
  
public:
	valdecl(RWCString cl, RWCString obj, event *expr);
	event *execute(ResContext *context);
	void print(ostream& out=cout);
};

#endif
