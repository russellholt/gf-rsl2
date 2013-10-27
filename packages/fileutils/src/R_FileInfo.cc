// R_FileInfo.cc
// $Id: R_FileInfo.cc,v 1.1 1998/11/17 23:03:32 toddm Exp $

// system header files
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/param.h>
#include <errno.h>
#include <unistd.h>	// for chdir

// destiny header files
#include "R_FileInfo.h"
#include "R_Boolean.h"
#include "R_String.h"
#include "destiny.h"
#include "slog.h"

#define _hSETPATH 303111248	// setPath
#define _hPATH 1885434984	// path
#define _hSETPATHTOCURRENTDIRECTORY 1361713685	// setPathToCurrentDirectory
#define _hRESOLVEPATH 2137488959    // resolvePath
#define _hEXISTS 285960563	// exists
#define _hDOESNOTEXIST 1382638146	// doesNotExist
#define _hISDIRECTORY 1952734749	// isDirectory
#define _hSIZE 1936292453	// size
#define _hPERMISSIONS 1987604996	// permissions
#define _hSETPERMISSIONS 186085462  // setPermissions
#define _hCREATEDIRECTORY 167787388	// createDirectory
#define _hCREATEDIRECTORYIFDNE 1816425840	// createDirectoryIfDNE
#define _hCURRENTDIRECTORY 453382444	// currentDirectory
#define _hCHANGEDIRECTORY 421150579	// changeDirectory


// R_FileInfo static member
rc_FileInfo R_FileInfo::rslType("FileInfo");

extern "C" res_class *Create_FileInfo_RC()
{
	return &(R_FileInfo::rslType);
}


// Spawn - create a new resource of this type (R_FileInfo)
// Called by res_class::New() if there is no object
// to pull off the free list.
Resource* rc_FileInfo::spawn(RWCString nm)
{
	return new R_FileInfo(nm);	// or other constructor
}

// New
// static convenience function
// See R_String::New(), R_Integer::New(), etc.
// If no functionality beyond res_class::New() is required
// (ie, no special values to be set conveniently), then this
// function simply eliminates the need to cast the result
// of res_class::New().
R_FileInfo *R_FileInfo::New(RWCString n)
{
	Resource *r= R_FileInfo::rslType.New(n);
//	((R_FileInfo *) r)->Set( /* values (add this function if needed) */ );
	return (R_FileInfo *) r;
}

// R_FileInfo constructor
R_FileInfo::R_FileInfo(RWCString nm)
	: Resource(nm)
{
	filenameSet = FALSE;
}

// StrValue()
// Return a String version of this Resource, only if applicable.
RWCString R_FileInfo::StrValue(void)
{
	return filename;
}

// LogicalValue()
// Evaluate the "trueness" of this Resource (1 for TRUE, 0 for FALSE)
// Used in logical comparisons.
int R_FileInfo::LogicalValue()
{
	return filenameSet;
}

// IsEqual()
// "equality" is by pathname
int R_FileInfo::IsEqual(Resource *r)
{
	if (r && r->TypeID() == R_FileInfo_ID)
		return ( ((R_FileInfo *) r)->path() == filename );

	return 0;
}

// SetFromInline
// Given a list of Resources, match with a data member of the same
// name and assign. eg, in RSL, "myclass { a:1, b:2, /* etc */ }"
// an object of type `myclass' is created and SetFromInline() is called
// for the list of resources enclosed in { }.
// (ResStructure provides a default version)
void R_FileInfo::SetFromInline(RWTPtrSlist<Resource>& inliner)
{
	/* modify this */
}

// Assign
// set this resource equal to r.
// (ResStructure provides a default version)
void R_FileInfo::Assign(Resource *r)
{
	if (r && r->TypeID() == R_FileInfo_ID)
		setpath( ((R_FileInfo *) r)->path() );
}

// Clear()
// Memory management - called to restore an object
// to a "just created" state, for free-list management.
// (ResStructure provides a default version)
void R_FileInfo::Clear()
{
	filename = "";
	filenameSet = FALSE;
}

// print()
// ECI syntax
// (ResStructure provides a default version)
void R_FileInfo::print(ostream &out)
{
	out << "FileInfo { path: \"" << filename << "\"}";
}

// rslprint()
// Printing from within RSL
// (ResStructure provides a default version)
void R_FileInfo::rslprint(ostream &out)
{
	out << filename;
}

void R_FileInfo::setpath(RWCString newpath)
{
	if (newpath.length() > 0)
	{
		filename = newpath;
		filenameSet = TRUE;	
	}
}

bool R_FileInfo::exists(RWCString path)
{
	struct stat stat_buf;
	int err = stat(filename, &stat_buf);

	if (err == EACCES)
	{
		cerr << "FileInfo: Access denied for a component of `"
			<< path << "'\n";
		logf->error(LOGRSL) << "FileInfo: Access denied for a component of `"
			<< path << "'" << endline;
	}

	cerr << "R_FileInfo::exists(): sizeof(stat_buf) == "
		 << sizeof(stat_buf) << endl;
	
	return (0 == err);
}

void R_FileInfo::setpathArg(const ResList& arglist)
{
	if (arglist.entries() > 0)
		setpath(arglist[0].StrValue());
}

ResStatus R_FileInfo::execute(int method, ResList& arglist)
{
	switch(method)
	{
		case _hSETPATH:	// "setPath"
			return rsl_setPath(arglist);

		case _hPATH:	// "path"
			return rsl_path(arglist);

		case _hSETPATHTOCURRENTDIRECTORY:	// "setPathToCurrentDirectory"
			return rsl_setPathToCurrentDirectory(arglist);

		case _hEXISTS:	// "exists"
			return rsl_exists(arglist);

        case _hRESOLVEPATH: // "resolvePath"
            return rsl_resolvePath(arglist);
 
		case _hDOESNOTEXIST:	// "doesNotExist"
			return rsl_doesNotExist(arglist);

		case _hISDIRECTORY:	// "isDirectory"
			return rsl_isDirectory(arglist);

		case _hSIZE:	// "size"
			return rsl_size(arglist);

		case _hPERMISSIONS:	// "permissions"
			return rsl_permissions(arglist);

        case _hSETPERMISSIONS:  // "setPermissions"
            return rsl_setPermissions(arglist);
 
		case _hCREATEDIRECTORY:	// "createDirectory"
			return rsl_createDirectory(arglist);

		case _hCREATEDIRECTORYIFDNE:	// "createDirectoryIfDNE"
			return rsl_createDirectoryIfDNE(arglist);

		case _hCURRENTDIRECTORY:	// "currentDirectory"
			return rsl_currentDirectory(arglist);

		case _hCHANGEDIRECTORY:	// "changeDirectory"
			return rsl_changeDirectory(arglist);

		default: ;
	}

	return ResStatus(ResStatus::rslFail);
}

// RSL method "setPath"
ResStatus R_FileInfo::rsl_setPath(const ResList& arglist)
{
	setpath(arglist[0].StrValue());
	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "path"
ResStatus R_FileInfo::rsl_path(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_String::New("", filename)
	);
}

// RSL method "setPathToCurrentDirectory"
ResStatus R_FileInfo::rsl_setPathToCurrentDirectory(const ResList& arglist)
{
	char resolved_name[MAXPATHLEN];	

	//	char *realpath(char *file_name, char *resolved_name);
	realpath(".", resolved_name);
	filename = resolved_name;

	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "resolvePath"
ResStatus R_FileInfo::rsl_resolvePath(const ResList& arglist)
{
	setpathArg(arglist);
	if (pathIsSet())
	{
		char resolved_name[MAXPATHLEN];	

		//	char *realpath(char *file_name, char *resolved_name);
		realpath((char *) filename.data(), resolved_name);
		setpath(resolved_name);
	}

    return ResStatus(ResStatus::rslOk, ResPathName());
}

// RSL method "exists"
//	Boolean exists();
//	Boolean exists(String newPath);
ResStatus R_FileInfo::rsl_exists(const ResList& arglist)
{
	setpathArg(arglist);

	return ResStatus(ResStatus::rslOk,	
			R_Boolean::New("", exists((arglist[0].StrValue()))?1:0 )
		);
}

// RSL method "doesNotExist"
ResStatus R_FileInfo::rsl_doesNotExist(const ResList& arglist)
{
	setpathArg(arglist);

	return ResStatus(ResStatus::rslOk,	
			R_Boolean::New("", !exists(arglist[0].StrValue()) )
		);
}

// RSL method "isDirectory"
ResStatus R_FileInfo::rsl_isDirectory(const ResList& arglist)
{
//	struct stat stat_buf;
//	int err = stat(filename, &stat_buf);
//
//	if (
// From mknod(2)
//        S_IFIFO   fifo special
//        S_IFCHR   character special
//        S_IFDIR   directory
//        S_IFBLK   block special
//        S_IFREG   ordinary file
	
	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "size"
ResStatus R_FileInfo::rsl_size(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "permissions"
ResStatus R_FileInfo::rsl_permissions(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk, NULL);
}
 
// RSL method "setPermissions"
ResStatus R_FileInfo::rsl_setPermissions(const ResList& arglist)
{
    return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "createDirectory"
// creates a new directory if it doesn't already exists.
ResStatus R_FileInfo::rsl_createDirectory(const ResList& arglist)
{
	R_Boolean *rb = R_Boolean::New("mkdir", 0);

	// There may be an argument. If so, it is the name of a new
	// path to set.
	setpathArg(arglist);

	if (pathIsSet())
	{
#ifndef PRODUCTION
		cerr << "TEST version of R_FileInfo: rwxr-xr-x directory permissions.\n";
		logf->notice(LOGRSL)
			<< "TEST version of R_FileInfo: rwxr-xr-x directory permissions."
			<< endline;
#endif
		// ******************************************
		// Set R,W,X permissions for owner only.
		// int mkdir(const char *path, mode_t mode);
		// ******************************************
		if (0 == mkdir(filename.data(), S_IRUSR | S_IWUSR | S_IXUSR
		
// **************************************************
#ifndef PRODUCTION
			| S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
#endif
// **************************************************
	
				))
			rb->Set(1);
		else
		if ( errno == EEXIST)	// directory already exists: OK.
			rb->Set(1);
		else
			perror("FileInfo::createDirectory() -- mkdir()");
	}
	else
		Logf.error(LOGRSL)
			<< "FileInfo::createDirectory(): no path specified." << endline;
	
	return ResStatus(ResStatus::rslOk, rb);

// File modes, from the man page for chmod(2):
//               S_ISUID   04000   Set user ID on execution.
//               S_ISGID   020#0   Set group ID on execution
//                                 if # is 7, 5, 3, or 1.
//                                 Enable mandatory file/record locking
//                                 if # is 6, 4, 2, or 0.
//               S_ISVTX   01000   Save text image  after execution.
//               S_IRWXU   00700   Read, write, execute by owner.
//               S_IRUSR   00400   Read by owner.
//               S_IWUSR   00200   Write by owner.
//               S_IXUSR   00100   Execute (search if a directory) by owner.
//               S_IRWXG   00070   Read, write, execute by group.
//               S_IRGRP   00040   Read by group.
//               S_IWGRP   00020   Write by group.
//               S_IXGRP   00010   Execute by group.
//               S_IRWXO   00007   Read, write, execute (search) by others.
//               S_IROTH   00004   Read by others.
//               S_IWOTH   00002   Write by others.
//               S_IXOTH   00001   Execute by others.
}


// RSL method "createDirectoryIfDNE"
ResStatus R_FileInfo::rsl_createDirectoryIfDNE(const ResList& arglist)
{
	return rsl_createDirectory(arglist);
}


// RSL method "currentDirectory"
ResStatus R_FileInfo::rsl_currentDirectory(const ResList& arglist)
{
	char resolved_name[MAXPATHLEN];	

	//	char *realpath(char *file_name, char *resolved_name);
	realpath(".", resolved_name);

	return ResStatus(ResStatus::rslOk, R_String::New("", resolved_name));
}

// RSL method "changeDirectory"
// if there is an argument, use that as the path but don't set
// the filename. otherwise, use filename.
ResStatus R_FileInfo::rsl_changeDirectory(const ResList& arglist)
{
	if (arglist.entries() == 0)
	{
		// use path
		cout << "Changing to directory `" << filename << "'\n";
		chdir(filename.data());
	}
	else
	{
		cout << "Changing to directory `" << arglist[0].StrValue() << "'\n";
		chdir(arglist[0].StrValue().data());
	}

	return ResStatus(ResStatus::rslOk, NULL);
}
