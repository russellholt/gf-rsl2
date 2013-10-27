#include <rw/tpvector.h>
#include <stream.h>
#include <stdlib.h>
#include "GcpManager.h"
#include "NetAddress.h"
#include "RegistrationList.h"

#ifndef _Broker_H_
#define _Broker_H_

/**
 * The Process Broker daemon of the Granite Foundation Network and
 * Process Architecture.
 *
 * Usage:
 *    java Broker <config_file> [silent]
 *
 * $Id: Broker.h,v 1.2 1998/11/23 19:21:50 cking Exp $
 */
class Broker {

  //--------------------------- PUBLIC -------------------------------
  public:

    /**
     * Constructs a Process Broker.
     *
     * Params: 
     *    brkAddr - the broker's host and port
     *    regAddr - the registrar's host and port
     *    gcpCount - the number of GCP's to create and manage
     */
    Broker (const NetAddress brkAddr, const NetAddress regAddr, 
	    const int gcpCount, const int encryption) : 
      _brkAddr (brkAddr), _regAddr (regAddr), _gcpCount (gcpCount),
      _gcpMgrs (gcpCount), _encryption (encryption), _shutdown (FALSE), _blist (NULL) {}

    /**
     * Registers with the registrar, starts the Granite Core Processes,
     * then listens for requests and services them.
     *
     * Params:
     *    gcpRunScript - the Granite Core run script
     * Throws:
     *    IOException 
     */
    void run (const RWCString& gcpRunScript);

  //--------------------------- PRIVATE ------------------------------
  private:

    /**
     * Registers with the registrar.  Sends request "register <host>:<port>".
     * Gets response "registered <broker_id>".
     *
     * Throws:
     *    IOException
     */
    void registerMe ();

    /**
     * Removes this broker from the registry.  Sends request 
     * "unregister <bId>".  Gets response "success".
     */
    void unregisterMe ();

    /**
     * Sends all active GCPs a shutdown message.
     *
     * Params:
     *    seconds - seconds until shutdown
     */
    void sendShutdownToGcps (int seconds);

    /**
     * Handles requests from channels and other brokers.
     * Throws:
     *    IOException 
     */
    void handleRequest (iostream& socket);

    /**
     * Creates a Granite Core Process ID from an integer.
     *
     * Params: 
     *    index - the integer
     * Returns:
     *    the id
     */
    RWCString gcpId (int index) 
        { char strIdx [10]; sprintf (strIdx, "%d", index + 1);
	  return RWCString ("G") + RWCString (strIdx); }

    /**
     * Gets the index associated with a GCP.
     *
     * Params: id the GCP id
     * Returns:s the index
     */
    int gcpIndex (const RWCString& id)
        { return atoi (id.data () + 1) - 1; }

    /**
     * This broker's identifier.
     */
    RWCString _id;
	
    /**
     * The list of all brokers.  The order doesn't matter.
     */
    RegistrationList *_blist;

    /**
     * Number of Granite Core Processes to create.
     */
    int _gcpCount;

    /**
     * The list of Granite Core Processes.
     */
    RWTPtrVector<GcpManager> _gcpMgrs;

    /**
     * This broker's address.
     */
    NetAddress _brkAddr;

    /**
     * The registrar's address.
     */
    NetAddress _regAddr;

    /**
     * Flag signaling that it is time to shutdown.
     */
    RWBoolean _shutdown;

    /**
     * Flag signaling to enable encryption support.
     */
    int _encryption;
};

#endif
