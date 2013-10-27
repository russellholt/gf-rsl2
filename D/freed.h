// free managers
//	for maintaining collections of free D objects
//		(not free blocks or bytes or pages or anything else..)
//
// The two typical classes used are defined here:
//	free_list is a linked list, RWTPtrSlist
//	free_hashset is a hash set by memory address, RWTPtrHashSet
// 
// $ID$

#include "D.h"

#include <rw/tphset.h>

#ifndef D_freeD_H
#define D_freeD_H

// freeManager, an abstract class
// for the most part, the interface follows Rogue Wave conventions.
class freeManager {
public:
	freeManager();
	virtual ~freeManager();

	virtual void add(D *d)=0;
	virtual D* get()=0;
	virtual int isEmpty()=0;
	virtual void clear()=0;
	virtual void clearAndDestroy()=0;
	virtual size_t entries()=0;
};

class free_list : public freeManager {
	RWTPtrSlist<D> inUse;
public:

	free_list();
	virtual ~free_list();

	void add(D *d);
	D* get();
	inline int isEmpty() { return inUse.isEmpty(); }
	inline void clear() { inUse.clear(); }
	inline void clearAndDestroy() { inUse.clearAndDestroy(); }
	size_t entries();

};

// the class free_hashset should probably be removed..
class free_hashset : public freeManager {
	RWTPtrHashSet<D> inUse;
public:

	free_hashset();
	virtual ~free_hashset();

	void add(D *d);
	D* get();
	D* remove(D* x);
	inline int isEmpty() { return inUse.isEmpty(); }
	inline void clear() { inUse.clear(); }
	inline void clearAndDestroy() { inUse.clearAndDestroy(); }
	size_t entries();

	static unsigned D_PtrHash(const D& d);
};

#endif

