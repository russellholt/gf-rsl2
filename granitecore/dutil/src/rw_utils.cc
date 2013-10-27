#include "rw_utils.h"

/* Read each line from an ifstream into a String and
 *  append it to a StringSLList.
 *
 * Russell Holt, May 17 1995
 * Modified for SLList<String> July 28 1995
 * fixed to work Oct 25 1995
 */
ifstream& operator >> (ifstream& in, RWTValSlist<RWCString>& alist)
{
//char x[1024];
//int i;
RWCString thestring(RWSize_T(80));	// set initial capacity to reduce resizing
//_IO_size_t len, Max = 1024;

	while (!in.eof())
	{
	/*
		for(i=0; i<1024; i++) x[i]='\0';	// initialize buffer

		// handle arbitrarily long lines without specifying length
		do {
			in.getline(x, Max);
			len = in.gcount();
			thestring += x;
							// 1024 total length, 1023 would hold '\0'
		} while (len >= Max && x[Max-1] != '\n' );
	*/
		thestring.readLine(in, FALSE);	// false - don't skip whitespace
		//	cerr << "Read: " << thestring << "\n";

		if (in.eof())
			break;
		// Copying the RWCString and changing the copy will set a minimal
		// capacity in the new string, rather than the extra capacity of thestring.
		// RWCString::readLine doesn't store the newline
		// alist.append(thestring.strip(RWCString::trailing, '\n'));

		alist.append(thestring);

		thestring = "";
	}

	return in;
}


ostream& operator<<(ostream& out, RWTValSlist<RWCString>&  list)
{
RWTValSlistIterator<RWCString> iter(list);

	while(iter())
		out << iter.key() << '\n';

	return out;
}

