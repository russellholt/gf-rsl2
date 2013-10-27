#include <stdio.h>
#include <ctype.h>
#include <typeinfo.h>
#include <iostream.h>
#include "DDecimal.h"
#include "DInt.h"

static char rcsid[] = "$Id: DDecimal.cc,v 1.5 1999/01/06 23:05:24 mbridges Exp $";

// ************************
// * DR_Decimal
// ************************

#define _hASMONEYFORMAT 342120776
#define _hASPERCENTAGEFORMAT 1080696835    
#define _hOpAdd 43  // +
#define _hOpSubt 45 // -
#define _hOpMult 42 // *
#define _hOpDiv 47  // /
#define _hOpLT 60   // <
#define _hOpGT 62   // >
#define _hOpLE 15421    // <=
#define _hOpGE 15933    // >=
#define _hOpNE 8509 // !=
#define _hOpEQ2 15677   // ==
#define _hOpASSIGN 14909    // :=
#define _hOpEQ 61   // =    (OBSOLETE)
#define _hOpPE 11069    // +=
#define _hOpME 11581    // -=
#define _hOpTE 10813    // *=
#define _hOpDE 12093    // /=

static RWDecimalPortable zeroDP(0);

RWDecimalBase::RoundingMethod roundingMethod = RWDecimalBase::BANKERS;
static RWDecimalPortable toRWDP(double d)
	{
	char buf[40];
	sprintf(buf,"%1.6f",d);
	return RWDecimalPortable(buf);
	}



DR_Decimal::DR_Decimal(D *d) : DR_Magnitude(d)
{

}

DR_Decimal::DR_Decimal(const DRef& ref) : DR_Magnitude(ref)
{

}

// If you redefine dump() or Recycle(), you must
// call dump() from this destructor because DRef::dump(), invoked
// from DRef::~DRef(), may call Recycle() - and destructors do not
// call functions in derived classes. otherwise, don't call it.
DR_Decimal::~DR_Decimal()
{

}



static RWCString stripStrange(const char *s)
// Called whenever a decimal is explicilty or implicitly created 
// in RSL.  We could just call it when processing user input.
// RWDecimalPortable has problems with '$' and spaces.
	{
	char buf[80];
	int length = strlen(s);
	int i=0, j=0;
	while (i < length)
		{
		char c = s[i++];
		if ( (c != ' ') && (c != '$'))
			buf[j++] = c;
		}
	buf[j]=0;
	return RWCString(buf);

	}

// const_get()
// get the object as the correct type, but without the potential
// to modify it if it is null.
DO_Decimal* DR_Decimal::const_get() const
{
	return dynamic_cast<DO_Decimal *> (unsafe_get());
}

static DR_Decimal toDecimal(const DRef &ref)
	{
//	cerr << "In toDecimalA" << endl;
	DRef tmp = ref;  // Avoid const problems
	if (tmp == DR_null)
		{
		return DR_null;
		}

	D *dobj = tmp.unsafe_get();

	DO_Decimal *d = dynamic_cast<DO_Decimal *>(dobj);
	if (d) 
		{
//		cerr << "DR_DEcimal: " << (*d).toString() << endl;
		return DR_Decimal(d);
		}
	DO_Int *i = dynamic_cast<DO_Int *>(dobj);
	if (i) 
		{
//		cerr << "int: " << i->i() << endl;
		return DR_Decimal(RWDecimalPortable(i->i()));
		}
	DO_String *s = dynamic_cast<DO_String *>(dobj);
	if (s)
		{
		RWCString tmp = stripStrange(s->data());

		RWDecimalPortable dec(tmp);
		if (dec.isNumber())
			{
			return DR_Decimal(dec);
//			cerr << "String: " << tmp << endl;

			}
		cerr << "DDecimal/toDecimal String, No Match"  << endl;
		return DR_null;
		}
	cerr << "DDecimal/toDecimal, No Match"  << endl;
	return DR_null;
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
DO_Decimal* DR_Decimal::safe_get()
{
	SAFE_GET(DO_Decimal);	// defined in D_macros.h
}



// rarely used.
// replace() when safe_get() will be used avoids double checking.
DO_Decimal* DR_Decimal::safe_set(D* d)
{
	SAFE_SET(DO_Decimal,d);	// defined in D_macros.h
}

DO_Decimal *DR_Decimal::New()
{
	DO_Decimal *dobj = DO_Decimal::New().const_get();
	replace(dobj);

	return dobj;
}



// ************************
// * DO_Decimal
// ************************

// C++ constructor
// use init() for individual object initialization
DO_Decimal::DO_Decimal()
{

}

// C++ destructor
// use destroy() for individual object de-construction
DO_Decimal::~DO_Decimal()
{
}

// init(): the "constructor"
void DO_Decimal::init()
{
	// initialize superclass first
	DO_Magnitude::init();

	value = RWDecimalPortable();

}

// destroy(): the "destructor"
void DO_Decimal::destroy()
{

	// destroy superclass last
	DO_Magnitude::destroy();
}


DR_String DO_Decimal::toString()
	{
	return (value);
	}

	


static RWCString addCommas(RWCString number)
// number has only digits, ie no '.' or '-'
	{
	const int bufLength=30;
	char buf[bufLength];
	buf[bufLength-1]=0;

	for (int i = number.length()-1,  j = bufLength-2, count = 0; i >= 0; i--, count++, j--)
		{
		if (count == 3)
			{
			count = 0;
			buf[j--]=',';
			}
		buf[j] = number[i];
		}
	return buf+j+1;

	}


static DR_String asHtmlFormat(RWCString prefix, RWCString suffix, RWDecimalPortable val)
	{
	// Insure  starting with string with 2 char decimal part
	RWCString str;
	if (val < zeroDP)
		str = -val;
	else
		str = val;

	int i=0, j=0;
	char integerPartBuf[30];
	while (i < str.length() && str[i] != '.')
			integerPartBuf[j++] = str[i++];
	integerPartBuf[j]=0;
	RWCString integerPart = addCommas(integerPartBuf);
	if (integerPart.length() == 0)
		integerPart="0";

	RWCString decimalPart = "";
	if (i < str.length() )
		decimalPart = str(i,str.length()-i);

	if (val < zeroDP)
		return("(" + prefix + integerPart + decimalPart + suffix + ")");
	return(prefix + integerPart + decimalPart + suffix + " &nbsp");

	}

	
DR_String DO_Decimal::asPercentageFormat()
	{
	RWDecimalPortable localVal = value * 100;
	localVal.trimZeros();
	return asHtmlFormat("", "%", localVal);
	}


DR_String DO_Decimal::asMoneyFormat(DR_String prefixIn)
// Positive numbers display as "$1,234.56 ", negative as "($1,234.56)"
	{
	RWDecimalPortable localVal = value;
	localVal.trimZeros();
	return asHtmlFormat(prefixIn.val(), "", round(localVal, 2,roundingMethod));
	}



// ********************************************************

// DOC_Decimal: the Decimal class class
// By defining this class in the .cc file, it is
// unavailable anywhere else. This is good, for now.
class DOC_Decimal : public DO_Class {
	D *spawn();

  public:
	DOC_Decimal() : DO_Class("Decimal") { }

};

DR_Class DO_Decimal::Decimalclass = new DOC_Decimal();
//DR_Class DO_Decimal::Decimalclass;

// The only place DO_Decimal objects should be created.
// This function is private; it is called from
// DO_Class::New().
D *DOC_Decimal::spawn() {
	return new DO_Decimal();
}

DRef DO_Decimal::Class()
{
	return Decimalclass;
}

// New()
// Create a new DO_Decimal by asking for one from
// the static class object, DO_Decimal::Decimalclass.
// That object will either create one, by calling the virtual
// function spawn(), that is, DOC_Decimal::spawn(),
// or getting one from its list of free objects.
//
// A static function
DR_Decimal DO_Decimal::New()
{
	return DO_Decimal::Decimalclass->New();
}

// Create_DOC_Decimal()
// not currently in use as of Oct 1, 1998
// It will be used as the "bootstrap" function
// to create the class object, which will in turn
// create and recycle instances of DO_Decimal
extern "C" DRef& Create_DOC_Decimal()
{
	DO_Decimal::Decimalclass = new DOC_Decimal();
	return DO_Decimal::Decimalclass;
}


// ********************************************************



// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
DRef DO_Decimal::route(DR_Message m)
{
    // until the semantics of route() are nailed down,
	// its body is commented out in the template text.
	// for the moment it simply bounces the message
	// to its superclass.

	switch(theIDHash(m->message->data()))
	{

        case _hASMONEYFORMAT: // RSL: String asNortherFormat( * String);
             return asMoneyFormat( DR_Object(m("String")) );
        case _hASPERCENTAGEFORMAT: // RSL: String asPercentageFormat( );
             return asPercentageFormat(  );
        case _hOpAdd: return opAdd(DRef(m("op")));
        case _hOpSubt: return opSubt(DRef(m("op")));
        case _hOpMult: return opMult(DRef(m("op")));
        case _hOpDiv: return opDiv(DRef(m("op")));

        case _hOpLT: return opLT(DRef(m("op")));
        case _hOpGT: return opGT(DRef(m("op")));
        case _hOpLE: return opLE(DRef(m("op")));
        case _hOpGE: return opGE(DRef(m("op")));
        case _hOpNE: return opNE(DRef(m("op")));
        case _hOpEQ2: return opEQ2(DRef(m("op")));

        case _hOpASSIGN: return opAssign(m("op"));
        case _hOpPE: return opPE(DRef(m("op")));
        case _hOpME: return opME(DRef(m("op")));
        case _hOpTE: return opTE(DRef(m("op")));
        case _hOpDE: return opDE(DRef(m("op")));



		default: ;
	}
	return DO_Magnitude::route(m);
}


DR_Magnitude DO_Decimal::Assign( const DR_Magnitude& ref )
{
	DR_Decimal d = toDecimal(ref);
	if ((DRef)d == DR_null)
		return this;
	getValue() = d->getValue();
	return this;
}




DR_Decimal DR_Decimal::operator+(DR_Decimal ref)
	{
	return DR_Decimal(safe_get()->value + ref->value);
	}

DR_Decimal DO_Decimal::opAdd(const DRef &ref)
	{
	DR_Decimal d = toDecimal(ref);
	if ((DRef)d == DR_null)
		return DR_null;

	return DR_Decimal(getValue() + d->getValue());

	}
DR_Decimal DR_Decimal::operator-(DR_Decimal ref)
	{
	return DR_Decimal(safe_get()->value - ref->value);
	}

DR_Decimal DO_Decimal::opSubt(const DRef &ref)
	{
	DR_Decimal d = toDecimal(ref);
	if ((DRef)d == DR_null)
		return DR_null;


	return DR_Decimal(getValue() - d->getValue());

	}

DR_Decimal DR_Decimal::operator*(DR_Decimal ref)
	{
	return DR_Decimal(toDouble(safe_get()->value) * toDouble(ref->value));
	}

		
DR_Decimal DO_Decimal::opMult(const DRef &ref)
	{
	DR_Decimal d = toDecimal(ref);
	if ((DRef)d == DR_null)
		return DR_null;

	double m1 = toDouble(getValue());
	double m2 = toDouble(d->getValue());

	return DR_Decimal(m1*m2);

	}


DR_Decimal DR_Decimal::operator/(DR_Decimal ref)
	{
	return DR_Decimal(toDouble(safe_get()->value) / toDouble(ref->value));
	}

		
DR_Decimal DO_Decimal::opDiv(const DRef &ref)
	{
	DR_Decimal d = toDecimal(ref);
	if ((DRef)d == DR_null)
		return DR_null;

	double m1 = toDouble(value);
	double m2 = toDouble(d->value);
	return DR_Decimal(m1/m2);

	}



DR_Decimal& DR_Decimal::operator+=(DR_Decimal ref)
	{
	safe_get()->value = safe_get()->value  + ref->value;
	return *this;
	}

		
DR_Decimal DO_Decimal::opPE(const DRef &ref)
	{
	DR_Decimal d = toDecimal(ref);
	if ((DRef)d == DR_null)
		return DR_null;

	getValue() = getValue() + d->getValue();
	return DR_Decimal(this);
	}

DR_Decimal& DR_Decimal::operator-=(DR_Decimal ref)
	{
	safe_get()->value = safe_get()->value  - ref->value;
	return *this;
	}

		
DR_Decimal DO_Decimal::opME(const DRef &ref)
	{
	DR_Decimal d = toDecimal(ref);
	if ((DRef)d == DR_null)
		return DR_null;

	getValue() = getValue() - d->getValue();
	return DR_Decimal(this);
	}

DR_Decimal& DR_Decimal::operator*=(DR_Decimal ref)
	{
	double m1 = toDouble(safe_get()->value);
	double m2 = toDouble(ref->value);
	safe_get()->value = toRWDP(m1*m2);
	return *this;
	}

		
DR_Decimal DO_Decimal::opTE(const DRef &ref)
	{
	DR_Decimal d = toDecimal(ref);
	if ((DRef)d == DR_null)
		return DR_null;

	double m1 = toDouble(getValue());
	double m2 = toDouble(d->getValue());
	value =toRWDP(m1*m2);
	return DR_Decimal(this);
	}



DR_Decimal& DR_Decimal::operator/=(DR_Decimal ref)
	{
	double m1 = toDouble(safe_get()->value);
	double m2 = toDouble(ref->value);
	safe_get()->value = toRWDP(m1/m2);
	return *this;
	}

		
DR_Decimal DO_Decimal::opDE(const DRef &ref)
	{
	DR_Decimal d = toDecimal(ref);
	if ((DRef)d == DR_null)
		return DR_null;

	double m1 = toDouble(getValue());
	double m2 = toDouble(d->getValue());
	value = toRWDP(m1/m2);
	return DR_Decimal(this);
	}

DR_Decimal& DR_Decimal::operator=(DR_Decimal ref)
	{
	safe_get()->value = ref->value;
	return *this;
	}

		
DR_Decimal DO_Decimal::opAssign(const DRef &ref)
	{
	DR_Decimal d = toDecimal(ref);
	if ((DRef)d == DR_null)
		return DR_null;

	value = d->getValue();

	return DR_Decimal(this);
	}



int DR_Decimal::operator<(DR_Decimal ref)
	{
	return safe_get()->value < ref->value;
	}

DR_Bool DO_Decimal::opLT(const DRef &ref)
	{
	DR_Decimal d = toDecimal(ref);
	if ((DRef)d == DR_null)
		return DR_Bool(0);

	if(getValue() < d->getValue())
			return DR_Bool(1);
	return DR_Bool(0);
	}


int DR_Decimal::operator>(DR_Decimal ref)
	{
	return safe_get()->value > ref->value;
	}

DR_Bool DO_Decimal::opGT(const DRef &ref)
	{
	DR_Decimal d = toDecimal(ref);
	if ((DRef)d == DR_null)
		return DR_Bool(0);

	if(getValue() > d->getValue())
			return DR_Bool(1);
	return DR_Bool(0);
	}


int DR_Decimal::operator<=(DR_Decimal ref)
	{
	return safe_get()->value <= ref->value;
	}

DR_Bool DO_Decimal::opLE(const DRef &ref)
	{
	DR_Decimal d = toDecimal(ref);
	if ((DRef)d == DR_null)
		return DR_Bool(0);

	if(getValue() <= d->getValue())
			return DR_Bool(1);
	return DR_Bool(0);
	}


int DR_Decimal::operator>=(DR_Decimal ref)
	{
	return safe_get()->value >= ref->value;
	}

DR_Bool DO_Decimal::opGE(const DRef &ref)
	{
	DR_Decimal d = toDecimal(ref);
	if ((DRef)d == DR_null)
		return DR_Bool(0);

	if(getValue() >= d->getValue())
			return DR_Bool(1);
	return DR_Bool(0);
	}

int DR_Decimal::operator==(DR_Decimal ref)
	{
	return safe_get()->value == ref->value;
	}

DR_Bool DO_Decimal::opEQ2(const DRef &ref)
	{
	DR_Decimal d = toDecimal(ref);
	if ((DRef)d == DR_null)
		return DR_Bool(0);

	if(getValue() == d->getValue())
			return DR_Bool(1);
	return DR_Bool(0);
	}


int DR_Decimal::operator!=(DR_Decimal ref)
	{
	return safe_get()->value != ref->value;
	}

DR_Bool DO_Decimal::opNE(const DRef &ref)
	{
	DR_Decimal d = toDecimal(ref);
	if ((DRef)d == DR_null)
		return DR_Bool(0);

	if(getValue() != d->getValue())
			return DR_Bool(1);
	return DR_Bool(0);
	}













DR_Decimal::DR_Decimal(RWDecimalPortable d) : DR_Magnitude((D*) 0)
{
#ifdef DMEMORY
	cerr << "DR_Decimal(RWDecimalPortable)\n";
#endif

	safe_get()->value = d;
}




DR_Decimal::DR_Decimal(double d): DR_Magnitude((D*) 0)
{
#ifdef DMEMORY
	cerr << "DR_Decimal(RWDecimalPortable)\n";
#endif

	
	safe_get()->value = toRWDP(d);
}

DR_Decimal::DR_Decimal(int i): DR_Magnitude((D*) 0)
{
	
safe_get()->value = RWDecimalPortable(i);
}

DR_Decimal::DR_Decimal(DR_Int i): DR_Magnitude((D*) 0)
{
	if (i.isValid())
		{
		safe_get()->value = RWDecimalPortable(i->i());
		}
}

DR_Decimal::DR_Decimal(RWCString s): DR_Magnitude((D*) 0)
{

	RWDecimalPortable d(s);
	if (d.isNumber())
		safe_get()->value = d;
}



DR_Decimal::DR_Decimal(DR_String s): DR_Magnitude((D*) 0)
{
	if (s.isValid())
		{
	RWCString tmp(s->data());
	RWDecimalPortable d(tmp);
	if (d.isNumber())
		safe_get()->value = d;
		}
}

DR_Decimal::DR_Decimal(char * s): DR_Magnitude((D*) 0)
{
	RWCString tmp(s);
	RWDecimalPortable d(tmp);
	if (d.isNumber())
		safe_get()->value = d;
}



DR_String DR_Decimal::toString()
	{
	return safe_get()->toString();
	}

dcompare_t DO_Decimal::compare(const DRef& d) const
	{
	if (!d.isValid())
		return c_greater;

	const DR_Decimal &drt= dynamic_cast<const DR_Decimal &>(d);
	DR_Decimal tmp = drt;

	if (value < tmp->getValue())
		return c_less;
	if (value > tmp->getValue())
		return c_greater;
	return c_equal;
	}

