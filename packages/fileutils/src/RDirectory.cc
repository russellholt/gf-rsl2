// RDirectory
// 
// a class for recursively reading on-disk directories
// and creating resources (R_Folder and R_String) to model them.
// 
// An enhancement might be to create R_FileInfo objects instead
// of R_Strings for files. maybe.
//
// $Id: RDirectory.cc,v 1.1 1998/11/17 23:03:26 toddm Exp $
// russell, 27 feb 1998
// Copyright (c) 1998 Destiny Software Corporation

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "RDirectory.h"
#include "R_String.h"

RDirectory::RDirectory()
{
	
}

RDirectory::RDirectory(R_Folder *theFolder, bool loadEntries)
{
	if (loadEntries)	// use defaults on skipDotFiles and recursive
		loadDirectoryEntries(theFolder);
}

RDirectory::~RDirectory()
{
	
}

// loadDirectoryEntries
//    Based on DirContents::loadDirectoryEntries
// returns whether there were any errors.
//
// skipDotFiles means that files either "." or ".." will be skipped or not,
//     not files beginning with '.'
//
// brecursive means whether to recurse on subdirectories or not
//   (addToContainer calls back here; mutual recursion)
bool RDirectory::loadDirectoryEntries(R_Folder *theFolder,
	bool skipDotFiles, bool brecursive, bool bOnlyDirectories)
{
	if (!theFolder)
	{
		cerr << "RDirectory: Null Folder\n";
		return TRUE;
	}
	// "open" the directory
	DIR *thedir = opendir(theFolder->pathName());

	if (!thedir) {
		cerr << "invalid directory `" << theFolder->pathName() << "'\n";
		return TRUE;
	}

	// Walk through it and add each one to the folder
	struct dirent *de = NULL;
	while ( (de = readdir(thedir)) != NULL)
	{
		if (de)
			addToContainer(theFolder, de, skipDotFiles, brecursive, bOnlyDirectories);
		else
			cerr << "\t(invalid dirent)\n";
	}

	closedir(thedir);

	return FALSE;
}


// addToContainer
// virtual function called by loadDirectoryEntries.
// subclasses can conveniently change this functionality.
//
// if brecursive is set, we'll call loadDirectoryEntries with
// a new R_Folder object.
void RDirectory::addToContainer(R_Folder *theFolder, dirent *de,
	bool skipDotFiles, bool brecursive, bool bOnlyDirectories)
{
	RWCString filename = (de->d_name==NULL? "": de->d_name);

	if (skipDotFiles &&  (filename == "." || filename == ".."))
		return;

	RWCString pathname = theFolder->pathName() + "/" + filename;
	struct stat stat_buf;	// probably inefficient to re-allocate for every file.

	if (0 != stat(pathname.data(), &stat_buf))
	{
		// unable to get stat info... add as string anyway.
		cerr << "unable to get stat info for `" << pathname << "'\n";
		return;
	}

	// If the file is a directory, create a new R_Folder and add it to theFolder.
	// then, if brecursive, call loadDirectoryEntries on that one! yow!
	if (S_ISDIR(stat_buf.st_mode))	// is a directory?
	{
		// create folder
		R_Folder *rf = R_Folder::New("");
		
		// pathName, the one that is printed, will be just the file name.
		// fullpathname is the full path to the actual on-disk directory.
		rf->setFullPathName(pathname);
		rf->setPathName(filename);

		// add it to the enclosing folder
		theFolder->add(rf);

		// recurse
		if (brecursive)
			loadDirectoryEntries(rf, skipDotFiles, brecursive, bOnlyDirectories);
	}
	else
	if (!bOnlyDirectories)	// if not just directories, add files.
		theFolder->add(R_String::New("", filename));

}

