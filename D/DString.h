// DString
// $Id: DString.h,v 1.2 1998/11/20 18:14:48 holtrf Exp $

#include "rw/cstring.h"
#include "DAtom.h"


#ifndef D_STRING_H
#define D_STRING_H

#define _D_String_ID 1024684649



class DR_String;
class DR_Class;

class DO_String : public DO_Atom {

  protected:
	RWCString s;

  public:

	// ************************
	// Constructors
	// ************************
	DO_String(RWCString str);
	DO_String() { }

	static DR_String New();
	static DR_Class Stringclass;
	DRef Class();
	
	// ************************
	// D virtuals etc.
	// ************************
	DRef route(DR_Message m);

	void init();
	void destroy();
	DR_String toString();
	BOOLEAN toBoolean() const;
	inline unsigned int dtypeid() { return _D_String_ID; }
	void assign(const DRef& obj);
	dcompare_t compare(const DRef& d) const;
	
	// ************************
	// String-specific behavior
	// ************************
	
	void prepend(DR_String drs);
	inline void prepend(RWCString& rwc) { s.prepend(rwc); }
	inline void prepend(const char *c) { if (c) s.prepend(c); }

	void append(DR_String drs);
	inline void append(RWCString& rwc) { s.append(rwc); }
	inline void append(const char *c) { if (c) s.append(c); }
	
	inline const char *data() const { return s.data(); }
	inline size_t length() const { return s.length(); }

	// friends
	friend BOOLEAN operator == (const DR_String& l, const DR_String& r);
	friend inline BOOLEAN operator != (const DR_String& l, const DR_String& r);
	friend ostream& operator<<(ostream& out, const DR_String& s);
	friend istream& operator>>(istream& in, DR_String& s);
	friend class DR_String;
	
};


class DR_String : public DR_Object {
  public:
	DR_String(D *d=0);
	DR_String(const DRef& dref);
	DR_String(const RWCString& s);
	DR_String(const char *);

	virtual ~DR_String();

	DO_String *safe_get();
	DO_String *const_get() const;
	DO_String *safe_set(D *d);
	DO_String *New();
	DO_String* operator->();
	
	// use val() and operator() only when absolutely necessary.
	RWCString& val();
	RWCString& operator()();
	RWCString const_val() const;
	
	// *********************
	// Convenience operators
	// *********************
	inline DR_String& operator=(RWCString s) {	safe_get()->s = s;	return *this;	}
	DR_String& operator=(const DR_String drs);
	DR_String& operator=(const char *s);

	inline DR_String& operator+=(RWCString s) {	safe_get()->s += s;	return *this;	}
	DR_String& operator+=(const DR_String drs);
	DR_String& operator+=(const char *s);

	DR_String& operator << (const DR_String& s);
	DR_String& operator << (const char * c);
	
	friend ostream& operator<<(ostream& out, const DR_String& s);
	friend istream& operator>>(istream& in, const DR_String& s);
};

BOOLEAN operator == (const DR_String& l, const DR_String& r);
inline BOOLEAN operator != (const DR_String& l, const DR_String& r) { return !(l==r); }

ostream& operator<<(ostream& out, const DR_String& s);
istream& operator>>(istream& in, DR_String& s);


/*
// DOC_String: the String class class
class DOC_String : public DO_Class {
	D *spawn();

  public:
	DOC_String() : DO_Class("String") { }

};
*/

/*
// "bootstrap" function for String's class
extern "C" DO_Class *Create_DOC_String()
{
	return new DOC_String();
}
*/




#endif

