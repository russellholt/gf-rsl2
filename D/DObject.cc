// $Id: DObject.cc,v 1.4 1998/12/03 21:53:29 holtrf Exp $
#include "D.h"
#include "DClass.h"
#include "DInt.h"
#include "DBool.h"

#include <iostream.h>

DO_Object *DR_Object::safe_get()
{
	DO_Object *d = dynamic_cast<DO_Object *> (unsafe_get());
	if (d)
		return d;

	// are there other conditions here specific
	// to DO_Object?

	cerr << "DR_Object::safe_get() cannot instantiate an unknown class\n";

	return (DO_Object *)0;
}

DR_Object::~DR_Object()
{
	
}

DO_Object *DR_Object::safe_set(D* d)
{
	SAFE_SET(DO_Object, d); // d_macros.h
}
	

DO_Object::~DO_Object()
{
#ifdef DMEMORY
	cerr << "~DO_Obect()\n";
#endif
}

#define _hASSIGNOP 14909
#define _hASSIGN 102593385
#define _hTOSTRING 101072147
#define _hTOBOOLEAN 1963140878
#define _hCLASSOBJ 268632857
#define _hDTYPEID 18685296
#define _hROUTE 393180532
#define _hLS 15420
#define _hDEEPCOPY 654972169

DRef DO_Object::route(DR_Message m)
{
#ifdef DMEMORY
	cerr << "DO_Object::route(DR_Message m)\n";
#endif

	switch(theIDHash(m->message->data()))
	{
		case _hASSIGNOP: // RSL: void :=( Object o);
		case _hASSIGN: // RSL: void assign( Object o);
			{ assign( m("o") ); return DR_null; }

		case _hTOSTRING: // RSL: String toString( );
			return toString(  );

		case _hTOBOOLEAN: // RSL: Bool toBoolean( );
			return DR_Bool(toBoolean(  ));

		case _hCLASSOBJ: // RSL: Class classobj( );
			return Class( );

		case _hDTYPEID: // RSL: Int dtypeid( );
			return DR_Int(dtypeid(  ));

		case _hLS: // RSL: void <<( Message m);
		case _hROUTE: // RSL: Object route( Message m);
		{
			// "msg" is the version that has a string
			// argument that contains the name of the
			// message. I'll just use that to change
			// the name of the current message and
			// bounce it along. Notice that unless the
			// acutal parameters were given with
			// names they won't be here.
			DR_String msg  = m("msg");
			if (msg.const_get())	// if it is a valid DO_String
			{
				// Change the message and re-route
				m->message = msg;
				return route(m);
			}
			return route( DR_Message(m("m")) );
		}

		//case _hLS: // RSL: void <<( Message m);
		//	return route( DR_Message(m("m")) );

		case _hDEEPCOPY: // RSL: Object deepCopy( );
			return deepCopy(  );

		default: ;
	}

	return doesNotUnderstand(m);
}

DRef DO_Object::Class() {
	
	return DR_Class((D*)0);
}

