#include "DInt.h"
#include <iostream.h>
#include <stdio.h>

static char rcsid[] = "$Id: DInt.cc,v 1.2 1998/11/24 14:55:32 holtrf Exp $";

// ************************
// * DR_Int
// ************************

#define _hADD 4285540


DR_Int::DR_Int(D *d) : DR_Magnitude(d)
{
#ifdef DMEMORY
	cerr << "DR_Int(D*)\n";
#endif

}

DR_Int::DR_Int(const DRef& ref) : DR_Magnitude(ref)
{
#ifdef DMEMORY
	cerr << "DR_Int(DRef&)\n";
#endif
}

DR_Int::DR_Int(int n) : DR_Magnitude((D*) 0)
{
#ifdef DMEMORY
	cerr << "DR_Int(int)\n";
#endif

	New();
	i() = n;
}

// If you redefine dump() or Recycle(), you must
// call dump() from this destructor because DRef::dump(), invoked
// from DRef::~DRef(), may call Recycle() - and destructors do not
// call functions in derived classes. otherwise, don't call it.
DR_Int::~DR_Int()
{
#ifdef DMEMORY
	cerr << "~DR_Int()\n";
#endif

}

// const_get()
// get the object as the correct type, but without the potential
// to modify it if it is null.
DO_Int* DR_Int::const_get() const
{
	return dynamic_cast<DO_Int *> (unsafe_get());
}

// safe_get()
// this method is how operator->() is implemented; 
// the purpose is to get the object, correctly typed,
// in a way that can be directly dereferenced witout checking.
// Because it is possible for the object to be null, safe_get()
// will create it by calling New(). This is the correct behavior
// 99% of the time. If it is not, use either const_get() or
// look in D_macros.h to find the actual code. It's possible that
// throwing an exception is more appropriate in some cases instead
// of creating a new object.
//
// Note that this is not a virtual function.
DO_Int* DR_Int::safe_get()
{
	SAFE_GET(DO_Int);	// defined in D_macros.h
}

// rarely used.
// replace() when safe_get() will be used avoids double checking.
DO_Int* DR_Int::safe_set(D* d)
{
	SAFE_SET(DO_Int,d);	// defined in D_macros.h
}

DO_Int *DR_Int::New()
{
#ifdef DMEMORY
	cerr << "DR_Int::New()\n";
#endif

	DO_Int *dobj = DO_Int::New().const_get();
	replace(dobj);

	return dobj;
}

// ************************
// * DO_Int
// ************************

// C++ constructor
// use init() for individual object initialization
DO_Int::DO_Int()
{
#ifdef DMEMORY
	cerr << "DO_Int()\n";
#endif
	val = 0;
}

// C++ destructor
// use destroy() for individual object de-construction
DO_Int::~DO_Int()
{
#ifdef DMEMORY
	cerr << "~DO_Int()\n";
#endif

}

// init(): the "constructor"
void DO_Int::init()
{
#ifdef DMEMORY
	cerr << "DO_Int::init()\n";
#endif

	// initialize superclass first
	DO_Magnitude::init();
	val = 0;
}

// destroy(): the "destructor"
void DO_Int::destroy()
{
#ifdef DMEMORY
	cerr << "DO_Int::destroy()\n";
#endif

	val = 0;
	// destroy superclass last
	DO_Magnitude::destroy();
}

BOOLEAN DO_Int::toBoolean() const
{
	return (val > 0);	
}

DR_String DO_Int::toString()
{
	char c[20];
	sprintf(c, "%d", val);
	return DR_String((const char *) c);
}

// ********************************************************

// DOC_Int: the Int class class
// By defining this class in the .cc file, it is
// unavailable anywhere else. This is good, for now.
class DOC_Int : public DO_Class {
	D *spawn();

  public:
	DOC_Int() : DO_Class("Int") { }

};

DR_Class DO_Int::Intclass = new DOC_Int();
//DR_Class DO_Int::Intclass;

// The only place DO_Int objects should be created.
// This function is private; it is called from
// DO_Class::New().
D *DOC_Int::spawn() {
#ifdef DMEMORY
	cerr << "DOC_Int::spawn()\n";
#endif
	return new DO_Int();
}

DR_Class DO_Int::DClass()
{
	return Intclass;
}

// New()
// Create a new DO_Int by asking for one from
// the static class object, DO_Int::Intclass.
// That object will either create one, by calling the virtual
// function spawn(), that is, DOC_Int::spawn(),
// or getting one from its list of free objects.
//
// A static function
DR_Int DO_Int::New()
{
#ifdef DMEMORY
	cerr << "DO_Int::New()\n";
#endif
	return DO_Int::Intclass->New();
}

// Create_DOC_Int()
// not currently in use as of Oct 1, 1998
// It will be used as the "bootstrap" function
// to create the class object, which will in turn
// create and recycle instances of DO_Int
extern "C" DRef& Create_DOC_Int()
{
	DO_Int::Intclass = new DOC_Int();
	return DO_Int::Intclass;
}


// ********************************************************



// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
DRef DO_Int::route(DR_Message m)
{
#ifdef DMEMORY
	cerr << "Int::route() for `" << m->message << "'\n";
#endif

	switch(theIDHash(m->message->data()))
	{
		case _hADD: // RSL: Int Add( Integer n);
			return Add( DR_Int(m("n")) );

		default: ;
	}
	return DO_Magnitude::route(m);
}

void DO_Int::assign(const DRef& obj)
{
	val = DR_Int(obj).i();
}

dcompare_t DO_Int::compare(const DRef& d) const
{
	int& x = DR_Int(d)->i();
	return val < x ? c_less : (val == x ? c_equal : c_greater );
}

/**
	Add
	In RSL: Int Add( Integer n);
*/
DR_Int DO_Int::Add( const DR_Int& n )
{
	return DR_null;
}


/* 	DR_Int& operator=(const DR_Magnitude& n); */

DR_Int& DR_Int::operator+=(const DR_Int& n)
{
	if (n.isValid())
	{	
		// const_get may return zero, even if
		// n.isValid() is true.
		DO_Int *doi = n.const_get();
		if (doi)
			i() += doi->i();
	}
	
	return *this;
}

/* 	DR_Int& operator+=(const DR_Magnitude& n); */

DR_Int& DR_Int::operator-=(const DR_Int& n)
{
	if (n.isValid())
	{
		// const_get may return zero, even if
		// n.isValid() is true.
		DO_Int *doi = n.const_get();
		if (doi)
			i() -= doi->i();
	}
	
	return *this;
}

DR_Magnitude DO_Int::Assign( const DR_Magnitude& n )
{
	if (n.isValid())
	{
		// instead of simply saying replace(n.unsafe_get())
		// we really want to assign our DO_Int  to the
		// DO_Int inside of n because this object may be
		// pointed to by many

		DO_Int * doi = DR_Int(n).const_get();

		if (doi)
			val = doi->i();
	}

	return this;
}

// DR_Int::i_val()
// return a copy of the int inside the DO_Int,
// or *zero* if there isn't one.
//
// This is correct since zero is the default initialized value.
// Use of this object, such as adding a value to it (+=),
// will create a DO_Int (via safe_get()), which is zero by default,
// and the operation would proceed unnoticed.
int DR_Int::i_val() const
{
	DO_Int *doi = const_get();
	return doi ? doi->i_val() : 0;
}


// ***********************
// comparisons & operators
// ***********************


int operator==(const DR_Int& left, int right)
{
	return left.i_val() == right;
}

int operator==(const DR_Int& left, const DR_Int& right)
{
	return left.i_val() == right.i_val();
}

int operator!=(const DR_Int& left, int right)
{
	return left.i_val() != right;
}

int operator!=(const DR_Int& left, const DR_Int& right)
{
	return left.i_val() != right.i_val();
}
