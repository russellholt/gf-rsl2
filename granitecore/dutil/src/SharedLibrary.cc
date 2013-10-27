// SharedLibrary.cc
// C++ encapsulation of Unix shared library functions
//
// $Id: SharedLibrary.cc,v 1.1 1998/11/17 23:47:15 toddm Exp $
// March 27, 1997 * Russell Holt
// Copyright (c) 1997 Destiny Software Corporation

#include "SharedLibrary.h"
#include <fstream.h>

#ifdef RSLERR
extern ofstream rslerr;
#endif

static char rcsid[] = "$Id: SharedLibrary.cc,v 1.1 1998/11/17 23:47:15 toddm Exp $";

// Construct SharedLibrary
// nm is the unqualified name of the library, such as "finance".
// path can have path/version info, like finance.so or
// even /usr/local/lib/libfinance.so.1.9
// path is used to actually open the library, nm is used
// for reference only.
SharedLibrary::SharedLibrary(RWCString nm, RWCString path, int closeOnDestr)
{
	Init();
	Set(nm, path);
	closeOnDestruct = closeOnDestr;
}

SharedLibrary::SharedLibrary()
{
	Init();
}

SharedLibrary::~SharedLibrary()
{
	if (libIsOpen && closeOnDestruct)
		dlclose(dlHandle);
}

void SharedLibrary::Init()
{
	hasError = 0;
	libIsOpen = 0;
	dlHandle = NULL;
	closeOnDestruct=1;
}

void SharedLibrary::Set(RWCString name, RWCString path)
{
	close();
	libName = name;
	libPath = path;	
}

// open()
// Open the shared lib.
// returns the value of hasError and sets
// the last error string with dlerror()
int SharedLibrary::open(int flag)
{
	if (libPath.length() <= 0)	// open from main program
	{
#ifdef RSLERR
		rslerr << "calling dlopen() on self: arg is char* to zero\n";
#endif
		dlHandle = dlopen((char *) 0, flag);
	}
	else
	{
#ifdef RSLERR
		rslerr << "calling dlopen() on `" << libPath << "'\n";
#endif
		dlHandle = dlopen(libPath.data(), flag);
	}

	if (!dlHandle)
	{
		last_dlError = dlerror();
		libIsOpen = 0;
		hasError = 1;
	}
	else
	{
		libIsOpen = 1;
		hasError = 0;
		last_dlError = "";		
	}

	return hasError;
}

// set_realPath
// assumes lib is already open, etc.
// Uses dladdr() to find out info about the library,
// particularly where it was actually loaded from disk.
void SharedLibrary::set_realPath(void *some_symbol)
{
/*
#ifndef sgi
	Dl_info libinfo;

	int dladdr_result = dladdr(some_symbol, &libinfo);

	if (dladdr_result == 0)
		real_libPath = "<self>";
	else
		real_libPath = ((libinfo.dli_fname)? (libinfo.dli_fname) : "<self>");
#else
    real_libPath = "feature not available";
#endif
*/
}

void SharedLibrary::close()
{
	if (libIsOpen)
		dlclose(dlHandle);

	libIsOpen = 0;
	dlHandle = NULL;
}

// getSymbol()
// Look up the named symbol and return it.
// The symbol itself may actually be NULL, so
// you should check the value of eror() if that
// is what you could expect from your symbol.
void *SharedLibrary::getSymbol(const char *symbol)
{
	hasError=0;
	last_dlError = "";

	if (libIsOpen)
	{
		void *sym = dlsym(dlHandle, symbol);
//		if (sym == NULL)
//		{
			char *err = dlerror();
			if (err)
			{
				last_dlError = err;
				hasError =(last_dlError.length() > 0);
			}
//		}

		real_libPath = "(feature not available)";

		/*
		if (real_libPath.length() <= 0)
			set_realPath(sym);	// locates file from which sym came
		*/

		return sym;
	}

	hasError = 1;
	last_dlError = "shared lib not opened.";
	return NULL;
}
