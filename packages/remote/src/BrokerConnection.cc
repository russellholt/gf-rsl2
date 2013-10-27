#include <iostream.h>
#include <rw/cstring.h>
#include <rw/tpslist.h>
#include <time.h>

#include "BrokerConnection.h"
#include "SessionId.h"
#include "destiny.h"
/**
 * BrokerConnection implementation.
 *
 * $Id: BrokerConnection.cc,v 1.1 1998/11/17 23:10:29 toddm Exp $
 */

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
BrokerConnection::BrokerConnection (const NetAddress &chanlAddr, 
				    const NetAddress &regsAddr)
{
  // Store to use in 'create' and 'connect' commands.
  BrokerConnection::chnlAddr = new NetAddress (chanlAddr);
  BrokerConnection::regAddr = new NetAddress (regsAddr);
  getBrokers ();
}

/**
 * Connects to a broker and sends a 'create' session message.
 *
 * Returns:
 *     the session id
 */
RWCString BrokerConnection::createSession ()
{
  // Fetch new broker list occasionally.
  getBrokersPeriodically ();

  // If broker invalid, return failure value.
  if (!brokerValid)
    return RWCString ();

  // Send 'create' session request to broker and get response.
  // If connection to broker fails, try the next broker and so on.
  // If no more brokers, return failure value.
  RWCString response;
  while ((response = blistIter->key()->addr->query
	  ("create " + chnlAddr->toString())).isNull ())
  {
    if (!(*blistIter) ())
    {
      brokerValid = FALSE;
      return RWCString ();
    }
  }

  RWCTokenizer st (response);
  RWCString firstWord = st ();

  // If Broker returns error, log error and return failure value.
  if (firstWord.compareTo ("error") == 0)
  {
    cerr << response << endl;
    return RWCString ();
  }

  // Return session id. (the second word)
  return st ();
}

/**
 * Find broker with specified ID.
 *
 * Params:
 *     bid - the broker id
 * Returns:
 *     the broker
 */
Broker *BrokerConnection::findBroker (const RWCString &bid)
{
  RWTPtrSlistIterator<Broker> bIter (*blist);
  for (; bIter (); )
  {
    if (bIter.key()->name == bid)
      return bIter.key();
  }
  return NULL;
}

/**
 * Connects to a broker and sends a 'connect' to GCP message.
 *
 * Params:
 *     sid - the session to connect to
 * Returns:
 *     zero on success, non-zero on failure
 */
int BrokerConnection::connectToGCP (const RWCString &sid)
{
  // Get broker ID from session ID.
  RWCString brokerId (SessionId(sid).brokerId);
  
  // Find broker with this ID.
  Broker* b = findBroker (brokerId);

  // If not found, get new broker list and try again.
  if (b == NULL)
  {
    getBrokers ();
    b = findBroker (brokerId);
  }

  // If still not found, give up.
  if (b == NULL)
  {
    return -1;
  }

  // Send 'connect' request to broker and get response.  If connection
  // to broker fails, try the next broker and so on.  If no more
  // brokers, return failure value.
  RWCString response;
  if ((response = b->addr->query
	  ("connect " + sid + " " + chnlAddr->toString())).isNull ())
  {
    return -1;
  }

  // If Broker returns error, log error and return failure value.
  if (response.compareTo ("success") != 0)
  {
    cerr << response << endl;
    return -1;
  }

  // Success.
  return 0;
}

/**
 * Gets the broker list from the registrar.  Records the
 * time when the list was fetched.
 */
void BrokerConnection::getBrokers () {
  // Get broker list from registrar.
  RegistrarConnection rc (*regAddr);
  blist = rc.getBrokers ();

  // Point iterator to first broker.  If there are no
  // brokers in list, set invalid broker flag.
  blistIter = new RWTPtrSlistIterator<Broker> (*blist);
  if (blist->isEmpty ())
    brokerValid = FALSE;
  else
  {
    brokerValid = TRUE;
    (*blistIter) ();
  }

  lastBlistRefresh = time (NULL);
}

/**
 * Infrequently gets a new broker list in case new
 * brokers have been added or removed.
 */
void BrokerConnection::getBrokersPeriodically ()
{
  long curTime = time (NULL);
  if (curTime - lastBlistRefresh >= (60 * BLIST_REFRESH_RATE))
    getBrokers ();
}

#ifdef _BC_MAIN

/**
 * For debugging only!
 */
main (int argc, char *argv[])
{

  // Check usage.
  if (argc != 3)
  {
    cerr << "usage: " << argv[0] << " <chnl_addr> <reg_addr>" << endl;
    exit (-1);
  }

  // Connect.
  NetAddress chnlAddr (argv[1]), regAddr (argv[2]);
  BrokerConnection bc (chnlAddr, regAddr);

  // Create session.
  RWCString sid = bc.createSession ();
  if (sid.isNull ()) 
  {
    cerr << "BrokerConnection: error creating session" << endl;
    exit (-1);
  }

  // Connect to GCP.
  cout << "Sid: " << sid << endl;
  if (bc.connectToGCP (sid) != 0)
  {
    cerr << "BrokerConnection: error connecting to GCP" << endl;
    exit (-1);
  }
  cout << "Connection to GCP succeeded!" << endl;
  exit (0);
}

#endif
