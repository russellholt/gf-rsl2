// R_FileInfo.h
// $Id: R_FileInfo.h,v 1.1 1998/11/17 23:04:02 toddm Exp $
// *******************
// * System Includes *
// *******************
#include <iostream.h>
#include <rw/cstring.h>

// ******************
// * Local Includes *
// ******************
#include "res_class.h"
#include "Resource.h"
#include "R_String.h"
#include "runtime.h"

#ifndef _R_FileInfo_H_
#define _R_FileInfo_H_

#define R_FileInfo_ID 252119562

// ********************************************
// * rc_FileInfo -- the FileInfo RSL class
// ********************************************
class rc_FileInfo : public res_class {
	Resource *spawn(RWCString aname);
public:
	rc_FileInfo(RWCString aname) : res_class(aname)
	{	}
};

// *************************************************
// * R_FileInfo -- the FileInfo Resource
// *************************************************
class R_FileInfo : public Resource {
	
	// private class data
	RWCString filename;
	bool filenameSet;

protected:

	void setpathArg(const ResList& arglist);
	Resource *ResPathName() {
			return R_String::New("", filename);
		}
  
public:
	static rc_FileInfo rslType;

// Constructors
	
	R_FileInfo(RWCString n);
	
// Resource virtuals

	// Info

	unsigned int TypeID() { return R_FileInfo_ID; }
	res_class *memberOf(void) { return &rslType; }
	RWCString StrValue(void);
	int LogicalValue();
	int IsEqual(Resource *r);
	
	// Execution
	
	ResStatus execute(int method, ResList& arglist);
	void SetFromInline(RWTPtrSlist<Resource>& inliner);
	void Assign(Resource *r);
	void Clear();
	
	// output
	
	void print(ostream &out=cout);	// ECI
	void rslprint(ostream &out=cout);

// R_FileInfo specific
	
	static R_FileInfo *New(RWCString n);
	void setpath(RWCString newpath);
	inline RWCString path() { return filename; }
	bool pathIsSet() { return filenameSet; }
	bool exists(RWCString path);
	
// R_FileInfo RSL methods
	
	ResStatus rsl_setPath(const ResList& arglist);
	ResStatus rsl_path(const ResList& arglist);
	ResStatus rsl_setPathToCurrentDirectory(const ResList& arglist);
	ResStatus rsl_resolvePath(const ResList& arglist);
	ResStatus rsl_exists(const ResList& arglist);
	ResStatus rsl_doesNotExist(const ResList& arglist);
	ResStatus rsl_isDirectory(const ResList& arglist);
	ResStatus rsl_size(const ResList& arglist);
	ResStatus rsl_permissions(const ResList& arglist);
	ResStatus rsl_setPermissions(const ResList& arglist);
	ResStatus rsl_createDirectory(const ResList& arglist);
    ResStatus rsl_createDirectoryIfDNE(const ResList& arglist);
	ResStatus rsl_currentDirectory(const ResList& arglist);
	ResStatus rsl_changeDirectory(const ResList& arglist);
};

#endif



