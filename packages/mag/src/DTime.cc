#include "DTime.h"
#include <stdio.h>

static char rcsid[] = "$Id: DTime.cc,v 1.3 1998/12/02 02:05:14 mbridges Exp $";


#define _hOpLT 60   // <
#define _hOpGT 62   // >
#define _hOpLE 15421    // <=
#define _hOpGE 15933    // >=
#define _hOpNE 8509 // !=
#define _hOpEQ2 15677   // ==

#define _hOpASSIGN 14909    // :=


// ************************
// * DR_Time
// ************************


DR_Time::DR_Time(D *d) : DR_Magnitude(d)
{
}


DR_Time::DR_Time(DR_String str): DR_Magnitude(0)
	{
	RWDBDateTime tmp(RWDate(), str.val());
	if (tmp.isValid())
		{
		safe_get()->rwdt = tmp;
		}
	}
	
DR_Time::DR_Time(const DRef& ref) : DR_Magnitude(ref)
{
}

// If you redefine dump() or Recycle(), you must
// call dump() from this destructor because DRef::dump(), invoked
// from DRef::~DRef(), may call Recycle() - and destructors do not
// call functions in derived classes. otherwise, don't call it.
DR_Time::~DR_Time()
{

}

// const_get()
// get the object as the correct type, but without the potential
// to modify it if it is null.
DO_Time* DR_Time::const_get() const
{
	return dynamic_cast<DO_Time *> (unsafe_get());
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
DO_Time* DR_Time::safe_get()
{
	SAFE_GET(DO_Time);	// defined in D_macros.h
}

// rarely used.
// replace() when safe_get() will be used avoids double checking.
DO_Time* DR_Time::safe_set(D* d)
{
	SAFE_SET(DO_Time,d);	// defined in D_macros.h
}

DO_Time *DR_Time::New()
{
	DO_Time *dobj = DO_Time::New().const_get();
	replace(dobj);

	return dobj;
}

// ************************
// * DO_Time
// ************************

// C++ constructor
// use init() for individual object initialization
DO_Time::DO_Time()
{
}

// C++ destructor
// use destroy() for individual object de-construction
DO_Time::~DO_Time()
{
}

// init(): the "constructor"
void DO_Time::init()
{
	// initialize superclass first
	DO_Magnitude::init();

	rwdt = RWDBDateTime();

}

// destroy(): the "destructor"
void DO_Time::destroy()
{

	// destroy superclass last
	DO_Magnitude::destroy();
}

DR_String DO_Time::toString()
{
	char buf[15];
	int h = rwdt.hour();
	int m = rwdt.minute();
	char *aorp = "a.m.";
	if ( h > 12)
		{
		h = h-12;
		aorp = "p.m.";
		}
	sprintf(buf,"%d:%02u%s", h, m, aorp);
	return DR_String(buf);
}

// ********************************************************

// DOC_Time: the Time class class
// By defining this class in the .cc file, it is
// unavailable anywhere else. This is good, for now.
class DOC_Time : public DO_Class {
	D *spawn();

  public:
	DOC_Time() : DO_Class("Time") { }

};

DR_Class DO_Time::Timeclass = new DOC_Time();
//DR_Class DO_Time::Timeclass;

// The only place DO_Time objects should be created.
// This function is private; it is called from
// DO_Class::New().
D *DOC_Time::spawn() {
	return new DO_Time();
}

DRef DO_Time::Class()
{
	return Timeclass;
}

// New()
// Create a new DO_Time by asking for one from
// the static class object, DO_Time::Timeclass.
// That object will either create one, by calling the virtual
// function spawn(), that is, DOC_Time::spawn(),
// or getting one from its list of free objects.
//
// A static function
DR_Time DO_Time::New()
{
	return DO_Time::Timeclass->New();
}

// Create_DOC_Time()
// not currently in use as of Oct 1, 1998
// It will be used as the "bootstrap" function
// to create the class object, which will in turn
// create and recycle instances of DO_Time
extern "C" DRef& Create_DOC_Time()
{
	DO_Time::Timeclass = new DOC_Time();
	return DO_Time::Timeclass;
}


// ********************************************************



// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
DRef DO_Time::route(DR_Message m)
{
    // until the semantics of route() are nailed down,
	// its body is commented out in the template text.
	// for the moment it simply bounces the message
	// to its superclass.

	switch(theIDHash(m->message->data()))
	{
        case _hOpLT: return opLT(DR_Time(m("op")));
        case _hOpGT: return opGT(DR_Time(m("op")));
        case _hOpLE: return opLE(DR_Time(m("op")));
        case _hOpGE: return opGE(DR_Time(m("op")));
        case _hOpNE: return opNE(DR_Time(m("op")));
        case _hOpEQ2: return opEQ2(DR_Time(m("op")));

        case _hOpASSIGN: return opAssign(m("op"));

		default: ;
	}
	return DO_Magnitude::route(m);
}


DR_Time DO_Time::opAssign(const DRef& ref)
	{
	DRef tmp = ref;
	if (tmp == DR_null) 
		return DR_Time(this);
	D *dobj = tmp.unsafe_get();
	if (!dobj)
		return DR_Time(this);

	DO_Time *t = dynamic_cast<DO_Time *>(dobj);
	if (t) 
		{
		rwdt = t->rwdt;
		return DR_Time(this);
		}

	DO_String *s = dynamic_cast<DO_String *>(dobj);
	if (s)
		{
		RWDBDateTime dt(RWDate(), s->data());;
		if (!dt.isValid())
			return DR_Time(this);
		rwdt = dt;
		return DR_Time(this);
		}

	return DR_Time(this);
	}

DR_Bool DO_Time::opLT(const DR_Time &d)
	{
	DR_Time tmp = d;
	return DR_Bool(rwdt < tmp->rwdt);
	}

DR_Bool DO_Time::opLE(const DR_Time &d)
	{
	DR_Time tmp = d;
	return DR_Bool(rwdt <= tmp->rwdt);
	}

DR_Bool DO_Time::opGT(const DR_Time &d)
	{
	DR_Time tmp = d;
	return DR_Bool(rwdt > tmp->rwdt);
	}

DR_Bool DO_Time::opGE(const DR_Time &d)
	{
	DR_Time tmp = d;
	return DR_Bool(rwdt >= tmp->rwdt);
	}


DR_Bool DO_Time::opEQ2(const DR_Time &d)
	{
	DR_Time tmp = d;
	return DR_Bool(rwdt == tmp->rwdt);
	}


DR_Bool DO_Time::opNE(const DR_Time &d)
	{
	DR_Time tmp = d;
	return DR_Bool(rwdt != tmp->rwdt);
	}


DR_Magnitude DO_Time::Assign( const DR_Magnitude& n )
{
	if (n.isValid())
	{
		// instead of simply saying replace(n.unsafe_get())
		// we really want to assign our DO_Int  to the
		// DO_Int inside of n because this object may be
		// pointed to by many

		DO_Time * doi = DR_Time(n).const_get();

		if (doi)
			rwdt = doi->rwdt;
	}

	return this;
}


DR_Time::DR_Time(RWDBDateTime indt)
	{
	safe_get()->rwdt = indt;
	}


dcompare_t DO_Time::compare(const DRef& d) const
	{
	if (!d.isValid())
		return c_greater;
	

	const DR_Time &drt= dynamic_cast<const DR_Time &>(d);

	DR_Time tmp = d;

	if (rwdt < tmp->rwdt)
		return c_less;
	if (rwdt > tmp->rwdt)
		return c_greater;
	return c_equal;
	}



