/**********************************************************************


 Context is
     DR_Context
     DO_Context
     DO_ContextEnumerator


 $Id: DContext.h,v 1.1 1998/11/12 18:51:21 holtrf Exp $
**********************************************************************/

// // Loading templates from directory "/dest/home/holtrf/dev/packages/rsl2cpp/templates/DCollection/"
// declaration: List otherContexts;
//   generating.


#include "DKeyedCollection.h"

#ifndef _D_Context_
#define _D_Context_

#define _D_Context_ID 639048308

class DO_Context;

// **********************************************************************
// class DR_Context is a DR_KeyedCollection
// a smart pointer
// **********************************************************************
class DR_Context : public DR_KeyedCollection {
  public:
	DR_Context (D *d=0);
	DR_Context (const DRef& ref);
	virtual ~DR_Context();

	DO_Context *safe_get();
	DO_Context *safe_set(D* d);
	DO_Context *New();

	inline DO_Context *operator->() { return safe_get(); }
};

// **********************************************************************
// DO_Context is a DO_KeyedCollection
// the DObject component: the real thing
// **********************************************************************
class DO_Context : public DO_KeyedCollection {
	RWTValSlist<DRef> thelist;
	
public:
	DO_Context();
	DO_Context(DRef r); // casting constructor
	virtual ~DO_Context();
	
	// override DO_Object virtual functions
	void init();
	void destroy();
	
	// DO_Collection virtuals
	inline DR_void add ( DR_Object o ) { return append(o); }
	DO_Enumerator *elements();

	// DO_Context methods
	DR_void append ( DR_Object o );
	
		// Instance variables generated from Context.rsl
	/**  */
	DR_List otherContexts ;


	// Methods generated from Context.rsl
	
};


// **********************************************************************
// Collection Enumerator
// * If you derive from an existing collection with well-defined, this
// class may not be necessary.
//
// * All DO_Enumerator subclasses are used through the
// DR_Enumerator interface class; no DR_ContextEnumerator
// class should be necessary.
// **********************************************************************
class DO_ContextEnumerator : public DO_Enumerator {
	// enumerator data goes here

public:
	DO_ContextEnumerator(RWTValSlist<DRef>& l) : iter(l) { }
	virtual ~DO_ContextEnumerator();

	// superclass virtuals
	inline BOOLEAN hasMoreElements (  ) { return (BOOLEAN) (iter()); }
	inline DRef nextElement ( ) { return iter.key(); }

	void init();
	void destroy();
	DR_String toString();
};
#endif

###FILESEPARATOR###
// $Id: DContext.h,v 1.1 1998/11/12 18:51:21 holtrf Exp $

#include "DContext.h"

// ************************
// * DR_Context
// ************************
static char rcsid[] = "$Id: DContext.h,v 1.1 1998/11/12 18:51:21 holtrf Exp $";

// Context
#define _hAPPEND 252997733
#define _hELEMENTS 135454



DR_Context::DR_Context(D *d) : DR_Collection(d)
{

}

DR_Context::DR_Context(const DRef& ref) : DR_Collection(ref)
{

}

DR_Context::~DR_Context()
{

}

DO_Context* DR_Context::safe_get()
{
	SAFE_GET(DO_Context);
}

DO_Context* DR_Context::safe_set(D* d)
{
	SAFE_SET(DO_Context,d);
}

DO_Context *DR_Context::New()
{
	// TEMPORARY!
	return safe_set(new DO_Context());
}

// ************************
// * DO_Context
// ************************

DO_Context::DO_Context()
{

}

DO_Context::~DO_Context()
{

}

// init(): the "constructor"
void DO_Context::init()
{
	// initialize superclass first
	DO_Object::init();

}

// destroy(): the "destructor"
void DO_Context::destroy()
{
	thelist.clear();

	// destroy superclass last
	DO_Object::destroy();
}



// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
/*
DRef DO_Context::route(DR_Message m)
{
	switch(m.messageID())
	{
		case _hAPPEND:
			return append( DR_Object(m[0]) );
		case _hELEMENTS:
			return elements(  );



		default: ;
	}
	return DO_KeyedCollection::route(m);
}
*/



/**
	append
	In RSL: void append( Object o);
*/
DR_void DO_Context::append( DR_Object o )
{
	thelist.append(o);
	return DR_null;
}

DO_Enumerator *DO_Context::elements()
{
	return new DO_ContextEnumerator(thelist);
}

// ************************
// * DO_ContextEnumerator
// ************************

// ContextEnumerator
#define _hHASMOREELEMENTS 84019309
#define _hNEXTELEMENT 1315399961

// C++ destructor
// use destroy() for individual object de-construction
DO_ContextEnumerator::~DO_ContextEnumerator()
{
}

// init(): the "constructor"
void DO_ContextEnumerator::init()
{
	// initialize superclass first
	DO_Enumerator::init();
	add("otherContexts", otherContexts.New());

}

// destroy(): the "destructor"
void DO_ContextEnumerator::destroy()
{
	otherContexts.dump();


	// destroy superclass last
	DO_Enumerator::destroy();
}

DR_String DO_ContextEnumerator::toString()
{
	return DR_String(DNULL);
}

// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
/*
DRef DO_ContextEnumerator::route(DR_Message m)
{
	switch(m.messageID())
	{
		case _hHASMOREELEMENTS:
			return hasMoreElements(  );
		case _hNEXTELEMENT:
			return nextElement(  );

		default: ;
	}
	return DO_Enumerator::route(m);
}
*/
