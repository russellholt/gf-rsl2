#include <rw/cstring.h>
#include <rw/tpslist.h>
#include "NetAddress.h"
#include "Broker.h"

#ifndef _RegistrarConnection_H_
#define _RegistrarConnection_H_

/**
 * Manages communication with the registrar.  Connects and gets
 * the prioritized list of brokers.
 *
 * $Id: RegistrarConnection.h,v 1.1 1998/11/17 23:11:37 toddm Exp $
 */
class RegistrarConnection
{

public:

  /**
   * Constructs a registrar connection.
   *
   * Params:
   *     addr - the registrar host and port
   */
  RegistrarConnection (const NetAddress &addr);

  /**
   * Destroys a registrar connection.
   */
  ~RegistrarConnection () { delete _addr; }

  /**
   * Gets a prioritized list of brokers.  This is an expensive
   * operation as it connects to the registrar to get the list.
   *
   * Returns:
   *     the broker list
   */
  RWTPtrSlist<Broker> *getBrokers ();

private:

  /**
   * Registrar address.
   */
  NetAddress *_addr;
};

#endif



