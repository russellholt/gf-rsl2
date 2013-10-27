// *****************************************************************************
// *
// * ECI_Interface
// *
// * History: Create - TGM 2/2/98
// * 
// * $Id: ECI_Client.h,v 1.1 1998/11/17 23:11:26 toddm Exp $
// * 
// * $Log: ECI_Client.h,v $
// * Revision 1.1  1998/11/17 23:11:26  toddm
// * Initial revision
// *
// * Revision 2.2  1998/06/29 19:25:52  toddm
// * Move all includes outside of exclusion symbol
// *
// * Revision 2.1  1998/04/21 13:11:46  toddm
// * Bump Version
// *
// * Revision 1.4  1998/04/21 04:19:05  toddm
// * Make Connect public
// *
// * Revision 1.3  1998/04/10 13:40:34  prehmet
// * Added Listen method that accepts port range.
// *
// * Revision 1.2  1998/03/31 16:22:18  prehmet
// * New model for handling connection.
// *
// * Revision 1.1  1998/02/13 22:02:22  toddm
// * Initial revision
// *
// * Revision 1.1  1998/02/13 22:00:14  toddm
// * Initial revision
// *
// *
// * Copyright 1998 by Destiny Software Corporation.
// * 
// *******************************************************************************
// *******************
// * System Includes *
// *******************
#include <iostream.h>
#include <sockinet.h>

// ******************
// * Local Includes *
// ******************
#include "lexer_context.h"
#include "b.h"
#include "rslEvents.h"
#include "CryptoSubSystem.h"

#ifndef _ECI_CLIENT_H_
#define _ECI_CLIENT_H_

class ECI_Client
{
    // ************************
    // * Private Data Members *
    // ************************
    iosockinet *pioGCore;                    // Server stream socket
    inline iosockinet &ioGCore(void) {return *pioGCore;}

    lexer_context lexc;

    RWCString strHost;
    int iPort;
    sockinetbuf sin;

    // ************************
    // * Private Methods      *
    // ************************
    void Init(void);

public:
    // ************************
    // * Public Data Members *
    // ************************
    CryptoDHDEsession  * dhdeSn;		    // diffie-hellman & digital hellman session descriptor
    CryptoSubSystem * cryptoSubsystem;		    // cryptography support subsystem

    ECI_Client(void);
    ~ECI_Client(void);

    int Connect(RWCString strHostName, int iPortNum);
    void Close( );

    int Listen(int portnum);
    int Listen(int startPort, int endPort);
    void Accept();
    void Disconnect();

    int Write(event *arTheRequest);
    int IsValidRead(void);
    event *Read(void);
    event *GetReturnArguments( event *arTheReturn );
    event *Execute(event *arTheRequest);
};

#endif



