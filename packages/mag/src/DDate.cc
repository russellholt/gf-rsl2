#include "DDate.h"
#include <stdio.h>

static char rcsid[] = "$Id: DDate.cc,v 1.4 1999/01/06 23:05:22 mbridges Exp $";

// ************************
// * DR_Date
// ************************

#define _hOpAdd 43 // +
#define _hOpSubt 45 // -

#define _hOpLT 60   // <
#define _hOpGT 62   // >
#define _hOpLE 15421    // <=
#define _hOpGE 15933    // >=
#define _hOpNE 8509 // !=
#define _hOpEQ2 15677   // ==

#define _hOpASSIGN 14909    // :=
#define _hOpPE 11069    // +=
#define _hOpME 11581    // -=




DR_Date::DR_Date(D *d) : DR_Magnitude(d)
{
}

DR_Date::DR_Date(const DRef& ref) : DR_Magnitude(ref)
{

}


// If you redefine dump() or Recycle(), you must
// call dump() from this destructor because DRef::dump(), invoked
// from DRef::~DRef(), may call Recycle() - and destructors do not
// call functions in derived classes. otherwise, don't call it.
DR_Date::~DR_Date()
{

}

// const_get()
// get the object as the correct type, but without the potential
// to modify it if it is null.
DO_Date* DR_Date::const_get() const
{
	return dynamic_cast<DO_Date *> (unsafe_get());
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
DO_Date* DR_Date::safe_get()
{
	SAFE_GET(DO_Date);	// defined in D_macros.h
}

// rarely used.
// replace() when safe_get() will be used avoids double checking.
DO_Date* DR_Date::safe_set(D* d)
{
	SAFE_SET(DO_Date,d);	// defined in D_macros.h
}

DO_Date *DR_Date::New()
{
	DO_Date *dobj = DO_Date::New().const_get();
	replace(dobj);

	return dobj;
}

// ************************
// * DO_Date
// ************************

// C++ constructor
// use init() for individual object initialization
DO_Date::DO_Date():rwdt(RWDate())
{

}

// C++ destructor
// use destroy() for individual object de-construction
DO_Date::~DO_Date()
{
}

// init(): the "constructor"
void DO_Date::init()
{
	// initialize superclass first
	DO_Magnitude::init();

	rwdt = RWDate();

}

// destroy(): the "destructor"
void DO_Date::destroy()
{

	// destroy superclass last
	DO_Magnitude::destroy();
}

DR_String DO_Date::toString()
{
	char buf[15];
	int y = rwdt.year();
	y %= 100;
	int m = rwdt.month();
	int d = rwdt.dayOfMonth();
	sprintf(buf,"%d/%d/%02d", m, d, y);
	return DR_String(buf);
}

// ********************************************************

// DOC_Date: the Date class class
// By defining this class in the .cc file, it is
// unavailable anywhere else. This is good, for now.
class DOC_Date : public DO_Class {
	D *spawn();

  public:
	DOC_Date() : DO_Class("Date") { }

};

DR_Class DO_Date::Dateclass = new DOC_Date();
//DR_Class DO_Date::Dateclass;

// The only place DO_Date objects should be created.
// This function is private; it is called from
// DO_Class::New().
D *DOC_Date::spawn() {
	return new DO_Date();
}

DRef DO_Date::Class()
{
	return Dateclass;
}

// New()
// Create a new DO_Date by asking for one from
// the static class object, DO_Date::Dateclass.
// That object will either create one, by calling the virtual
// function spawn(), that is, DOC_Date::spawn(),
// or getting one from its list of free objects.
//
// A static function
DR_Date DO_Date::New()
{
	return DO_Date::Dateclass->New();
}

// Create_DOC_Date()
// not currently in use as of Oct 1, 1998
// It will be used as the "bootstrap" function
// to create the class object, which will in turn
// create and recycle instances of DO_Date
extern "C" DRef& Create_DOC_Date()
{
	DO_Date::Dateclass = new DOC_Date();
	return DO_Date::Dateclass;
}


// ********************************************************



// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
DRef DO_Date::route(DR_Message m)
{
    // until the semantics of route() are nailed down,
	// its body is commented out in the template text.
	// for the moment it simply bounces the message
	// to its superclass.

	switch(theIDHash(m->message->data()))
	{
        case _hOpSubt: return opSubt(m("op"));
        case _hOpAdd: return opAdd(DR_Int(m("op")));

        case _hOpLT: return opLT(m("op"));
        case _hOpGT: return opGT(m("op"));
        case _hOpLE: return opLE(m("op"));
        case _hOpGE: return opGE(m("op"));
        case _hOpNE: return opNE(m("op"));
        case _hOpEQ2: return opEQ2(m("op"));

        case _hOpASSIGN: return opAssign(m("op"));
        case _hOpPE: return opPE(DR_Int(m("op")));
        case _hOpME: return opME(DR_Int(m("op")));

		default: ;
	}
	return DO_Magnitude::route(m);
}


static DR_Date toDate(DRef &ref)
	{
	if (ref == DR_null)
		return DR_null;

	D *dobj = ref.unsafe_get();

	DO_Date *d = dynamic_cast<DO_Date *>(dobj);
	if (d) 
		return DR_Date(d);

	DO_String *s = dynamic_cast<DO_String *>(dobj);
	if (s)
		{
		RWDate dt(s->data());
		if (!dt.isValid())
			{
			cerr << "DDate/toDate, Couldn't convert string"  << endl;
			return DR_null;
			}
		return DR_Date(RWDBDateTime(dt));
		}
	cerr << "DDate/toDate, No Match"  << endl;
	return DR_null;
	}




DRef DO_Date::opSubt(const DRef &ref)
	{
	DRef tmp = ref;  // Avoid const problems
	if (tmp == DR_null)
		{
		return DR_null;
		}

	D *dobj = tmp.unsafe_get();
	DO_Int *i = dynamic_cast<DO_Int *>(dobj);
	if (i) 
		{
//		cerr << "int: " << i->i() << endl;
		return DR_Date(RWDate(rwdt.rwdate().julian() - i->i()));
		}
	DO_Date *d = dynamic_cast<DO_Date *>(dobj);
	if (d) 
		{
//		cerr << "int: " << i->i() << endl;
		return DR_Int(rwdt.rwdate().julian() - (d->rwdt).rwdate().julian());
		}
	return DR_null;
	}


DR_Date DO_Date::opAdd(DR_Int &ir)
	{
	int i = ir->i();
	return DR_Date(RWDate(rwdt.rwdate().julian() + i));
	}



DR_Bool DO_Date::opLT(DRef &ref)
	{
	DR_Date d = toDate(ref);
	if (d.DRef::operator==( DR_null))
		return DR_Bool(0);

	return DR_Bool(rwdt < d->rwdt);
	}

DR_Bool DO_Date::opLE( DRef &ref)
	{
	DR_Date d = toDate(ref);
	if (d.DRef::operator==( DR_null))
		return DR_Bool(0);
	return DR_Bool(rwdt <= d->rwdt);
	}

DR_Bool DO_Date::opGT( DRef &ref)
	{
	DR_Date d = toDate(ref);
	if (d.DRef::operator==( DR_null))
		return DR_Bool(0);
	return DR_Bool(rwdt > d->rwdt);
	}

DR_Bool DO_Date::opGE( DRef &ref)
	{
	DR_Date d = toDate(ref);
	if (d.DRef::operator==( DR_null))
		return DR_Bool(0);
	return DR_Bool(rwdt >= d->rwdt);
	}


DR_Bool DO_Date::opEQ2( DRef &ref)
	{
	DR_Date d = toDate(ref);
	if (d.DRef::operator==( DR_null))
		return DR_Bool(0);
	return DR_Bool(rwdt == d->rwdt);
	}


DR_Bool DO_Date::opNE( DRef &ref)
	{
	DR_Date d = toDate(ref);
	if (d.DRef::operator==( DR_null))
		return DR_Bool(0);
	return DR_Bool(rwdt != d->rwdt);
	}

DR_Date DO_Date::opAssign(DRef &ref)
	{
	DR_Date d = toDate(ref);
	if (d.DRef::operator==( DR_null))
		return DR_null;

	rwdt = d->rwdt;
	return DR_Date(this);
	}



DR_Date DO_Date::opPE(const DR_Int &i)
	{
	DR_Int tmp = i;
	rwdt.addDays(tmp->i());
	return DR_Date(this);
	}


DR_Date DO_Date::opME(const DR_Int &i)
	{
	DR_Int tmp = i;
	rwdt.addDays(-(tmp->i()));
	return DR_Date(this);
	}


DR_Magnitude DO_Date::Assign( const DR_Magnitude& n )
{
	if (n.isValid())
	{
		// instead of simply saying replace(n.unsafe_get())
		// we really want to assign our DO_Int  to the
		// DO_Int inside of n because this object may be
		// pointed to by many

		DO_Date * doi = DR_Date(n).const_get();

		if (doi)
			rwdt = doi->rwdt;
	}

	return this;
}


DR_Date::DR_Date(DR_String s)
	{
	RWDate rwd(s.val());
	if (rwd.isValid())
		{
		RWDBDateTime rwdt(rwd);
		safe_get()->rwdt = rwdt;
		}
	}
DR_Date::DR_Date(RWCString s)
	{
	RWDate rwd(s);
	if (rwd.isValid())
		{
		RWDBDateTime rwdt(rwd);
		safe_get()->rwdt = rwdt;
		}
	}

DR_Date::DR_Date(char *s)
	{
	RWDate rwd(s);
	if (rwd.isValid())
		{
		RWDBDateTime rwdt(rwd);
		safe_get()->rwdt = rwdt;
		}
	}


DR_Date::DR_Date(unsigned day, unsigned month, unsigned year)
	{
	if (year > 1000 && (month>=1 && month <= 12) && (day>= 1 && day <=31))
		{
		RWDate rwd(day, month, year);
		if (rwd.isValid())
			{
			RWDBDateTime rwdt(rwd);
			safe_get()->rwdt = rwdt;
			}
		}
	}


dcompare_t DO_Date::compare(const DRef& d) const
	{
	if (!d.isValid())
		return c_greater;
	

	const DR_Date &drt= dynamic_cast<const DR_Date &>(d);

	DR_Date tmp = d;

	if (rwdt < tmp->rwdt)
		return c_less;
	if (rwdt > tmp->rwdt)
		return c_greater;
	return c_equal;
	}


