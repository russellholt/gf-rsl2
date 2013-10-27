#include <stream.h>
#include "RegistrationList.h"

#ifndef _RegistrarRequestHandler_H_
#define _RegistrarRequestHandler_H_

/**
 * A handler for Registrar requests.  It reads a request from
 * a socket connection, determines the type of request and
 * handles it.  The two types are registration requests and
 * broker-list requests.
 *
 * $Id: RegistrarRequestHandler.h,v 1.1 1998/11/17 23:44:15 toddm Exp $
 */
class RegistrarRequestHandler {

  //--------------------------- PUBLIC -------------------------------
  public:

    /**
     * Constructs a request handler for a given socket connection.
     *
     * Params:
     *    theStream - the socket stream
     *    rlist - the registration list
     */
    RegistrarRequestHandler (iostream& theStream, RegistrationList& rlist):
        _shutdownReq (FALSE), _stream (theStream), _rlist (rlist) {}
  
    /** 
     * Main body of thread's execution.  Reads a request from the
     * socket, interprets it and services the request. 
     */
    void run ();

    /**
     * Flag signaling that a shutdown request has been made.
     */
    RWBoolean _shutdownReq;

  //--------------------------- PRIVATE ------------------------------
  private:

    /**
     * List of registered brokers.
     */
    RegistrationList& _rlist;

    /**
     * The socket stream.
     */
    iostream& _stream;
};

#endif
