// rsl_html.h
// simple html-ification shortcuts
// $Id: rsl_html.h,v 1.1 1998/11/18 00:01:23 toddm Exp $

// *******************
// * System Includes *
// *******************
#include <rw/cstring.h>

// ******************
// * Local Includes *
// ******************

#define endAnchor "</A>"

inline void linkClassName(RWCString cname, ostream& out)
{
	out << "<A HREF=\"" << cname << ".html\">"
		<< cname << "</A>";
}

inline RWCString methodTargetName(RWCString cname, RWCString method)
{
	// simple for now.
	return cname + RWCString("_") + method;
}

inline void beginTargetDef(RWCString target, ostream& out)
{
	out << "<A NAME=\"" << target << "\">";
}




