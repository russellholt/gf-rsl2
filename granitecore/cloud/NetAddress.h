#include <rw/cstring.h>
#include <stdio.h>

#ifndef _NetAddress_H_
#define _NetAddress_H_
#include "CryptoSubSystem.h"

/**
 * A network address, i.e., host and port.
 *
 * $Id: NetAddress.h,v 1.1 1998/11/17 23:43:55 toddm Exp $
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
   * Constructs a default network address.
   */
  NetAddress (): host ("unknown"), port (0), cryptoSubsystem (NULL) {}

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
   * Throws:
   *     IllegalArgumentException
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
   * Throws:
   *     IOException
   */
  RWCString query (const RWCString& message, const int timeout = 2) const;

  /**
   * Determines if a network addresses is equivalent to this one.
   *
   * Params:
   *    other - the other address
   * Returns:
   *    whether they are equivalent
   */
  int operator==(const NetAddress& other) const
      { return ((host == other.host) && (port == other.port)); }

  /**
   * Generates a hash code for a net address.
   */
  static unsigned hash (const NetAddress& addr)
      { char strPort [10]; sprintf (strPort, "%d", addr.port);
        return RWCString::hash (addr.host)/2 + RWCString::hash (strPort)/2; }
};

#endif
