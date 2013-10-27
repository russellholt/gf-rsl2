#include "DMagnitude.h"
#include <iostream.h>

static char rcsid[] = "$Id: DMagnitude.cc,v 1.2 1998/11/24 14:55:49 holtrf Exp $";

// ************************
// * DR_Magnitude
// ************************

#define _hASSIGN 14909
#define _hADD 43
#define _hSUBT 45
#define _hMULT 42
#define _hDIV 47
#define _hEQ2 15677
#define _hNE 8509
#define _hLT 60
#define _hGT 62
#define _hLE 15421
#define _hGE 15933


DR_Magnitude::DR_Magnitude(D *d) : DR_Object(d)
{

}

DR_Magnitude::DR_Magnitude(const DRef& ref) : DR_Object(ref)
{

}

// If you redefine dump() or Recycle(), you must
// call dump() from this destructor because DRef::dump(), invoked
// from DRef::~DRef(), may call Recycle() - and destructors do not
// call functions in derived classes. otherwise, don't call it.
DR_Magnitude::~DR_Magnitude()
{

}

DO_Magnitude* DR_Magnitude::const_get()
{
	return dynamic_cast<DO_Magnitude*>(unsafe_get());
}


DO_Magnitude* DR_Magnitude::safe_get()
{
	SAFE_GET(DO_Magnitude);
}

// rarely used.
// _set() when safe_get() is used avoids double checking.
DO_Magnitude* DR_Magnitude::safe_set(D* d)
{
	SAFE_SET(DO_Magnitude,d);	// D_macros.h
}

DO_Magnitude *DR_Magnitude::New()
{
	cerr << "Magnitude::New() will default to Integer.\n";
	return (DO_Magnitude *) 0;
}

// ************************
// * DO_Magnitude
// ************************

// C++ constructor
// use init() for individual object initialization
DO_Magnitude::DO_Magnitude()
{

}

// C++ destructor
// use destroy() for individual object de-construction
DO_Magnitude::~DO_Magnitude()
{
}

// init(): the "constructor"
void DO_Magnitude::init()
{
	// initialize superclass first
	DO_Atom::init();

}

// destroy(): the "destructor"
void DO_Magnitude::destroy()
{

	// destroy superclass last
	DO_Atom::destroy();
}

DRef DO_Magnitude::Class()
{
	return DClass();
}

//DR_String DO_Magnitude::toString()
//{
//	return DR_String("0");
//}

//// ********************************************************
//
//// DOC_Magnitude: the Magnitude class class
//// By defining this class in the .cc file, it is
//// unavailable anywhere else. This is good, for now.
//class DOC_Magnitude : public DO_Class {
//	D *spawn();
//
//  public:
//	DOC_Magnitude() : DO_Class("Magnitude") { }
//
//};
//
//DR_Class DO_Magnitude::Magnitudeclass = new DOC_Magnitude();
//
//// The only place DO_Magnitude objects should be created.
//// This function is private; it is called from
//// DO_Class::create().
//D *DOC_Magnitude::spawn() {
//	return new DO_Magnitude();
//}
//
//DRef DO_Magnitude::Class()
//{
//	return Magnitudeclass;
//}
//
//// New()
//// A static function
//DR_Magnitude DO_Magnitude::New()
//{
//	// this calls the function
//	// D* DO_Class::New()
//	// so returning a D* invokes the constructor
//	// DR_Magnitude(D *)
//	return DO_Magnitude::Magnitudeclass->New();
//}
//
//// Create_DOC_Magnitude()
//// not currently in use as of Oct 1, 1998
//// It will be used as the "bootstrap" function
//// to create the class object, which will in turn
//// create and recycle instances of DO_Magnitude
//extern "C" DRef& Create_DOC_Magnitude()
//{
//	DO_Magnitude::Magnitudeclass = new DOC_Magnitude();
//	return DO_Magnitude::Magnitudeclass;
//}


// ********************************************************

#ifdef DR_Boolean
#define cboolean(x) DR_Boolean(x)
#else
#define cboolean(x) DR_null
#endif

// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
DRef DO_Magnitude::route(DR_Message m)
{

#ifdef DMEMORY
	cerr << "Magnitude::route() for `" << m->message << "'\n";
#endif

	switch(m->messageCode)
	{
		case _hASSIGN: // RSL: Magnitude :=( Magnitude n);
			return Assign( DR_Magnitude(m("n")) );
//		case _hADD: // RSL: Magnitude +( Magnitude n);
//			return Add( DR_Magnitude(m("n")) );
//		case _hSUBT: // RSL: Magnitude -( Magnitude n);
//			return Subt( DR_Magnitude(m("n")) );
//		case _hMULT: // RSL: Magnitude *( Magnitude n);
//			return Mult( DR_Magnitude(m("n")) );
//		case _hDIV: // RSL: Magnitude /( Magnitude n);
//			return Div( DR_Magnitude(m("n")) );
//		case _hEQ2: // RSL: Boolean ==( Magnitude n);
//			return cboolean(isEqual( DR_Magnitude(m("n")) ) );
//		case _hNE: // RSL: Boolean !=( Magnitude n);
//			return cboolean(notEqual( DR_Magnitude(m("n")) ) );
//		case _hLT: // RSL: Boolean <( Magnitude n);
//			return cboolean(less( DR_Magnitude(m("n")) ) );
//		case _hGT: // RSL: Boolean >( Magnitude n);
//			return cboolean(greater( DR_Magnitude(m("n")) ) );
//		case _hLE: // RSL: Boolean <=( Magnitude n);
//			return cboolean(lessOrEqual( DR_Magnitude(m("n")) ) );
//		case _hGE: // RSL: Boolean >=( Magnitude n);
//			return cboolean(greaterOrEqual( DR_Magnitude(m("n")) ) );

		default: ;
	}
	return DO_Atom::route(m);
}


//DR_Magnitude& DO_Magnitude::Assign( const DR_Magnitude& n )
//{
//	return (*this);
//}
//DR_Magnitude DO_Magnitude::Add( const DR_Magnitude& n )
//{
//	return n;
//}
//DR_Magnitude DO_Magnitude::Subt( const DR_Magnitude& n )
//{
//	return n;
//}
//DR_Magnitude DO_Magnitude::Mult( const DR_Magnitude& n )
//{
//	return n;
//}
//DR_Magnitude DO_Magnitude::Div( const DR_Magnitude& n )
//{
//	return n;
//}
//BOOLEAN DO_Magnitude::isEqual( const DR_Magnitude& n )
//{
//	return DR_FALSE;
//}
//BOOLEAN DO_Magnitude::notEqual( const DR_Magnitude& n )
//{
//	return DR_FALSE;
//}
//BOOLEAN DO_Magnitude::less( const DR_Magnitude& n )
//{
//	return DR_FALSE;
//}
//BOOLEAN DO_Magnitude::greater( const DR_Magnitude& n )
//{
//	return DR_FALSE;
//}
//BOOLEAN DO_Magnitude::lessOrEqual( const DR_Magnitude& n )
//{
//	return DR_FALSE;
//}
//BOOLEAN DO_Magnitude::greaterOrEqual( const DR_Magnitude& n )
//{
//	return DR_FALSE;
//}
