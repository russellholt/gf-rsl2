// Classes for directory search paths and contents
// Class DirContents represents a collection of files in a directory.
// class DirList is a directory search path, that is an ordered list
// of DirContents objects.
//
// Russell Holt, March 30, 1997 (yes, Easter!)
// $Id: DirContents.h,v 1.1 1998/11/17 23:48:11 toddm Exp $

#include <dirent.h>
#include <rw/cstring.h>
#include <rw/ctoken.h>
#include <rw/tvslist.h>
#include <rw/tpslist.h>
#include "rw_utils.h"

#ifndef _DIRCONTENTS_H_
#define _DIRCONTENTS_H_

class DirContents {
	RWCString dirPath;
	RWTValSlist<RWCString> dirlist;

public:
	DirContents(RWCString& nm, int loadEntries=1);
	int loadDirectoryEntries(int skipDotFiles=1);

	RWCString path() { return dirPath; }
	int entries() { return dirlist.entries(); }
	RWTValSlist<RWCString>& getList() { return dirlist; }
	
	int contains(RWCString fname)
	{
		return (dirlist.contains(fname));
	}

	friend ostream& operator<<(ostream& out, DirContents& dirc);
};

inline ostream& operator<<(ostream& out, DirContents& dirc)
{
//	out << dirc.path() << ":\n";
//	out << dirc.getList();	// in Destiny's rw_utils.h

        // in Destiny's rw_utils.h
	return (out << dirc.path() << ":\n" << dirc.getList());	
}

class DirList {
public:
	RWTPtrSlist<DirContents> path;

	DirList() { }
	DirList(RWCString pathstr);
	void loadFromPath(RWCString pathstr, RWCString delim=":");
	RWCString findPath(RWCString filename);
};

#endif




