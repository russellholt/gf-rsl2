// *******************
// * System Includes *
// *******************
#include <dirent.h>
#include <rw/cstring.h>
#include <rw/ctoken.h>
#include <rw/tvslist.h>
#include <rw/tpslist.h>

// ******************
// * Local Includes *
// ******************
#include "rw_utils.h"
#include "R_Folder.h"
#include "destiny.h"

#ifndef _RDIRCONTENTS_H_
#define _RDIRCONTENTS_H_

// Very much like DirContents
// But produces R_Folders for directories and R_Strings for filenames.

// a subclass can change this to be any Resource.
// RFH 2/27/1998.
class RDirectory {

  public:
	RDirectory();
	RDirectory(R_Folder *theFolder, bool loadEntries=TRUE);
	virtual ~RDirectory();

	virtual bool loadDirectoryEntries(R_Folder *theFolder,
		bool skipDotFiles=TRUE, bool brecursive=FALSE,
		bool bOnlyDirectories=FALSE);
	
  protected:
	virtual void addToContainer(R_Folder *theFolder, dirent *de,
		bool skipDotFiles=TRUE, bool brecursive=FALSE,
		bool bOnlyDirectories=FALSE);
};

#endif



