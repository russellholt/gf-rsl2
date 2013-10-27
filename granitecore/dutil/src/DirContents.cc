// DirContents.cc
// $Id: DirContents.cc,v 1.1 1998/11/17 23:46:54 toddm Exp $

#include <stdlib.h>
#include <iostream.h>
#include "DirContents.h"


// DirContents constructor.
// Sets the pathname of the directory, and optionally (by default)
// loads the filenames.
DirContents::DirContents(RWCString& nm, int loadEntries)
{
	dirPath = nm;
	if (loadEntries)
		loadDirectoryEntries();
}

// loadDirectoryEntries()
// Load filenames from this directory.
// `skipDotFiles' will skip the files "." and ".." (current and
// parent directory) if set to 1 (the default).
// This call will only have effect once.
int DirContents::loadDirectoryEntries(int skipDotFiles)
{
	if (dirlist.entries() > 0 || dirPath.length() == 0)
		return 0;

	DIR *thedir = opendir(dirPath);

	if (!thedir)
		return 1;

	struct dirent *de = NULL;
	RWCString filename;
	while ( (de = readdir(thedir)) != NULL)
	{
		filename = de->d_name;
		if (!skipDotFiles)
			dirlist.append(filename);
		else
			if (filename != "." && filename != "..")
				dirlist.append(filename);
	}

	closedir(thedir);
	return 0;
}


// DirList constructor
// This consturctor loads directory entries from pathstr
// using the default delimiter ":".
DirList::DirList(RWCString pathstr)
{
	loadFromPath(pathstr);
}

// findPath()
// search each directory in the path for the
// given filename. Return the actual path for
// the file. Eg, if you ask for file "a" and
// it's found in /d/e/f, the string returned
// will be "/d/e/f/a". Failure is indicated by
// a null (zero-length) string.
RWCString DirList::findPath(RWCString filename)
{
	RWTPtrSlistIterator<DirContents> iter(path);
	DirContents *dirp=NULL;
	while(iter())
	{
		dirp = iter.key();
#ifdef DEBUG
		cout << "\tSearching in `" << dirp->path() << "'\n";
#endif

		if (dirp->contains(filename))
			return (dirp->path() + RWCString("/") + filename);
	}
	return "";
}

// loadFromPath()
// given a `delim' (default ":") - separated list of directories,
// load the contents of each directory.
void DirList::loadFromPath(RWCString pathstr, RWCString delim)
{
	if (path.entries() > 0)
		return;

	RWCTokenizer next(pathstr);
	RWCString dirname;

	while(!(dirname = next(delim.data())).isNull())
	{
		DirContents *dirc = new DirContents(dirname);
		path.append(dirc);

#ifdef DEBUG
		cout << "------\ndirectory: " << dirname << endl;
		cout << (*dirc) << endl;
#endif
	}
}

