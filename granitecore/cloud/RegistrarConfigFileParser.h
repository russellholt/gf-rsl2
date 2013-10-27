#include <rw/cstring.h>
#include "ConfigFileParser.h"
#include "NetAddress.h"

#ifndef _RegistrarConfigFileParser_H_
#define _RegistrarConfigFileParser_H_

/**
 * A parser for Registrar configuration files.
 *
 * $Id: RegistrarConfigFileParser.h,v 1.3 1998/11/30 20:29:47 cking Exp $
 */
class RegistrarConfigFileParser : public ConfigFileParser {

  //--------------------------- PUBLIC -------------------------------
  public:

    /**
     * Constructs a registrar file parser.
     *
     * Params:
     *    filename - the config file
     */
    RegistrarConfigFileParser (const RWCString& strFilename):
        ConfigFileParser (strFilename) { address = NULL; encryption = 0;}

    /**
     * Gets the network address (host and port).
     *
     * Returns:
     *    the address
     */
    NetAddress getAddress () const { return *address; }

    /**
     * Gets the encryption support flag.
     *
     * Returns:
     *    the flag
     */
     int getUseCryptoGraphy () { return encryption; }
     
  //------------------------ PROTECTED ----------------------------
  protected:

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
    void parseDataLine (const RWCString& line, int count);

  //--------------------------- PRIVATE ------------------------------
  private:

    /**
     * The registrar's host and port.
     */
    NetAddress *address;

    /**
     * Flag signaling to enable encryption support.
     */
    int encryption;
};

#endif
