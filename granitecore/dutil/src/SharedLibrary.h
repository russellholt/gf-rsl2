// SharedLibrary.h
// C++ encapsulation of Unix shared library functions
//
// $Id: SharedLibrary.h,v 1.1 1998/11/17 23:48:35 toddm Exp $
// March 27, 1997 * Russell Holt
// Copyright (c) 1997 Destiny Software Corporation

#include <iostream.h>
#include <dlfcn.h>
#include <rw/cstring.h>

#ifndef DSC_SHARED_LIB_H_
#define DSC_SHARED_LIB_H_

class SharedLibrary {
	RWCString libName, libPath, real_libPath;
	int libIsOpen;
	void *dlHandle;
	int hasError;
	RWCString last_dlError;
	int closeOnDestruct;
	
	void Init();

public:

	SharedLibrary(RWCString nm, RWCString path, int closeOnDestr=1);
	SharedLibrary();
	~SharedLibrary();
	
	int open(int flag=RTLD_LAZY);
	void close();
	void set_realPath(void *some_symbol);

	void *getSymbol(const char *symbol);
	
	void Set(RWCString name, RWCString path);
	inline RWCString name() { return libName; }
	inline RWCString path() { return libPath; }
	inline RWCString realPath() { return real_libPath; }
	inline int isOpen() { return libIsOpen; }
	inline void *handle() { return dlHandle; }
	inline int error() { return hasError; }
	inline RWCString lastError() { return last_dlError; }
};

#endif


