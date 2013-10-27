#include "RegistrarConnection.h"
#include "Broker.h"
#include "PerfLog.h"

#include <rw/cstring.h>
#include <rw/tpslist.h>
#include <rw/ctoken.h>

/**
 * RegistrarConnection implementation.
 *
 * $Id: RegistrarConnection.cc,v 1.1 1998/11/17 23:10:42 toddm Exp $
 */

/**
 * Constructs a registrar connection.
 *
 * Params:
 *     addr - the registrar host and port
 */
RegistrarConnection::RegistrarConnection (const NetAddress &addr)
{
  _addr = new NetAddress (addr);
}

/**
 * Gets a prioritized list of brokers.  This is an expensive
 * operation as it connects to the registrar to get the list.
 *
 * Returns:
 *     the broker list
 */
RWTPtrSlist<Broker> *RegistrarConnection::getBrokers ()
{
  // ***********************
  // * Performance logging *
  // ***********************
  PerfLogger tmGetBlist;
  tmGetBlist.StartTime();

  // Connect to registrar, request broker list, get response.
  // Format of response is "blist <name1>=<addr1>[,<name2>=<addr2>...]"
  RWCString strBlist = _addr->query ("reqBlist");

  // ***********************
  // * Performance logging *
  // ***********************
  tmGetBlist.EndTime();
  tmGetBlist.ReportPerf("Get Blist", logf);

  RWTPtrSlist<Broker> *blist = new RWTPtrSlist<Broker> ();

  // Parse response and put brokers into list.
  RWCTokenizer st (strBlist);
  st ();
  RWCString broker;
  while (!(broker = st (" ,")).isNull ()) 
  {
    Broker *b = new Broker (broker);
    blist->insert (b);
  }

  return blist;
}

#ifdef _RC_MAIN

/**
 * For debugging only!
 */
main (int argc, char *argv[])
{
  // Check usage.
  if (argc != 2)
  {
    cerr << "usage: " << argv[0] << " <reg_addr>" << endl;
    exit (-1);
  }

  // Get broker list.
  NetAddress regAddr (argv[1]);
  RegistrarConnection rc (regAddr);
  RWTPtrSlist<Broker> *blist = rc.getBrokers ();

  // Write out brokers.
  RWTPtrSlistIterator<Broker> i (*blist);
  cout << "Brokers:" << endl;
  for (; i(); )
    cout << "  " << i.key()->name << "=" << i.key()->addr->toString()
	 << endl;
}

#endif
