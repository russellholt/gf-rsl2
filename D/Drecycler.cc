#include "Drecycler.h"

#include <iostream.h>

static char rcsid[] = "$Id: Drecycler.cc,v 1.1 1998/11/12 18:31:42 holtrf Exp $";

// ************************
// * DR_recycler
// ************************

#define _hGET 6776180
#define _hRECYCLE 285804153

#ifdef PURIFY
#include "purify.h"
#endif

DR_recycler::DR_recycler(D *d) : DR_Object(d)
{

}

DR_recycler::DR_recycler(const DRef& ref) : DR_Object(ref)
{

}

// If you redefine dump() or Recycle(), you must
// call dump() from this destructor because DRef::dump(), invoked
// from DRef::~DRef(), may call Recycle() - and destructors do not
// call functions in derived classes. otherwise, don't call it.
DR_recycler::~DR_recycler()
{
#ifdef DMEMORY
	cerr << "~DR_recycler()\n";
#endif
}

DO_recycler* DR_recycler::safe_get()
{
	SAFE_GET(DO_recycler);	// D_macros.h
}

// rarely used.
// _set() when safe_get() is used avoids double checking.
DO_recycler* DR_recycler::safe_set(D* d)
{
	SAFE_SET(DO_recycler,d);	// D_macros.h
}

// DR_recycler::Recycle()
// This should only be called by the destructor for DR_recyler,
// which usually means that a DR_recycler was declared as it is
// in the definition of DO_Class.
//
// We're overriding the normal behavior of DRef::Recycle(),
// which finds a class object (DO_Class) in which to store
// unused objects. Because we don't care about storing unused
// recycler objects, we'll delete it.
void DR_recycler::Recycle()
{
#ifdef PURIFY
	purify_printf("DR_recycler::Recycle().. deleting the DO_ ...\n");
#endif

	D * x = unsafe_get();
	if (x)
	{
		// let it clean up.
		x->_destroy();

		// delete the object
		delete (unsafe_get());	// DRef::unsafe_get()
	}

	// clear the pointer for safety's sake
	_unsafe_clear();	// DRef::_unsafe_clear()
}

DO_recycler *DR_recycler::New()
{
	// Should recyclers be recycled?
	// Should a recycler have a DO_Class of its own?
	// The object created here is deleted by the
	// DRef::dump() case when it can't find a class object
	// to do recycling.

	return safe_set(new DO_recycler());
}

// ************************
// * DO_recycler
// ************************

// C++ constructor
// use init() for individual object initialization
DO_recycler::DO_recycler()
{
	freeList = new free_list();
}

// C++ destructor
// use destroy() for individual object de-construction
DO_recycler::~DO_recycler()
{
#ifdef DMEMORY
	cerr << "~DO_recycler(): destroying free list for `" << classname << "'.\n";
#endif
#ifdef PURIFY
	purify_printf("~DO_recycler(): destroying free list `%s'.", classname.data());
#endif

	if (!freeList->isEmpty())
		freeList->clearAndDestroy();

	delete freeList;
}


// init(): the "constructor"
void DO_recycler::init()
{
	// initialize superclass first
	DO_Object::init();

// DO_recycler is not a DO_Composite (yet?)
//
//	add("classname", classname.New());


}

// destroy(): the "destructor"
void DO_recycler::destroy()
{
#ifdef PURIFY
	purify_printf("DO_recycler::destroy(). freeList has %d.\n",
		(int) (freeList->entries()) );
#endif

	// destroy superclass last
	DO_Object::destroy();
}

DR_String DO_recycler::toString()
{
	return DR_String("recycler toString() not implemented");
}

// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
DRef DO_recycler::route(DR_Message m)
{
// it is not likely at this point that a recycler
// would be available through a messaging interface
// like this. Not yet, at least, because it can
// lead to an "infinite regression" in DRef de-referencing.

/*
	switch(m->messageCode())
	{

		default: ;
	}
*/
	return DO_Object::route(m);
}

// **********************************************************
//	get()
//		gets an object from the free list.
//
//	If the free list is empty, we'll return zero
//	and the user is expected to create one.
//	Generally, this is called from DO_Class::New() who
//	will call DO_Class::spawn() in the latter case, which
//	is subclassed for every type and the only place that
//	the C++ new operator is used to allocate memory.
//
//	It is the user's (eg DO_Class) responsibility to invoke
//	_init()	on the object this function returns.
//
// **********************************************************
D* DO_recycler::get(  )
{
#ifdef DMEMORY
	cerr << "\t\tDO_recycler::get() for `" << classname << "'.\n";
#endif

	if (!freeList->isEmpty())
		return freeList->get(); // removes & returns the first element.

	return DNULL;
}


/**
	recycle
	 puts an object on the free list or destroys it 

	In RSL: void recycle( Object o);
*/
void DO_recycler::recycle(D *o)
{
#ifdef DMEMORY
	cerr << "\t\tDO_recycler::recycle() for `" << classname << "'-- FREELISTING.\n";
#endif

	if (!o)
	{
#ifdef DMEMORY
		cerr << "\t\t\tnull in DO_recycler::recycle() for `" << classname << "': ignoring.\n";
#endif
		return;
	}

	freeList->add(o);

#ifdef DMEMORY
	cerr << "\t\t\tfreelist for `" << classname
		<< "' has length " << freeList->entries() << " (post append).\n";
#endif

}


void DO_recycler::setClassName(RWCString clname)
{
	classname = clname;
}
