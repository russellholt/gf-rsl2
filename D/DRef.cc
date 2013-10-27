#include "D.h"

#include "DClass.h"

#include "DCollection.h"

#include <iostream.h>



#include "DInt.h"
#include "DBool.h"

#ifdef PURIFY
#include "purify.h"
#endif

DRef::DRef(D *d)
{
	that = DNULL;

	if (d != DNULL)
	    dump(d, primary_ref);
}

// Constructor to explicitly allow a kind of reference.
DRef::DRef(D *d, ref_t kind)
{
	that = DNULL;
	refKind=primary_ref;
	if (d != DNULL)
	    dump(d, kind);
}
 
DRef::DRef(const DRef& ref)
{
	that = DNULL;
	refKind=primary_ref;
	if (ref.isValid())
		dump(ref.unsafe_get(), ref.RefKind());
}

DRef::DRef(const DRef& dref, ref_t kind)
{
	that = DNULL;
	refKind=primary_ref;
	if (dref.isValid())
		dump(dref.unsafe_get(), kind);
}

DRef::~DRef()
{
#ifdef DMEMORY
	cerr << "\t~DRef()\n";
#endif

	// avoid obviously extraneous invocations of dump()
	if (that != DNULL)
		dump();
}

void DRef::init()
{
	if (that)
		that->init();
}

void DRef::destroy()
{
	if (that)
		that->destroy();
}

BOOLEAN DRef::toBoolean() const
{
	if (that)
		return that->toBoolean();
}

void DRef::assign(const DRef& obj)
{
	if (that)
		that->assign(obj);
}

unsigned DRef::dtypeid()
{
	if (that)
		return that->dtypeid();

	return 0;
}

DRef DRef::Class()
{
	if (that)
		return that->Class();
	return DR_null;
}

dcompare_t DRef::compare(const DRef& d) const
{
	if (that)
		return that->compare(d);

	return c_less;
}


// dump current reference, and
// replace with d (0 by default)
// return the ref count of the object that is set.
void DRef::dump(D* d, ref_t ref_kind)
{
#ifdef DMEMORY
	cerr << "\tDRef::dump()\n";
#endif

	if (that == d) // including both being null
	{
#ifdef DMEMORY
		cerr << "\t\tDRef::dump(): replace with same object will be ignored.\n";
#endif
		return;
	}

	if (that)
	{
		// if we are a primary reference, then decrement the primary
		// reference count of our object. A primary reference count
		// decrement may result in the object being recycled.
		if (refKind == primary_ref)
		{
			int dr = that->ref_count.decPrimary();

			if (dr <= 0)
			{
#ifdef DMEMORY
				cerr << "\t\tDRef::dump(): calling Recycle...\n";
#endif
#ifdef PURIFY
			purify_printf("dumped a primary reference and recycling (%dp, %ds)\n",
				that->ref_count.Primary(), that->ref_count.Secondary());
#endif PURIFY
				Recycle();
			}
		}
		else
		// we're a secondary reference, so decrement the secondary ref
		// count. This will not recycle.
		{

#ifdef DMEMORY
				cerr << "\t\tDRef::dump(): dec seconary, ignoring ref count....\n";
#endif

			that->ref_count.decSecondary();

#ifdef PURIFY
			purify_printf("dumped a secondary reference to: (%dp, %ds)\n",
				that->ref_count.Primary(), that->ref_count.Secondary());
#endif PURIFY
		}
	}

	// if there are no primary references, then we are the primary reference.
	// otherwise be whatever kind was given in the ref_kind argument.

	that = d;

	if (that)
	{

		if (d->ref_count.Primary() == 0)
		{
#ifdef PURIFY
			if (ref_kind == secondary_ref)
				purify_printf("Making a primary reference although secondary was requested from (%dp, %ds)",
				that->ref_count.Primary(), that->ref_count.Secondary());
#endif PURIFY
			refKind = primary_ref;
		}
		else
		{
#ifdef PURIFY
		if (ref_kind == secondary_ref)
			purify_printf("Making a secondary reference from (%dp, %ds)",
				that->ref_count.Primary(), that->ref_count.Secondary());
#endif PURIFY
			refKind = ref_kind;
		}

		that->ref_count.newReference(refKind);

#ifdef PURIFY
	purify_printf("  to (%dp, %ds)\n", that->ref_count.Primary(), that->ref_count.Secondary());
#endif PURIFY

	}
}

// Recycle()
// give our object back to its class object.
// If we can't find a class object, attempt to delete it.
//
// This situation will probably happen at exit during the destruction
// of global static objects. Some static class objects will have been
// destroyed and others will not, and there may be objects still hanging
// around whose class objects will already have been destroyed, so we
// need to just delete them.
void DRef::Recycle()
{
#ifdef DMEMORY
	cerr << "\tDRef::Recycle() ";
#endif

	if (!that)
	{
#ifdef DMEMORY
	cerr << "\tDRef::Recycle() : null pointer!\n";
#endif
		return;
	}

	DR_Class drc = that->Class();
	if (drc.isValid())
	{
#ifdef DMEMORY
		cerr << "\t\tcalling Recycle() on the object's class..\n";
#endif
		drc->Recycle(that);
	}
	else
	{
#ifdef DMEMORY
		// At this point, a recyler for the object can't be found.
		// So we will be deleting it. But first try and figure
		// out what type it is or what supertype it has.

		cerr << "\tRecycling error, no class object. refcount " << that->refCount() << "\n";
		DO_Class *clobj = dynamic_cast<DO_Class *> (that);
		if (clobj)
		{
			cerr << "\t\tClass objects should be recycled. (`" << clobj->className()
				<< "') ";
		}
		else
		if (dynamic_cast<DO_Enumerator *> (that))
			cerr << "\t\tcan't recycle a DO_Enumerator! ";
		else
		if (dynamic_cast<DO_recycler *> (that))
			cerr << "\t\tCan't recycle a DO_recycler! ";
		else
		if (dynamic_cast<DO_Atom *> (that))
		{
			cerr << "\t\tit's a DO_Atom.. ";
			if (dynamic_cast<DO_Magnitude *> (that))
			{
				cerr << "and a DO_Magnitude..";
					if (dynamic_cast<DO_Int *> (that))
						cerr << "and a DO_Int.. ";
					else
						if (dynamic_cast<DO_Bool *> (that))
							cerr << "and a DO_Bool..";
						else
							cerr << "an unknown DO_Magnitude subclass..";
			}
			else
				if (dynamic_cast<DO_String *> (that))
				{
#ifdef D_KEEP_INUSE
					cerr << "and a DO_String. Calling _destroy() and explicitly deleting.\n";

					/*
					that->_destroy();
					delete that;
					_unsafe_clear();
					*/
#endif
				}
				else
					cerr << "and an uknown DO_Atom subclass!! ";

		}
		else
			if (dynamic_cast<DO_Collection *> (that)) {
				cerr << "\t\tcan't recycle a DO_Collection??";
			}
#endif

#ifndef D_KEEP_INUSE
		delete that;
#endif
	}

	that = DNULL;
}

// Pointer assignment.
DRef& DRef::operator=(const DRef &dref)
{
	dump(dref.unsafe_get(), dref.RefKind());
	return *this;
}

DRef& DRef::operator=(D *d)
{
	dump(d);
	return *this;
}

// Do they point to the same things?
int DRef::operator==(const DRef &dref)
{
	return (unsafe_get() == dref.unsafe_get());
}

int DRef::operator==(const D* d)
{
	return (unsafe_get() == d);
}

// Do they not point to the same things?
int DRef::operator!=(const DRef &dref)
{
	return (unsafe_get() != dref.unsafe_get());
}


DRef DRef::route(DR_Message m)
{
	if (unsafe_get())
		return unsafe_get()->route(m);
	return DR_null;
}

DR_String DRef::toString()
{
	if (isValid())
		return unsafe_get()->toString();
	return DR_String("");
}


// **************************************************************************


killer_ref::killer_ref(D* d) : DRef(d) { }

killer_ref::~killer_ref()
{
	dump();
}

void killer_ref::Recycle()
{
#ifdef PURIFY
	purify_printf("killer_ref::Recyle() deletes.\n");
#endif

	delete (unsafe_get());
	_unsafe_clear();
}

// t_ref: threshold reference.

/*
t_ref::t_ref(D* d) : DRef(d) { }

t_ref::~t_ref()
{
	dump();
}

void t_ref::dump(D* d, ref_t ref_kind)
{
#ifndef D_KEEP_INUSE
	DRef::dump(d);
#else
	D* x = unsafe_get();
	if (x == d)
		return;

	if (x)
	{
		int dr = x->decReference();
		if (dr == 1)
		{
#ifdef PURIFY
			purify_printf("t_ref::dump() to 1 means recycle.\n");
#endif
			Recycle();
		}
		else if (dr == 0)
		{
#ifdef PURIFY
			purify_printf("t_ref::dump() to 0 means *ignore*.\n");
#endif
			// ignore
		}
	}
#endif
}

*/


