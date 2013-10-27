#include <rw/cstring.h>
#include <stdlib.h>
#include "BrokerConfigFileParser.h"

/**
 * Implementation of the parser for Broker configuration files.
 *
 * $Id: BrokerConfigFileParser.cc,v 1.2 1998/11/23 19:21:57 cking Exp $
 */

/**
 * Stores the Broker and Registrar net addresses for
 * later retrieval.
 *
 * Params:
 *    line - the data line
 *    count - the data line number
 * Throws:
 *    FileFormatException
 *    IllegalArgumentException
 */
void BrokerConfigFileParser::parseDataLine (const RWCString& line,
					    int count)
{
    switch (count) {

    case 1:
        brkAddr = new NetAddress (line);
	break;
    case 2:
        regAddr = new NetAddress (line);
	break;
    case 3:
        gcpCount = atoi (line);
	if (gcpCount == 0)
	{
	    throw RWCString ("FileFormatException: invalid Broker configuration file format");
	}
	break;
    case 4:
        gcpRunScript = line;
	break;
    case 5:
        encryption = atoi (line);
	if (encryption != 1)
	    encryption = 0;
	break;

    // There should be only three data lines.
    default:
        throw RWCString ("FileFormatException: invalid Broker configuration file format");
    }
}

//--------------------------- TESTING ------------------------------
#ifdef _BC_MAIN

#include <stream.h>

/**
 * For unit testing only!
 *
 * Usage:  BrokerConfigFileParser <configFile1> ...
 */
main (int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        try
	{
      	    BrokerConfigFileParser bcfp (argv[i]);
	    bcfp.open ();
	    cout << bcfp.getRegistrarAddr().toString () << endl;
	    cout << bcfp.getBrokerAddr().toString () << endl;
	    int gcpCount = bcfp.getGcpCount();
	    char strGcpCount [10];
	    sprintf (strGcpCount, "%d", gcpCount);
	    cout << strGcpCount << endl;
	}
	catch (RWCString s)
	{
	    cerr << s << endl;
	}
    }
}

#endif
