#include "D.h"

#ifndef DO_OBJECT_DESTINY
#define DO_OBJECT_DESTINY

class DO_Object : public D {
  protected:
	virtual ~DO_Object();

	DO_Object() { }
  public:

	inline void init() { }
	inline void destroy() { }

	DRef route(DR_Message m);

	virtual DRef Class();
};


#endif
