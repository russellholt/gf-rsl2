#include <rw/cstring.h>
#include <stdio.h>

#ifndef _NetAddress_H_
#define _NetAddress_H_
#include "CryptoSubSystem.h"

/**
 * A network address, i.e., host and port.
 *
 * $Id: NetAddress.h,v 1.1 1998/11/17 23:11:30 toddm Exp $
 */
class NetAddress {

public:

  /**
   * The hostname, e.g., "horizon.destinyusa.com"
   */
  RWCString host;

  /**
   * The port.
   */
  int port;

    CryptoSubSystem * cryptoSubsystem;		// cryptography support subsystem
    
  /**
   * Constructs a network address.
   *
   * Params:
   *     host - the host
   *     port - the port
   */
  NetAddress (const RWCString& theHost, const int thePort): 
    host (theHost), port (thePort), cryptoSubsystem (NULL) {}

  /**
   * Constructs a network address from an address string.
   *
   * Params:
   *     addrStr - the address string, e.g., "host:port"
   */
  NetAddress (const RWCString& addrStr);

  /**
   * Copies a network address.
   *
   * Params:
   *     addr - a network address
   */
  NetAddress (const NetAddress& addr):
    host (addr.host), port (addr.port), cryptoSubsystem (addr.cryptoSubsystem) {}

  /**
   * Gets the network address string "host:port".
   *
   * Returns:
   *     the string
   */
  RWCString toString () const { char thePort[10]; 
                                sprintf (thePort, "%d", port);
                                return (host + ":" + thePort); }

  /**
   * Sends a message to the server at this address and gets the response.
   *
   * Params:
   *     message - the query
   *     timeout - the timeout in seconds when reading the response
   *               (-1 = block, 0 = poll)
   * Returns:
   *     the response
   */
  RWCString query (const RWCString& message, const int timeout = 60) const;
};

#endif



