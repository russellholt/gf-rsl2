#include <rw/cstring.h>
#include "RegistrarConfigFileParser.h"

/**
 * Implementation of the parser for Registrar configuration files.
 *
 * $Id: RegistrarConfigFileParser.cc,v 1.2 1998/11/23 19:22:38 cking Exp $
 */

/**
 * Stores the host and port for later retrieval.
 *
 * Params:
 *    line - the data line
 *    count - the data line number
 * Throws:
 *    FileFormatException
 *    IllegalArgumentException
 */
void RegistrarConfigFileParser::parseDataLine (const RWCString& line,
					       int count) 
{
    switch (count) {

    // First line is the net address.
    case 1:
        address = new NetAddress (line);
	break;

    case 2:
        encryption = atoi (line);
	if (encryption != 1)
	    encryption = 0;
	break;

        // File should have only one line.
    default:
        throw RWCString ("FileFormatException: only one non-comment line expected (format=host:port)");
    }
}

//--------------------------- TESTING ------------------------------
#ifdef _RC_MAIN

#include <stream.h>

/**
 * For unit testing only!
 *
 * Usage:  RegistrarConfigFileParser <configFile1> ...
 */
main (int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        try
	{
      	    RegistrarConfigFileParser rcfp (argv[i]);
	    rcfp.open ();
	    cout << rcfp.getAddress().toString () << endl;
	}
	catch (RWCString s)
	{
	    cerr << s << endl;
	}
    }
}

#endif
