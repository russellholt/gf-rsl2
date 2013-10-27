#include <fstream.h>
#include <rw/cstring.h>
#include "ConfigFileParser.h"

/**
 * Implementation of the Granite Foundation Network and Process
 * Architecture parser.
 *
 * $Id: ConfigFileParser.cc,v 1.1 1998/11/17 23:42:36 toddm Exp $
 */

/**
 * Loads data from a configuration file.
 * 
 * Throws:
 *    FileFormatException
 *    IllegalArgumentException
 */
void ConfigFileParser::open () 
{
    ifstream cfStream (filename);
    if (!cfStream)
    {
      throw RWCString ("FileNotFoundException: ") + filename;
    }

    RWCString line;
    line.readLine (cfStream, FALSE);
    while (!line.isNull ())
    {
        parseLine (line);
	line.readLine (cfStream, FALSE);
    }
}

/**
 * Parses a config file line, handling it if a data line
 * and ignoring it if a comment line.  Keeps track of the
 * number of data lines read.
 *
 * Params:
 *    line - the config file line
 * Throws:
 *    FileFormatException
 *    IllegalArgumentException
 */
void ConfigFileParser::parseLine (const RWCString& line)
{
    if (line.first ('#') != 0)
    {
        dataLineCount++;
	parseDataLine (line, dataLineCount);
    }
}
