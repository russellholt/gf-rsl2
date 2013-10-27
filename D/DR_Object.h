#include "D.h"

#ifndef DR_Object_dref
#define DR_Object_dref

class DR_Object : public DRef {
  public:
	DR_Object(D* d=0) : DRef(d) {  }
	DR_Object(const DRef& dref) : DRef(dref) {  }
	
	virtual ~DR_Object();

	DO_Object *safe_get();
	DO_Object *safe_set(D *d);
	inline  DO_Object *operator->() { return safe_get(); }
/* 	DO_Object *New(); */
};

#endif
