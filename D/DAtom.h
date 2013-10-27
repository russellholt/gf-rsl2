#include "D.h"

#ifndef _D_ATOM
#define _D_ATOM

class DO_Atom : public DO_Object {
  public:

	// toString() is abstract

	inline void init() { DO_Object::init(); }
	inline void destroy() { DO_Object::destroy(); }
	
};


#endif
