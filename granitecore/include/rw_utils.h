// rw_utils.h
// Random utility functions, mostly having to do with
// RogueWave library things.

#include <iostream.h>
#include <fstream.h>
#include <rw/tvslist.h>
#include <rw/cstring.h>

#ifndef _DRW_UTILS_H_
#define _DRW_UTILS_H_

extern ifstream& operator >> (ifstream& in, RWTValSlist<RWCString>& alist);
extern ostream& operator << (ostream& out, RWTValSlist<RWCString>&  list);

#endif
