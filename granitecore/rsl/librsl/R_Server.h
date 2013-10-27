// *****************************************************************************
// *
// * ECI Server Resource
// *
// * $Id: R_Server.h,v 1.2 1998/11/23 19:37:45 cking Exp $
// *
// * $Log: R_Server.h,v $
// * Revision 1.2  1998/11/23 19:37:45  cking
// * Additional modifications to initial implementation of encryption support for GF.
// *
// * Revision 1.1  1998/11/17 23:55:19  toddm
// * Initial revision
// *
// * Revision 2.3  1998/06/29 19:29:55  toddm
// * Move all includes outside of exclusion symbol
// *
// * Revision 2.2  1998/04/30 18:50:26  toddm
// * Add RecycleServer method
// *
// * Revision 2.1  1998/03/27 18:18:03  toddm
// * Add an execute method and make NewConnection a virtual fuction
// *
// * Copyright 1997 1998 by Destiny Software Corporation.
// *                       
// *******************************************************************************
#ifndef R_Server_H_
#define R_Server_H_

// *******************
// * System Includes *
// *******************
#include <iostream.h>
#include <Fork.h>
#include <sockinet.h>
#include <rw/cstring.h>

// ******************
// * Local Includes *
// ******************
#include "res_class.h"
#include "Resource.h"
#include "rslServer.h"
#include "lexer_context.h"
#include "CryptoSubSystem.h"

#define R_Server_ID 907506294

// ********************************************
// * rc_Server -- the Server RSL class
// ********************************************
class rc_Server : public res_class {
    Resource *spawn(RWCString aname);
public:
    rc_Server(RWCString aname) : res_class(aname)
    {
    }
};

// *************************************************
// * R_Server -- the Server Resource
// *************************************************
class R_Server : public rslServer {
protected:
    // **************************
    // * Protected Data Members *
    // **************************
    elist *incomingArgs;

    CryptoDHDEsession  * dhdeSn;		// diffie-hellman & digital hellman session descriptor
    CryptoSubSystem * cryptoSubsystem;		// cryptography support subsystem


    // constructor for subclasses
    R_Server(RWCString n, res_class *super);

    // *******************
    // * Server specific *
    // *******************
    int RecycleServer(void);    
    void closeCryptographySession (void);

public:
    // ***********************
    // * Public Data Members *
    // ***********************
    static rc_Server rslType;
    lexer_context lexc;

    // ****************
    // * Constructors *
    // ****************
    R_Server(RWCString n);  // don't call from subclass (use above)
    
    // *********************
    // * Resource virtuals *
    // *********************
    unsigned int TypeID() { return R_Server_ID; }
    res_class *memberOf(void) { return &rslType; }
    ResStatus execute(int method, ResList& arglist);

    // **************************************
    // * R_Server specific member functions *
    // **************************************
    static R_Server *New(RWCString n);
    
    // **********************
    // * rslServer virtuals *
    // **********************
    virtual void ListenLoop(int portnum=0, int toFork=1);
    virtual void NewConnection(iostream& in);
    virtual void NewConnection(iosockinet& in);

    virtual void GetIncomingRequest();
    virtual event *TranslateIncoming(void) { return lexc.dynamic_request; }
    int ValidRequest();
    virtual void FinalizeRequest();
    virtual void TranslateOutLoop(event *outgoing);
    virtual event *ExecuteIncomingEvents(event *e);
};
#endif



