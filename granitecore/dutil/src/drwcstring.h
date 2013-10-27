// drwcstring.h
// Destiny subclass of RWCString
// $Id: drwcstring.h,v 1.1 1998/11/17 23:48:47 toddm Exp $

#include <iostream.h>
#include <rw/cstring.h>
#include <rw/ctoken.h>
#include <rw/regexp.h>

#ifndef _DRWCSTRING_H_
#define _DRWCSTRING_H_

class DRWCString : public RWCString {

public:
	
	DRWCString() : RWCString() { }
	DRWCString(const char * s) : RWCString(s) { }
	DRWCString(const RWCString& s) : RWCString(s) { }
	DRWCString(const RWCSubString& ss) : RWCString(ss) { }

	int freq(const char c);
	int freq(const char *str);
	int freq(const RWCRegexp& reg);
    int freq(const DRWCString& str);
	
	int split(DRWCString res[], int maxn, const char* sep);
	int split(DRWCString res[], int maxn, const RWCRegexp&  sep);
	int split(DRWCString res[], int maxn, const DRWCString& sep);
	
	void replace(const char *what, const char *with);
	void replace(const RWCRegexp reg, const RWCString with);
	
	DRWCString after(const char *what);
	DRWCString after(const DRWCString& what);
	DRWCString after(const RWCRegexp& what);
	inline DRWCString after(int where)
	{
		if (++where < length())
			return operator()(where, length() - where);
		return "";
	}

	DRWCString from(const char *what);
	DRWCString from(const DRWCString& what);
	DRWCString from(const RWCRegexp& what);
	inline DRWCString from(int where)
	{
		if (where < length())
			return operator()(where - 1, length() - where);
		return "";
	}

	DRWCString before(const char *what);
	DRWCString before(const DRWCString& what);
	DRWCString before(const RWCRegexp& what);
	inline DRWCString before(int where)
	{
		if (++where < length())
			return operator()(0, where);
		return "";
	}

	int isNumber(void);
};

#endif

