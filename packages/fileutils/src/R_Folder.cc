// *******************************************************************
// R_Folder.cc
//
// automatically generated from Folder.rsl and the template:
// $Id: R_Folder.cc,v 1.1 1998/11/17 23:03:37 toddm Exp $
// *******************************************************************

#include "R_Folder.h"
#include "R_String.h"
#include "R_List.h"
#include "slog.h"
#include "RDirectory.h"

// ********************************
// Hash codes for the RSL methods
// defined in Folder.rsl
// ********************************
#define _hLOAD 1819238756	// load
#define _hLOADRECURSIVELY 689246823	// loadRecursively
#define _hCONTENTS 100735495	// contents
#define _hFILES 359230565	// files
#define _hFOLDERS 52240228	// folders
#define _hONLYFOLDERS 1282634525    // onlyFolders
#define _hONLYFILES 1510408220  // onlyFiles
#define _hALL 6384748   // all

// ********************************
// Names of the RSL data members
// defined in Folder.rsl
// ********************************
const RWCString RFolderPathName = "name";
const RWCString RFolderContentsName = "contents";


// R_Folder static member
rc_Folder *R_Folder::rsl_type = NULL;

extern "C" res_class *Create_Folder_RC()
{
	R_Folder::rsl_type = new rc_Folder("Folder");
	return R_Folder::rsl_type;
}


// Spawn - create a new resource of this type (R_Folder)
// Called by res_class::New() if there is no object
// to pull off the free list.
Resource* rc_Folder::spawn(RWCString nm)
{
	return new R_Folder(nm);	// or other constructor
}

// New
// static convenience function
// See R_String::New(), R_Integer::New(), etc.
// If no functionality beyond res_class::New() is required
// (ie, no special values to be set conveniently), then this
// function simply eliminates the need to cast the result
// of res_class::New().
R_Folder *R_Folder::New(RWCString n /*, some value(s) */)
{
	Resource *r= R_Folder::rsl_type->New(n);
//	((R_Folder *) r)->Set( /* values (add this function if needed) */ );
	return (R_Folder *) r;
}

// R_Folder constructor
R_Folder::R_Folder(RWCString nm)
	: ResObj(nm.data(), rsl_type)
{
//	dirc = NULL;
}

R_Folder::~R_Folder()
{
	
}

// StrValue()
// Return a String version of this Resource, only if applicable.
RWCString R_Folder::StrValue(void)
{
	return ClassName();
	
}

// LogicalValue()
// Evaluate the "trueness" of this Resource (1 for TRUE, 0 for FALSE)
// Used in logical comparisons.
int R_Folder::LogicalValue()
{
	return theName.isValid();	
}

void R_Folder::Clear()
{
	// Must get rid of these references, or the
	// actual objects will still be there the next
	// time.
	theName.Set("", NULL);
	theContents.Set("", NULL);
	
	useInternalPath = FALSE;
	internalFullPathName = "";
	
 	foldersFilesOrAll = all_files;

	ResObj::Clear();
}

// reset
// clear the datamembers, ie the name and the contents,
// but don't get rid of them..
void R_Folder::reset(void)
{
	theName.Clear();
	theContents.Clear();
	useInternalPath = FALSE;
	internalFullPathName = "";
}

void R_Folder::setTheName()
{
	theName = GetDataMember(RFolderPathName);
}

void R_Folder::setTheContents()
{
	theContents = GetDataMember(RFolderContentsName);
}

void R_Folder::add(Resource *r)
{
	if (!r) {
		cerr << "Folder.add(): null argument.\n";
		return;
	}

	if (!theContents.isValid())
		setTheContents();

	if (theContents.isValid() && theContents.HierarchyContains(R_List_ID))
		((R_List *) theContents())->append(r);
	else
		cerr << "Folder.add(): bogus!\n";
}

RWCString R_Folder::pathName()
{
	if (!theName.isValid())
		setTheName();

	if (useInternalPath)
		return internalFullPathName;

	return (GetDataMember(RFolderPathName)).StrValue();
}

void R_Folder::setPathName(RWCString fname)
{
	if (!theName.isValid())
		setTheName();

	((R_String *) theName())->Set(fname);
}

// *************************************************************
// execute()
// This is the interface between RSL and C++ Resources.
//
// Automatically generated from "Folder.rsl"
// DO NOT MODIFY !
// *************************************************************

ResStatus R_Folder::execute(int method, ResList& arglist)
{
	switch(method)
	{
		case _hLOAD:	// "load"
			return rsl_load(arglist);

		case _hLOADRECURSIVELY:	// "loadRecursively"
			return rsl_loadRecursively(arglist);

		case _hCONTENTS:	// "contents"
			return rsl_contents(arglist);

        case _hONLYFOLDERS: // "onlyFolders"
            return rsl_onlyFolders(arglist);
 
        case _hONLYFILES:   // "onlyFiles"
            return rsl_onlyFiles(arglist);
 
        case _hALL: // "all"
            return rsl_all(arglist);
 
		case _hFILES:	// "files"
			return rsl_files(arglist);

		case _hFOLDERS:	// "folders"
			return rsl_folders(arglist);

		default: ;
	}

	return ResStatus(ResStatus::rslFail);
}

// RSL method "load"
//	load(String fromPath);
ResStatus R_Folder::rsl_load(const ResList& arglist)
{
	reset();
	setTheName();
	setTheContents();
	setPathName(arglist[0].StrValue());	// well..

	// load directory entries using all defaults:
	// 		theFolder = this
	//   	skipdotfiles = TRUE (default)
	//  	recursive = FALSE (default)
	RDirectory rdc(this);
	
	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "loadRecursively"
//	loadRecursively(String fromPath);
ResStatus R_Folder::rsl_loadRecursively(const ResList& arglist)
{
	reset();
	setPathName(arglist[0].StrValue());	// well..

	RDirectory rdc;

	// load directory entries using these settings:
	// 		theFolder = this	(first arg)
	//   	skipdotfiles = TRUE	(second arg)
	//  	recursive = TRUE (third arg)
	//		onlyDirectories = whether foldersFilesOrAll is set to only_folders
	//		   (at this time, no ability to do files only)
	rdc.loadDirectoryEntries(this, TRUE, TRUE, (foldersFilesOrAll==only_folders));
	
	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "contents"
//	List contents();
ResStatus R_Folder::rsl_contents(const ResList& arglist)
{
	ResReference rf = GetDataMember("contents");

	return ResStatus(ResStatus::rslOk, rf());
}

// RSL method "files"
//	List files();
ResStatus R_Folder::rsl_files(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "folders"
//	List folders();
ResStatus R_Folder::rsl_folders(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk, NULL);
}
 
// RSL method "onlyFolders"
ResStatus R_Folder::rsl_onlyFolders(const ResList& arglist)
{
	foldersFilesOrAll = only_folders;
    return ResStatus(ResStatus::rslOk, NULL);
}
 
// RSL method "onlyFiles"
ResStatus R_Folder::rsl_onlyFiles(const ResList& arglist)
{
	foldersFilesOrAll = only_files;
    return ResStatus(ResStatus::rslOk, NULL);
}
 
// RSL method "all"
ResStatus R_Folder::rsl_all(const ResList& arglist)
{
 	foldersFilesOrAll = all_files;
   return ResStatus(ResStatus::rslOk, NULL);
}
 
