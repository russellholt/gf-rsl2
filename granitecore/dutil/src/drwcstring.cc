// drwcstring.cc
// Destiny subclass of RWCString
// $Id: drwcstring.cc,v 1.1 1998/11/17 23:47:21 toddm Exp $
#include "drwcstring.h"

static char rcsid[] = "$Id: drwcstring.cc,v 1.1 1998/11/17 23:47:21 toddm Exp $";

// rw_freq -- char
// count the occurrences of c in s.
int DRWCString::freq(const char c)
{
    register int i=0, x=0, len=length();

	for(; i<len; i++)
		if (operator[](i) == c)
			x++;

	return x;
}

// rw_freq -- char*
// count the occurrences of c in s.
int DRWCString::freq(const char *str)
{
    register int i=0, c=0, safety = length();

	// only count the length of the string, just in case.
	for(; safety; safety--)
	{
		// index: from position i, return first occurrence of str.
		i = index(str, i);
		if (i != RW_NPOS)	// if it is found..
		{
			c++;
			i++;	// must advance or it will stay in the same place!
		}
		else
			break;
	}

	return c;
}

// rw_freq -- RWCRegexp
// count the occurrences of reg in s.
int DRWCString::freq(const RWCRegexp& reg)
{
    register int i=0, c=0, safety = length();

	// only count the length of the string, just in case.
	for(; safety; safety--)
	{
		// index: from position i, return first occurrence of str.
		i = index(reg, i);
		if (i != RW_NPOS)	// if it is found..
		{
			c++;
			i++;	// must advance or it will stay in the same place!
		}
		else
			break;
	}

	return c;
}

// rw_freq -- DRWCString
// count the occurrences of reg in s.
int DRWCString::freq(const DRWCString& str)
{
    register int i=0, c=0, safety = length();

	// only count the length of the string, just in case.
	for(; safety; safety--)
	{
		// index: from position i, return first occurrence of str.
		i = index(str, i);
		if (i != RW_NPOS)	// if it is found..
		{
			c++;
			i++;	// must advance or it will stay in the same place!
		}
		else
			break;
	}

	return c;
}

int DRWCString::split(DRWCString res[], int maxn, const char * sep)
{
    register int i=0, c=0, last_i=0, safety = length();
    int len = strlen(sep);


	// only count the length of the string, just in case.
	for(; safety; safety--)
	{
		// index: from position i, return first occurrence of str.
		i = index(sep, i);
		if (i != RW_NPOS)	// if it is found..
		{
			if (c < maxn)
			{
				res[c++] = operator()(last_i, i-last_i);
			}
			else
				break;

			// skip the found string
			i += len;	// must advance or it will stay in the same place!
			last_i = i;
		}
		else
			break;
	}

	// Get the last one
	if (c < maxn)
		res[c] = operator()(last_i, length() - last_i);

	return c+1;
}

int DRWCString::split(DRWCString res[], int maxn, const DRWCString& sep)
{
    register int i=0, c=0, last_i=0, safety = length();
    int len = sep.length();

	// only count the length of the string, just in case.
	for(; safety; safety--)
	{
		// index: from position i, return first occurrence of str.
		i = index(sep, i);
		if (i != RW_NPOS)	// if it is found..
		{
			if (c < maxn)
				res[c++] = operator()(last_i, i-last_i);
			else
				break;

			// skip the found string
			i += len;	// must advance or it will stay in the same place!
			last_i = i;
		}
		else
			break;
	}

	// Get the last one
	if (c < maxn)
		res[c] = operator()(last_i, length() - last_i);
	return c+1;
}



// Split on a regular expression. -- note: RWCRegexp, not RWCREexpr,
// so it is only simple re's, ie without { } ( ).
int DRWCString::split(DRWCString res[], int maxn, const RWCRegexp&  sep)
{
    register int i=0, c=0, last_i=0, safety = length();

	// only count the length of the string, just in case.
	for(; safety; safety--)
	{
		// from position i, return first occurrence of sep.
		RWCSubString substr = operator()(sep, i);
		i = substr.start();
		if (i != RW_NPOS)	// if it is found..
		{
			if (c < maxn)
				res[c++] = operator()(last_i, i-last_i);
			else
				break;

			i += substr.length();	// must advance or it will stay in the same place!
			last_i = i;
		}
		else
			break;
	}

	// Get the last one
	if (c < maxn)
		res[c] = operator()(last_i, length() - last_i);
	return c+1;
}


//----------------------------------

DRWCString  DRWCString::after(const char *what)
{
	int w = index(what) + strlen(what);
	return operator()(w, length() - w); 
}


DRWCString DRWCString::after(const DRWCString& what)
{
	int w = index(what) + what.length();
	return  operator()(w, length() - w); 
}

DRWCString DRWCString::after(const RWCRegexp& what)
{
	RWCSubString substr = operator()(what);
	int pos = substr.start() + substr.length();
	return operator()(pos, length() - pos); 
}


//----------------------------------

DRWCString  DRWCString::before(const char *what)
{
	int w = index(what);
	return operator()(0, w); 
}


DRWCString DRWCString::before(const DRWCString& what)
{
	int w = index(what);
	return  operator()(0, w); 
}

DRWCString DRWCString::before(const RWCRegexp& what)
{
	RWCSubString substr = operator()(what);
	int pos = substr.start();
	return operator()(0, pos); 
}

//----------------------------------

DRWCString  DRWCString::from(const char *what)
{
	int w = index(what);
	return operator()(w, length() - w); 
}


DRWCString DRWCString::from(const DRWCString& what)
{
	int w = index(what);
	return  operator()(w, length() - w); 
}

DRWCString DRWCString::from(const RWCRegexp& what)
{
	RWCSubString substr = operator()(what);
	int pos = substr.start();
	return operator()(pos, length() - pos); 
}


//----------------------------------

// replace
// like GNU's gsub, just replace every occurrence of 'what'
// with 'with'.
void DRWCString::replace(const char *what, const char *with)
{
register int i=0, i_last=0, len = length(), withlen = strlen(with);

	do
	{
		RWCSubString substr = subString(what, i) = with;

        if (substr.isNull())
            break;

		i = substr.start() + withlen;
		i_last = i;
		
	} while(i < length());	// length changes every time
}

// replace() with a regexp

void DRWCString::replace(const RWCRegexp what, const RWCString with)
{
register int i=0, i_last=0, len = length(), withlen = strlen(with);

	do
	{
		RWCSubString substr = operator()(what, i) = with;

        if (substr.isNull())
            break;

		i = substr.start() + withlen;
		i_last = i;
		
	} while(i < length());	// length changes every time
}

int DRWCString::isNumber(void)
{
    RWCRegexp RXint = "[0-9]";

	if (isNull())
        return(0);
		
	for (int i=0; i<length(); i++)
	{
	    RWCSubString subChar = operator()(i,1);
        RWCString strChar = subChar;
        RWCSubString substr = strChar(RXint);

        if (substr.isNull())
            return(0);
    }

    return(1);
}
