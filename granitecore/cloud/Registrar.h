#include <sockinet.h>
#include "NetAddress.h"
#include "RegistrationList.h"
#include "CryptoSubSystem.h"

#ifndef _Registrar_H_
#define _Registrar_H_

/**
 * The Registrar daemon listens on the specified port, waiting for 
 * requests.
 *
 * Usage:
 *    java Registrar <config_file>
 *
 * $Id: Registrar.h,v 1.2 1998/11/23 19:22:30 cking Exp $
 */
class Registrar {

  //--------------------------- PUBLIC -------------------------------
  public:

    /**
     * Constructs a registrar.
     *
     * Params:
     *    address the host and port
     * Throws:
     *    IOException
     */
    Registrar (NetAddress address, int encryptionSupport);

    /**
     * Listens for connection requests and handles each in a
     * separate thread.
     */
    void listen ();

  //--------------------------- PRIVATE ------------------------------
  private:

    /**
     * This registrar's host and port.
     */
    NetAddress address;

    /**
     * List of registered brokers.
     */
    RegistrationList _rlist;

    /**
     * This registrar listens on this socket.
     */
    sockinetbuf socket;

    /**
     * Flag signalling that a shutdown request been made.
     */
    RWBoolean shutdownReq;

    /**
     * Flag signaling to enable encryption support.
     */
    int encryption;
};

#endif
