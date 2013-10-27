#include <rw/cstring.h>
#include "NetAddress.h"
#include "RegistrationList.h"

#ifndef _ConfigFileParser_H_
#define _ConfigFileParser_H_

/**
 * The Granite Foundation administration utility.
 *
 * Usage:
 *    Admin <registrar_address>
 *
 * $Id: Admin.h,v 1.1 1998/11/17 23:43:29 toddm Exp $
 */
class Admin {

  //--------------------------- PUBLIC -------------------------------
  public:

    /**
     * Constructs an administrator.
     *
     * Params: regAddr the registrar's host and port
     */
    Admin (const NetAddress& regAddr): _regAddr (regAddr) {}

    /**
     * Continually prompts the user to enter Admin commands.
     */
    void prompt ();

  //--------------------------- PRIVATE ------------------------------
  private:

    /**
     * Gets the list of brokers from the registrar, sends them the
     * request then optionally sends the request to the registrar.
     *
     * Params:
     *    message - message to broadcast
     *    includeReg - whether or not registrar should be sent the message
     */
    void broadcast (const RWCString& message, RWBoolean includeReg);

    /**
     * Gets the list of registered brokers.
     *
     * Returns:
     *    broker list
     * Throws:
     *    IllegalArgumentException
     *    IOException
     */
    RegistrationList* getBrokers ();

    /**
     * The registrar's address.
     */
    NetAddress _regAddr;
};

#endif


