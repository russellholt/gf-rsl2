#include <rw/cstring.h>

#include "NetAddress.h"
#include "RegistrarConnection.h"
#include "destiny.h"

#ifndef _BrokerConnection_H_
#define _BrokerConnection_H_

/**
 * Manages communication with the brokers.  Uses the registrar to
 * determine the priority of brokers to use.  Switches brokers if
 * the current no longer accepts connections.  Once the list of
 * brokers is depleted, all further communication fails.
 *
 * $Id: BrokerConnection.h,v 1.1 1998/11/17 23:11:22 toddm Exp $
 */
class BrokerConnection
{

public:

  /**
   * Constructs a broker connection object.  Connects to the 
   * registrar, gets a prioritized broker list and prepares to
   * connect to the first broker.  It uses the other brokers if
   * it can't connect to the first.
   *
   * Params:
   *     chnlAddr - the host and port where the channel listens
   *     regAddr - the registrar's host and port
   */
  BrokerConnection (const NetAddress &chnlAddr, const NetAddress &regAddr);

  /**
   * Destroys a broker connection object.
   */
  ~BrokerConnection () { delete chnlAddr; delete blistIter; delete blist; }

  /**
   * Connects to a broker and sends a 'create' session message.
   *
   * Returns:
   *     the newly created session id
   */
  RWCString createSession ();

  /**
   * Determines if this broker connection is valid.
   *
   * Returns:
   *     boolean validity value
   */
  bool isValid () { return brokerValid; }

  /**
   * Connects to a broker and sends a 'connect' to GCP message.
   *
   * Params:
   *     sid - the session to connect to
   * Returns:
   *     zero on success, non-zero on failure
   */
  int connectToGCP (const RWCString& sid);

private:

  /**
   * Fetches a new broker list from the registrar.
   */
  void getBrokers ();

  /**
   * If it is time for a broker list refresh, fetches a
   * new list from the registrar.
   */
  void getBrokersPeriodically ();

  /**
   * Find broker with specified ID.
   *
   * Params:
   *     bid - the broker id
   * Returns:
   *     the broker
   */
  Broker *BrokerConnection::findBroker (const RWCString &bid);

  /**
   * Address of the channel to connect to.
   */
  NetAddress *chnlAddr;

  /**
   * Address of the registrar.
   */
  NetAddress *regAddr;

  /**
   * The list of brokers.
   */
  RWTPtrSlist<Broker> *blist;

  /**
   * Points to the current broker.
   */
  RWTPtrSlistIterator<Broker> *blistIter;

  /**
   * Does the iterator point at a valid broker?
   */
  bool brokerValid;

  /**
   * Records the last time of a broker list refresh.
   */
  long lastBlistRefresh;

  /**
   * Broker list refresh rate in minutes.
   */
//  static const int BLIST_REFRESH_RATE = 10;
  enum {BLIST_REFRESH_RATE = 10};
};

#endif



