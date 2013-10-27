#include <rw/cstring.h>
#include "ConfigFileParser.h"
#include "NetAddress.h"

#ifndef _BrokerConfigFileParser_H_
#define _BrokerConfigFileParser_H_

/**
 * The parser for Broker configuration files.
 *
 * $Id: BrokerConfigFileParser.h,v 1.3 1998/11/30 20:29:26 cking Exp $
 */
class BrokerConfigFileParser: public ConfigFileParser {

  //--------------------------- PUBLIC -------------------------------
  public:

    /**
     * Constructs a broker configuration file parser.
     */
    BrokerConfigFileParser (const RWCString& strFilename):
        ConfigFileParser (strFilename) { brkAddr = NULL; regAddr = NULL; gcpCount = 0; encryption = 0;}

    /**
     * Gets the Broker's address.
     *
     * Returns:
     *    the address
     */
    NetAddress getBrokerAddr () { return *brkAddr; }

    /**
     * Gets the Registrar's address.
     *
     * Returns:
     *    the address
     */
    NetAddress getRegistrarAddr () { return *regAddr; }

    /**
     * Gets the number of Granite Core Processes to create.
     *
     * Returns:
     *    the count
     */
    int getGcpCount () { return gcpCount; }

    /**
     * Gets the encryption support flag.
     *
     * Returns:
     *    the flag
     */
     int getUseCryptoGraphy () { return encryption; }
     
    /**
     * Gets the Granite Core Process run script.
     *
     * Returns:
     *    the script
     */
    RWCString getGcpRunScript () {
      return gcpRunScript;
    }

  //------------------------ PROTECTED ----------------------------
  protected:

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
    void parseDataLine (const RWCString& line, int count);

  //--------------------------- PRIVATE ------------------------------
  private:

    /**
     * The broker's address.
     */
    NetAddress *brkAddr;
  
    /**
     * The registrar's address.
     */
    NetAddress *regAddr;

    /**
     * The number of GCP's to create.
     */
    int gcpCount;

    /**
     * The Granite Core Process run script.
     */
    RWCString gcpRunScript;

    /**
     * Flag signaling to enable encryption support.
     */
    int encryption;
};

#endif
