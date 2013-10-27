#include "DString.h"

#include "DClass.h"

#include <iostream.h>

DR_String::DR_String(D *d) : DR_Object(d)
{ }
 
DR_String::DR_String(const DRef& ref) : DR_Object(ref)
{ }

DR_String::DR_String(const RWCString& s)
{
	New();
	operator=(s);
}

DR_String::DR_String(const char *c)
{
	New();
	operator=(c);
}

DR_String::~DR_String()
{
#ifdef DMEMORY
	cerr << "\t~DR_String()\n";
#endif
}

RWCString& DR_String::val() { return safe_get()->s; }
RWCString& DR_String::operator()() { return safe_get()->s; }


RWCString DR_String::const_val() const
{
	DO_String *dos = const_get();

	if (dos)
		return dos->s;

	return "";
}

DO_String* DR_String::operator->()
{
	return safe_get();
}

DO_String *DR_String::safe_get()
{
	if (!unsafe_get())
		return New();
	DO_String *x = dynamic_cast<DO_String *> (unsafe_get());
	if (x)
		return x;
	return New();
}

DO_String *DR_String::safe_set(D *d)
{
	DO_String *ds = dynamic_cast<DO_String *> (d);
	replace(ds);
	return ds;
}

DO_String *DR_String::const_get() const
{
	return dynamic_cast<DO_String *>(unsafe_get());
}

DR_String& DR_String::operator=(const char *s)
{
	if (s)
		safe_get()->s = s;
	return *this;
}

// I know this can be more efficient.
void DO_String::assign(const DRef& obj)
{
	if (obj.isValid())
		s = DR_String(obj).val();
}

// =
// Even though the argument is not declared const,
// we want to avoid modifying it so we use const_get()
// instead of drs->s, which invokes safe_get()
DR_String& DR_String::operator=(const DR_String drs)
{
	DO_String *dos = drs.const_get();
	if (drs.isValid())
		safe_get()->s = dos->s;

	return *this;
}

DR_String& DR_String::operator+=(const char *s)
{
	if (s)
		safe_get()->s += s;
	return *this;
}

// +=
// Even though the argument is not declared const,
// we want to avoid modifying it so we use const_get()
// instead of drs->s, which invokes safe_get()
DR_String& DR_String::operator+=(const DR_String drs)
{
	DO_String *dos = drs.const_get();
	if (dos)
		safe_get()->s += dos->s;

	return *this;
}

DR_String& DR_String::operator << (const DR_String& s)
{
	return this->operator+=(s);
}

DR_String& DR_String::operator << (const char * c)
{
	return this->operator+=(c);
}


BOOLEAN operator == (const DR_String& l, const DR_String& r)
{
	if (l.unsafe_get() == r.unsafe_get())
		return DR_TRUE; // even if they are both null.
	
	DO_String *pl, *pr;

	// const_get() returns a dynamic_cast, so
	// return false if either object is NULL or not a DO_String.
	if (!(pl=l.const_get()) || !(pr=l.const_get()))
		return DR_FALSE;
		
	// finally, invoke Rogue Wave comparison
	return (pl->s == pr->s);
}


DO_String *DR_String::New()
{
	DO_NEW(DO_String);	// D_macros.h
/*
	DO_String *dobj = DO_String::New().const_get();
	replace(dobj);

	return dobj;
*/
}

ostream& operator<<(ostream& out, const DR_String& s)
{
	// want to maintain the const-ness of s,
	// and since safe_get(), called by the operator->()
	// will create a DO_String if s points to null,
	// we must use other means.
	DO_String *dx;
	if ((dx=s.const_get()))
		out << dx->s;

	return out;
}

istream& operator>>(istream& in, DR_String& s)
{
	// if s is null, its DO_String implementation will be created.
	in >> s->s;
	return in;
}

// ****************************************************

DO_String::DO_String(RWCString str) : s(str)
{

}

BOOLEAN DO_String::toBoolean() const
{
	return (s.length() > 0) ;
}

DR_String DO_String::toString()
{
	return DR_String(this);
}

void DO_String::prepend(DR_String drs)
{
	s.prepend(drs->s);
}


DRef DO_String::route(DR_Message m)
{
	cout << "\tDO_String::route()\n";

	cout << "\t\tmessage: `" << m->message << "'\n";
	
	// for now!
	return DRef(this);
}

void DO_String::init()
{
	DO_Atom::init();
	s = "";
#ifdef DMEMORY
	cerr << "\tDO_String::init()\n";
#endif
}

void DO_String::destroy()
{
#ifdef DMEMORY
	cerr << "\tDO_String::destroy()\n";
#endif

	s = "";
	DO_Atom::destroy();
}

dcompare_t DO_String::compare(const DRef& d) const
{
	// The Rogue Wave documentation says that RWCString::collate() is
	// not efficient when the same strings are to be compared
	// many times because it uses scoll(). It recommends that
	// strXForm(RWCString&) be used instead. We would have to
	// *store* the results of  StrXForm() to be useful, and use
	// a dirty bit so as to not interfere with the rest of the
	// string operations ...

	if (!d.isValid())
		return c_greater;

	DR_String drs = DR_String(d);
	int scoll = s.collate(drs.val());

	//cerr << "STRING: self(" << s << ").compare(" << drs << ") == " << scoll << ".\n";

	// scoll being negative means *self* is less than the argument
	return scoll < 0? c_less : (scoll == 0? c_equal : c_greater);
}

// ************************************************************

// DOC_String: the String class class
class DOC_String : public DO_Class {
	D *spawn();

  public:
	DOC_String() : DO_Class("String") { }
	void Recycle(D* d) {
		DO_Class::Recycle(d);
	}

};


DR_Class DO_String::Stringclass = new DOC_String();

D *DOC_String::spawn() {
	return new DO_String();
}

DRef DO_String::Class()
{
#ifdef DMEMORY
	if (Stringclass.isValid())
		cerr << "Stringclass: refcount " << Stringclass->refCount() << endl;
	else
		cerr << "Stringclass: not valid!\n";
#endif
	return Stringclass;
}

DR_String DO_String::New()
{
	return DO_String::Stringclass->New();
}

