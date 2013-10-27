#include <rw/cstring.h>

#ifndef _ConfigFileParser_H_
#define _ConfigFileParser_H_

/**
 * A parser for Granite Foundation Network and Process Architecture
 * configuration files.  Reads lines of data, differentiating data
 * lines from comment lines.  Comment lines begin with '#'.
 * Specific configuration file parsers (subclasses) handle parsing
 * of data lines.
 *
 * $Id: ConfigFileParser.h,v 1.1 1998/11/17 23:43:41 toddm Exp $
 */
class ConfigFileParser {

  //--------------------------- PUBLIC -------------------------------
  public:

    /**
     * Constructs a configuration file parser.
     *
     * Params:
     *    filename - name of config file
     */
    ConfigFileParser (const RWCString& strFilename):
        dataLineCount (0), ConfigFileParser::filename (strFilename) {}

    /**
     * Loads data from a configuration file.
     * 
     * Throws:
     *    FileFormatException
     *    IllegalArgumentException
     */
    void open ();

  //------------------------ PROTECTED ----------------------------
  protected:

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
    virtual void parseLine (const RWCString& line);

    /**
     * Parses a data line, storing the contents for later
     * retrieval.
     *
     * Params:
     *    line - the data line
     *    count - the data line number (assigned sequentially from 1)
     * Throws:
     *    FileFormatException
     *    IllegalArgumentException
     */
    virtual void parseDataLine (const RWCString& line, int count) = 0;

  //--------------------------- PRIVATE ------------------------------
  private:

    /**
     * Count of data lines read.
     */
    int dataLineCount;

    /**
     * Name of configuration file.
     */
    const RWCString filename;
};

#endif
