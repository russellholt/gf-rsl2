// ******************************************************************
// R_Folder.h
//
// automatically generated from Folder.rsl and the template:
// $Id: R_Folder.h,v 1.1 1998/11/17 23:04:09 toddm Exp $
// ******************************************************************
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
#include "runtime.h"
#include "destiny.h"

#ifndef _R_Folder_H_
#define _R_Folder_H_

#define R_Folder_ID 589130852

// ********************************************
// * rc_Folder -- the Folder RSL class
// ********************************************
class rc_Folder : public res_class {
	Resource *spawn(RWCString aname);
public:
	rc_Folder(RWCString aname) : res_class(aname)
	{	}
};

// *************************************************
// * R_Folder -- the Folder Resource
// *************************************************
class R_Folder : public ResObj {
/* 	DirContents *dirc; */
  public:
	enum file_kinds_t { all_files=0, only_folders, only_files };

  private:
	
	ResReference theName, theContents;

	RWCString internalFullPathName;
	bool useInternalPath;
	
	file_kinds_t foldersFilesOrAll;

  protected:
	void setTheName();
	void setTheContents();

  public:
	static rc_Folder *rsl_type;

// Constructors
	R_Folder(RWCString n);
	virtual ~R_Folder();
	
// Resource virtuals
	// Info
	unsigned int TypeID() { return R_Folder_ID; }
	res_class *memberOf(void) { return rsl_type; }
	RWCString StrValue(void);
	int LogicalValue();
	void Clear();
	
	// Execution
	
	ResStatus execute(int method, ResList& arglist);

// R_Folder specific
	static R_Folder *New(RWCString n);
	virtual void add(Resource *r);
	virtual RWCString pathName();
	virtual void setPathName(RWCString fname);
	inline void setFullPathName(RWCString fname)
	{	internalFullPathName = fname;
		useInternalPath = TRUE;	}

	void reset();

// R_Folder RSL methods
	ResStatus rsl_load(const ResList& arglist);
	ResStatus rsl_loadRecursively(const ResList& arglist);
	ResStatus rsl_contents(const ResList& arglist);
	ResStatus rsl_files(const ResList& arglist);
	ResStatus rsl_folders(const ResList& arglist);
    ResStatus rsl_onlyFolders(const ResList& arglist);
    ResStatus rsl_onlyFiles(const ResList& arglist);
    ResStatus rsl_all(const ResList& arglist);
	
	friend class rc_Folder;
};

#endif



