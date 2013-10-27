#include "Dcomparator.h"

#include <iostream.h>

static char rcsid[] = "$Id: Dcomparator.cc,v 1.1 1998/11/12 18:31:37 holtrf Exp $";

// ************************
// * DR_comparator
// ************************

#define _hLS 15420
#define _hADDNAME 590158
#define _hCLEARNAMES 1951466508
#define _hCOMPARE 35457136
#define _hSETNAMELIST 2071684354
#define _hADDSUBCOMPARATOR 402794812


DR_comparator::DR_comparator(D *d) : DR_Object(d)
{

}

DR_comparator::DR_comparator(const DRef& ref) : DR_Object(ref)
{

}

// If you redefine dump() or Recycle(), you must
// call dump() from this destructor because DRef::dump(), invoked
// from DRef::~DRef(), may call Recycle() - and destructors do not
// call functions in derived classes. otherwise, don't call it.
DR_comparator::~DR_comparator()
{

}

// const_get()
// get the object as the correct type, but without the potential
// to modify it if it is null.
DO_comparator* DR_comparator::const_get() const
{
	return dynamic_cast<DO_comparator *> (unsafe_get());
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
DO_comparator* DR_comparator::safe_get()
{
	SAFE_GET(DO_comparator);	// defined in D_macros.h
}

// rarely used.
// replace() when safe_get() will be used avoids double checking.
DO_comparator* DR_comparator::safe_set(D* d)
{
	SAFE_SET(DO_comparator,d);	// defined in D_macros.h
}

DO_comparator *DR_comparator::New()
{
	DO_comparator *dobj = DO_comparator::New().const_get();
	replace(dobj);

	return dobj;
}

// ************************
// * DO_comparator
// ************************

// C++ constructor
// use init() for individual object initialization
DO_comparator::DO_comparator()
{

}

// C++ destructor
// use destroy() for individual object de-construction
DO_comparator::~DO_comparator()
{
}

// init(): the "constructor"
void DO_comparator::init()
{
	// initialize superclass first
	DO_Object::init();

	fieldNames.New();
	followSecondaryRefs = 0;
}

// destroy(): the "destructor"
void DO_comparator::destroy()
{
	fieldNames.dump();
	subcomparators.dump();

	// destroy superclass last
	DO_Object::destroy();
}

DR_String DO_comparator::toString()
{
	return DR_String("comparator toString() not implemented");
}

// ********************************************************

// DOC_comparator: the comparator class class
// By defining this class in the .cc file, it is
// unavailable anywhere else. This is good, for now.
class DOC_comparator : public DO_Class {
	D *spawn();

  public:
	DOC_comparator() : DO_Class("comparator") { }

};

DR_Class DO_comparator::comparatorclass = new DOC_comparator();
//DR_Class DO_comparator::comparatorclass;

// The only place DO_comparator objects should be created.
// This function is private; it is called from
// DO_Class::New().
D *DOC_comparator::spawn() {
	return new DO_comparator();
}

DRef DO_comparator::Class()
{
	return comparatorclass;
}

// New()
// Create a new DO_comparator by asking for one from
// the static class object, DO_comparator::comparatorclass.
// That object will either create one, by calling the virtual
// function spawn(), that is, DOC_comparator::spawn(),
// or getting one from its list of free objects.
//
// A static function
DR_comparator DO_comparator::New()
{
	return DO_comparator::comparatorclass->New();
}

// Create_DOC_comparator()
// not currently in use as of Oct 1, 1998
// It will be used as the "bootstrap" function
// to create the class object, which will in turn
// create and recycle instances of DO_comparator
extern "C" DRef& Create_DOC_comparator()
{
	DO_comparator::comparatorclass = new DOC_comparator();
	return DO_comparator::comparatorclass;
}


// ********************************************************



// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
DRef DO_comparator::route(DR_Message m)
{
    // until the semantics of route() are nailed down,
	// its body is commented out in the template text.
	// for the moment it simply bounces the message
	// to its superclass.

	switch(theIDHash(m->message->data()))
	{
		case _hLS: // RSL: void <<( String field);
		case _hADDNAME: // RSL: void addName( String name);
			return addName( DR_String(m("name")) );

		case _hCLEARNAMES: // RSL: void clearNames( );
			return clearNames(  );

		case _hCOMPARE: // RSL: Int compare( Composite l, Composite r);
			return DR_Int( compare( DR_Composite(m("l")), DR_Composite(m("r")) ) );

        case _hSETNAMELIST: // RSL: void setNameList( List names);
			return setNameList( DR_List(m("names")) );

        case _hADDSUBCOMPARATOR: // RSL: void addSubComparator( String className, comparato>
            return addSubComparator( DR_String(m("className")), DR_comparator(m("c")) );

		default: ;
	}
	return DO_Object::route(m);
}


// comparing comparators!
dcompare_t DO_comparator::compare(const DRef& d) const
{
	return c_less;
}


DR_void DO_comparator::addName ( const char *fieldName)
{
	fieldNames->append(DR_String(fieldName));
	return DR_null;
}

DR_void DO_comparator::addName( const DR_String& name )
{
	fieldNames->append(name);
	return DR_null;
}

DR_void DO_comparator::setNameList ( DR_List names, ref_t t )
{
	fieldNames.replace(names.unsafe_get(), t);
	return DR_null;
}

/**
	clearNames
	In RSL: void clearNames( );
*/
DR_void DO_comparator::clearNames(  )
{
	if (fieldNames.isValid())
		fieldNames->clear();

	return DR_null;
}


/**
	compare
	In RSL: Int compare( Composite l, Composite r);
*/
dcompare_t DO_comparator::compare( const DR_Composite& l, const DR_Composite& r )
{

	DO_Composite *left_obj = l.const_get();
	DO_Composite *right_obj = r.const_get();

	if (!left_obj || !right_obj)
		return c_less;

// there's nothing like a ton of ifdefs to destroy the visual
// cohesion of an algorithm. C++ is cool.
#ifdef DMEMORY
	cerr << "comparator: comparing `" << left_obj->toString() << "' and `"
		<< right_obj->toString() << "'.\n";
#endif

	DR_Enumerator fn = fieldNames->elements();

	dcompare_t comparison = c_equal;
	while(comparison==c_equal && fn->hasMoreElements())
	{
		DR_String s = fn->nextElement();

		DRef left_field = left_obj->get(s);
		DRef right_field = right_obj->get(s);

		// following secondary references could lead to infinite loops.
		// Choosing to compare them is optional.
		if ( !followSecondaryRefs )
		{
			// skip secondary reference comparison
			// Questions: if one is, does it matter if the other is?
			// We'll check both.
			if (left_field.RefKind() == secondary_ref
				|| right_field.RefKind() == secondary_ref)
			{

#ifdef DMEMORY
				cerr << "comparator: skipping secondary ref.\n";
#endif

				continue;
			}
		}

		if (!left_field.isValid() || !right_field.isValid())
		{

#ifdef DMEMORY
			cerr << "comparator: found invalid field.. leaving..\n";
#endif

			// always "less than" by default.
			comparison = c_less;
			break;
		}

		if (dynamic_cast<DO_Composite *> (left_field.unsafe_get())
			&& dynamic_cast<DO_Composite *> (right_field.unsafe_get()))
		{

#ifdef DMEMORY
			cerr << "comparator: found composite field.\n";
#endif

			if (subcomparators.isValid())
			{
				DR_comparator subc = findSubCFrom(left_field);
				if (subc.isValid())
					comparison = subc->compare(left_field, right_field);
			}

#ifdef DMEMORY
			else
				cerr << "comparator: can't compare composite fields `" << s << "'.\n";
#endif

		}

		else
		{
#ifdef DMEMORY
			cerr << "\tcomparing `" << left_field.toString() << "' and `"
				<< right_field.toString() << "'...";
#endif
			comparison = left_field.compare(right_field);
		}

	}

#ifdef DMEMORY
	cerr << "\tcomparison is:" << comparison << endl;
#endif

	return comparison;
}

DRef DO_comparator::findSubCFrom(DRef& ref)
{
	DR_Class cl = ref.Class();
	if (cl.isValid())
		return subcomparators->rw_get(cl->className());

	return DR_null;
}


DR_void DO_comparator::addSubComparator ( const DR_String& className, DR_comparator c )
{
	DO_String *dos = className.const_get();
	if (dos)
		subcomparators->cc_add(dos->data(), c);

	return DR_null;
}


