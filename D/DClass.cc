#include "DClass.h"

#include <iostream.h>

static char rcsid[] = "$Id: DClass.cc,v 1.2 1998/11/20 18:15:43 holtrf Exp $";

// ************************
// * DR_Class
// ************************

#define _hNEW 5137783
#define _hRECYCLE 822675065


#ifdef PURIFY
#include "purify.h"
#endif

DR_Class::DR_Class(DO_Class *doc) : DR_Object(doc)
{

}

DR_Class::DR_Class(const DRef& ref) : DR_Object(ref)
{

}

// If you redefine dump() or Recycle(), you must
// call dump() from this destructor because DRef::dump(), invoked
// from DRef::~DRef(), may call Recycle() - and destructors do not
// call functions in derived classes. otherwise, don't call it.
DR_Class::~DR_Class()
{
#ifdef DMEMORY
	cerr << "~DR_Class(): calling dump()\n";
#endif
	dump();
}

DO_Class *DR_Class::safe_get()
{
	SAFE_GET(DO_Class);
}

DO_Class *DR_Class::const_get() const
{
	return dynamic_cast<DO_Class *> (unsafe_get());
}

DO_Class *DR_Class::safe_set(D *d)
{
	SAFE_SET(DO_Class, d);
}

// This New() means
// a new DO_Class object, *NOT* a new object
// of the class defined by this class!
DO_Class *DR_Class::New()
{
	cerr << "DR_Class::New() - create a new DO_Class ?? not implemented.\n";
	// TEMPORARY!
//	return safe_set(new DO_Class("noclass"));
	return (DO_Class *)0;
}

void DR_Class::dump(D* d, ref_t t)
{
#ifdef DMEMORY
	cerr << "DR_Class::dump() - calling DRef::dump()\n";
#endif
	DRef::dump(d, t);
}

void DR_Class::Recycle()
{
#ifdef DMEMORY
	cerr << "==DR_Class::Recycle() - deleting!\n";
#endif

#ifdef PURIFY
	purify_printf("DR_Class::Recycle() - deleting.\n");
#endif


	D* x = unsafe_get();
	if (x)
	{
		x->_destroy();

//#ifndef D_KEEP_INUSE
		delete x;
//#endif
		_unsafe_clear();
	}

}

// ************************
// * DO_Class
// ************************


// C++ constructor
// use init() for individual object initialization
DO_Class::DO_Class(const char *name)
{
	classname = name;
	classID = theIDHash(name);

	recycler.New();
	recycler->setClassName(classname);
}

// C++ destructor
// use destroy() for individual object de-construction
DO_Class::~DO_Class()
{
#ifdef DMEMORY
	cerr << "~DO_Class() for `" << classname << "'.";
#endif

#ifdef PURIFY
		purify_printf("~DO_Class()...\n");
#endif

}


// init(): the "constructor"
void DO_Class::init()
{
#ifdef DMEMORY
	cerr << "\tDO_Class::init()\n";
#endif

	// initialize superclass first
	DO_Object::init();


// recycler is not a composite (yet?)
//	add("classname", classname.New());
//	add("recycler", recycler.New());

}

// destroy(): the "destructor"
void DO_Class::destroy()
{
#ifdef DMEMORY
	cerr << "\tDO_Class::destroy()\n";
#endif

#ifdef D_KEEP_INUSE

#ifdef PURIFY
		purify_printf("DO_Class::destroy() `%s'; inUse has %d: dumping into freelist.\n",
			classname.data(), (int) (h_inUse.entries()) );
#endif

	// transfer all in-use objects to the recycler
	// ** THIS IS DANGEROUS IF THE RECYCLER IS NOT USING
	// A COLLECTION THAT ENFORCES UNIQUENESS, such as
	// free_hashset from freed.h
	// This is done so that objects that are in use but are not
	// on the free list will be cleaned up as well.

/*
	while(!p_inUse.isEmpty())
		recycler->recycle(p_inUse.get());
*/

	h_inUse.clear();	// forget these pointers!

#endif D_KEEP_INUSE

#ifdef PURIFY
	purify_printf("DO_Class::destroy(): Dumping freelist `%s'.\n", classname.data());
#endif

	recycler.dump(); // interesting!

	// destroy superclass last
	DO_Object::destroy();
}

DR_String DO_Class::toString()
{
	return DR_String("Class toString() not implemented");
}

// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
DRef DO_Class::route(DR_Message m)
{
    // until the semantics of route() are nailed down,
	// its body is commented out in the template text.
	// for the moment it simply bounces the message
	// to its superclass.

	switch(m->messageCode)
	{
		case _hNEW: // RSL: Object New( );
			return New(  );

		default: ;
	}
	return DO_Object::route(m);
}



// this new means create a new DO
D* DO_Class::New()
{
#ifdef DMEMORY
	cerr << "DO_Class::New() for `" << classname << "' (using recycler)\n";
#endif

	D* d = recycler->get();

	if (!d)
	{
		d = spawn();

#ifdef D_KEEP_INUSE
#ifdef DMEMORY
	cerr << "DO_Class::New() for `" << classname << "': remembering all in-use objects.\n";
#endif
		//p_inUse.append(d);
		h_inUse.add(d);
#endif
	}

	if (!d)
	{
#ifdef DMEMORY
	cerr << "DO_Class::New() for `" << classname << "': can't create object.\n";
#endif
		return DNULL;
	}

	d->_init();
	
	// Note, this invokes DRef(D*) and makes a new reference
	return d;
}

/**
	Recycle
	In RSL: void Recycle( Object o);
*/
void DO_Class::Recycle(D* d)
{
#ifdef DMEMORY
	cerr << "DOC_" << classname << "::Recycle()--  ";
#endif

#ifdef PURIFY
		purify_printf("DO_Class::Recycle() actually recycles.\n");
#endif

	if (d)
	{
#ifdef DMEMORY
		cerr << "invoking _destroy() and passing to the recycler..\n";
#endif

#ifdef D_KEEP_INUSE
		// move from in-use list to recyler
		h_inUse.remove(d);
		// we could examine the return value.. or not..
#endif

		d->_destroy();
		recycler->recycle(d);
	}
#ifdef DMEMORY
	else
		cerr << "object is null, ignoring.\n";
#endif

	return;
}


D* DO_Class::spawn()
{
	cerr << "DO_Class::spawn() does nothing, but may eventually create a composite by default.\n";
	return DNULL;
}

RWCString DO_Class::className()
{
	return classname;
}

// ***************************************************************

// DOC_Class: DO_Class's "class", the guy who recycles
// DO_Class objects. But since we actually don't *yet* care
// about recycling for class objects, we just override Recycle()
// to do a delete.
class DOC_Class : public DO_Class {
	D *spawn();

  public:
	DOC_Class() : DO_Class("Class") { }

	D *New() {
		// This version of New() does not keep track of in-use objects
		// or use a recycler.

		D* d = spawn();

		// this is not an else because spawn() will create a D,
		// and if it doesn't, that's an error!
		if (d)
			d->_init();

		return d;
	}

	void Recycle(D* d)
	{
#ifdef PURIFY
		purify_printf("DOC_Class::Recycle() does a delete.\n");
#endif

#ifdef DMEMORY
		cerr << "DOC_Class::Recycle() calling _destroy(), and deleting!\n";
#endif
		if (d)
			d->_destroy();

		delete d;
	}

	DRef Class() {
		return this;	// an "instance" of itself?
	}
};

DR_Class DO_Class::Classclass = new DOC_Class();

D *DOC_Class::spawn()
{
#ifdef DMEMORY
	cerr << "DOC_Class::spawn()\n";
#endif
	return new DO_Class("");
}

DRef DO_Class::Class()
{
	return DO_Class::Classclass;		
}


