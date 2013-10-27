#include <rw/cstring.h>
#include <rw/ctoken.h>
#include "NetAddress.h"

#ifndef _Broker_H_
#define _Broker_H_

/**
 * A broker is represented by a name and network address.
 *
 * $Id: Broker.h,v 1.1 1998/11/17 23:11:19 toddm Exp $
 */
class Broker
{
public:

  /**
   * This broker's identifier.
   */
  RWCString name;

  /**
   * This broker's network address.
   */
  NetAddress *addr;

  /**
   * Constructs a broker.
   *
   * Params:
   *     brkName - the broker's name
   *     brkAddr - the broker's network address
   */
  Broker (const RWCString &brkName, const NetAddress &brkAddr):
    name (brkName) { addr = new NetAddress (brkAddr); }

  /**
   * Constructs a broker from the given string.
   *
   * Params:
   *     strBroker - the string, e.g., "B1=horizon:3838"
   */
  Broker (const RWCString &strBroker) { RWCTokenizer st (strBroker);
                                        name = st ("=");
                                        addr = new NetAddress (st ("=")); }

  /**
   * Destroys a broker.
   */
  ~Broker () { delete addr; }
};

#endif



