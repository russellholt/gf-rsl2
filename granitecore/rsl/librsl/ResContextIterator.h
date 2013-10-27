#include "Resource.h"

#ifndef _RESCITERATOR_H_
#define _RESCITERATOR_H_


class ResContextIterator : public ResIterator {
  protected:
	RWTValHashSetIterator<ResReference>& iter;

  public:
	ResContextIterator(RWTValHashSet<ResReference>& table) : iter(table) { }
	int hasMoreElements() { return (int) (iter()); }
	ResReference nextElement() { return iter.key(); }
};

#endif

