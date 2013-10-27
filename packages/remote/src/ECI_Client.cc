// ***************************************************************************
// *
// *  NAME:  ECI_Client.cc
// *
// *  DESCRIPTION:                                                      
// *     Used to communicate with an ECI Server.
// *                                                                    
// *  HISTORY: 
// *    Create - TGM 2/2/98
// * 
// *    $Id: ECI_Client.cc,v 1.7 1999/01/07 14:46:16 cking Exp $
// * 
// *    $Log: ECI_Client.cc,v $
// *    Revision 1.7  1999/01/07 14:46:16  cking
// *    Additional performance enhancements for encryption support in the GF 2.5 release.
// *
// *    Revision 1.6  1998/12/22 15:12:14  cking
// *    Patch implements a configurable receive time-out for the GF Web Channel.
// *
// *    Revision 1.5  1998/12/17 21:43:07  cking
// *    Additional modification for encryption support in the GF 2.5 release.
// *
// *    Revision 1.4  1998/12/14 17:55:36  cking
// *    Additional modifications for encryption support in GF 2.5 release
// *
// *    Revision 1.3  1998/11/30 20:46:04  cking
// *    Additional modifications for encryption support in GF 2.5 release
// *
// *    Revision 1.2  1998/11/23 19:58:58  cking
// *    Additional modifications to initial implementation of encryption support for GF.
// *
// *    Revision 1.1  1998/11/17 23:10:32  toddm
// *    Initial revision
// *
// *    Revision 2.2  1998/05/13 23:31:34  toddm
// *    Add performace logging
// *
// *    Revision 2.1  1998/04/21 13:11:44  toddm
// *    Bump Version
// *
// *    Revision 1.3  1998/04/10 13:41:59  prehmet
// *    Added Listen method that accepts port range.
// *
// *    Revision 1.2  1998/03/31 16:22:15  prehmet
// *    New model for handling connection.
// *
// *    Revision 1.1  1998/02/13 21:59:59  toddm
// *    Initial revision
// *
// *
// * Copyright 1998 by Destiny Software Corporation.
// * 
// *******************************************************************************

// *******************
// * System Includes *
// *******************
#include <stdio.h>			// Standard I/O
#include <stddef.h>			// Standard Definitions
#include <stdlib.h>			// Standard Library
#include <stream.h>
#include <iostream.h>
#include <strstream.h>

// ******************
// * Local Includes *
// ******************
#include "ECI_Client.h"
#include "slog.h"
#include "destiny.h"
#include "PerfLog.h"

extern int parse_it(lexer_context &lexcIt);
int coreReceiveTimeOut = 60;		// 1 minute receive time-out with core process(s)


// *************************************************************************
// **                         PUBLIC MEMBER FUNCTIONS                      *
// **                                                                      *
// **                  Constructors/Destructors for this class             *
// *************************************************************************

// ***********************************************************************
// *
// * NAME:    ECI_Client         Constructor
// *
// * DESCRIPTION:
// *        Constructor for the the ECI_Client class
// *
// * INPUT: 
// *        None
// *                                                                  
// * RETURNS:
// *        Nothing 
// *                                                                  
// ************************************************************************
ECI_Client::ECI_Client(void): sin (sockbuf::sock_stream)
{
    Init();
}

// ***********************************************************************
// *
// * NAME:    ~ECI_Client         Destructor
// *
// * DESCRIPTION:
// *        Destructor for the the ECI_Client class
// *
// * INPUT: 
// *        None
// *                                                                  
// * RETURNS:
// *        Nothing 
// *                                                                  
// ************************************************************************
ECI_Client::~ECI_Client(void)
{
    if (pioGCore)
    {
        ioGCore().clear();
        ioGCore()->close();
        delete pioGCore;    
    }

    if (dhdeSn != NULL)
    {
        if (cryptoSubsystem != NULL)
            cryptoSubsystem->closeSession (dhdeSn);		// close cryptography session

        delete dhdeSn;
        dhdeSn = NULL;
    }
}


void ECI_Client::Init(void)
{
    pioGCore = NULL;    
    dhdeSn = NULL;
    cryptoSubsystem = NULL;
}


// ***********************************************************************
// *
// * NAME:    Listen                    Public Function
// *
// * DESCRIPTION:
// *        Listen on the specifed port.  Should be done only once
// *        per port.
// *
// * INPUT: 
// *        iPortNum - Integer containing the port number to listen on.
// *                                                                  
// * RETURNS: 
// *        Port number selected to listen
// *        0 if there has been an error
// *                                                                  
// ************************************************************************
int ECI_Client::Listen(int portnum)
{
    return Listen(portnum, portnum);
}

int ECI_Client::Listen(int startPort, int endPort)
{
    logf->debug(LOGSERVER) << "(ECI_Client) In Listen" << endline;

    int bindresult=0;

    if (startPort > endPort)
    {
        logf->fatal(LOGSERVER) << "(ECI_Client) Invalid port range, unable to bind to address" << endline;
        return(0);
    }

    // *****************************************************
    // * If there was a port range specified than use any  *
    // * available in range, otherwise pick at random.     *
    // *****************************************************
    if (startPort > 0)
    {
        for (int p = startPort; p <= endPort; p++)
	    if ((bindresult=sin.bind(INADDR_ANY, p)) == 0)
	        break;
    }
    else
    {
        bindresult=sin.bind();
    }
        
    // *****************************************
    // * Could we bind to the address.         *
    // *****************************************
    if (bindresult != 0)
    {
        logf->fatal(LOGSERVER) << "(ECI_Client) Unable to bind to address" << endline;
        return(0);
    }
        
    // ***************************
    // * Set some socket options *
    // ***************************
    sin.reuseaddr(1);
    sin.keepalive(1);
    sin.linger(60); 

    strHost = sin.localhost();
    iPort = sin.localport();

    cout << "localhost = " << strHost << endl
         << "localport = " << iPort << endl << flush;

    sin.listen();

    Logf.notice(LOGSERVER) << "(ECI_Client) Waiting for connection..." << endline;

    // ************************************************
    // * Initialize the granite core streaming socket *
    // ************************************************
    return(iPort);
}

// ***********************************************************************
// *
// * NAME:  Accept                      Public Member Function
// *
// * DESCRIPTION: Accepts a connection on the current socket.
// *              May be called more than once for the same socket.
// *
// * SIDE EFFECTS: Points member 'pioGCore' to the new connection.
// *
// ************************************************************************
void ECI_Client::Accept()
{
    cout << "ECI_Client: Waiting for connection...\n" << flush;
    pioGCore = new iosockinet(sin.accept());

    // ***************************
    // * Set current lexer state *
    // ***************************
    cout << "ECI Client: Accepted connection" << endl
         << "Switching lexer streams with client socket.\n" << flush;


    lexc.SetSource(lexer_context::Stream, "<network>");
    lexc.SetLexerIO(ioGCore(), ioGCore());
    lexc.done = lexer_context::Ready;
    lexc.type = lexer_context::interactive;
    lexc.keystate = lexer_context::Everywhere;


    if (cryptoSubsystem != NULL)
    {
             //
             // Open cryptography session.
             //
        CryptoError crc;
        dhdeSn = new CryptoDHDEsession;
        dhdeSn->setNodeType (CRYPTO_NODE_CENTRAL_AUTHORITY);	// set node type       
        dhdeSn->setPeerNodeType (CRYPTO_PEER_NODE_USER_ACTIVE);	// set node type       
        dhdeSn->setPeerNodeConnection (pioGCore);  // set socket connection to peer node
        if ((crc = cryptoSubsystem->openSession (dhdeSn)) != CRYPTO_ERR_OK)
        {
            logf->error (LOGSERVER) << "(ECI Client) Fatal error: unable to open cryptography session; error = " << crc << endline;
        }
    }

}

// ***********************************************************************
// *
// * NAME:    Disconnect                  Public Member Function
// *
// * DESCRIPTION: Closes and cleans up the current connection.
// *
// ************************************************************************
void ECI_Client::Disconnect()
{
    if (dhdeSn != NULL)
    {
        logf->debug(LOGSERVER) <<  "(ECI_Client::Disconnect): delete cryptography objects" << endline;
        if (dhdeSn != NULL)
        {
            if (cryptoSubsystem != NULL)
                cryptoSubsystem->closeSession (dhdeSn);		// close cryptography session
            delete dhdeSn;
            dhdeSn = NULL;
        }
    }
    delete pioGCore;
    pioGCore = NULL;
}

// ***********************************************************************
// *
// * NAME:    Connect                    Public Function
// *
// * DESCRIPTION:
// *        Connect to the specifed port.  When a connection is made 
// *        setup the lexer.
// *
// * INPUT: 
// *        iPortNum - Integer containing the port number to listen on.
// *                                                                  
// * RETURNS: 
// *      Nothing
// *                                                                  
// ************************************************************************
int ECI_Client::Connect(RWCString strHostName, int iPortNum)
{
    logf->debug(LOGSERVER) << "(ECI_Client) In Connect" << endline;

    pioGCore = new iosockinet(sockbuf::sock_stream);

	if (ioGCore()->connect(strHostName, iPortNum))
	{
	    cout << "Unable to make connection\n";
        return(0);
	}

    strHost = strHostName;
    iPort = iPortNum;

    cout << "localhost = " << strHost << endl
         << "localport = " << iPort << endl << flush;

    // ***************************
    // * Set current lexer state *
    // ***************************
    cout << "ECI Client: Successful connection" << endl
         << "Switching lexer streams with client socket.\n" << flush;

    lexc.SetSource(lexer_context::Stream, "<network>");
    lexc.SetLexerIO(ioGCore(), ioGCore());
    lexc.done = lexer_context::Ready;
    lexc.type = lexer_context::interactive;
    lexc.keystate = lexer_context::Everywhere;

    // ************************************************
    // * Initialize the granite core streaming socket *
    // ************************************************
    return(1);
}


void ECI_Client::Close( )
{
    logf->debug(LOGSERVER) << "(ECI_Client) In Close" << endline;

    ioGCore().clear();
    ioGCore()->close();    
}   

 
// ***********************************************************************
// *
// * NAME:    Write                     Public Function
// *
// * DESCRIPTION:
// *        Print an AuditRequest out the socket stream.
// *
// * INPUT: 
// *        arTheRequest - Pointer to the AuditRequest to send.
// *                                                                  
// * RETURNS: 
// *                                                                  
// ************************************************************************
int ECI_Client::Write( event *arTheRequest )
{
    CryptoError crc;
    char *eciCiphertext;
    int eciCipherTextLength; 


    logf->debug(LOGSERVER) << "(ECI_Client) In Write" << endline;
        
    if ( (dhdeSn != NULL) && (cryptoSubsystem != NULL) )
    {
        logf->debug(LOGSERVER) << "ECI_Client::Write(): Before processing ECI request"  << endline;

        ostrstream eciRequest;		// No buffer specification, causes ostrstream to manage any memory
                                        // requirements necessary for output operations.
                                        
            // Construct ECI request
        arTheRequest->print (eciRequest);

        if(ioGCore()->is_writeready(120))
        {
            logf->debug(LOGSERVER) << "ECI_Client::Write(): Before encrypt ECI request" << eciRequest.str() << endline;

            crc = cryptoSubsystem->encryptData (dhdeSn, pioGCore, eciRequest.str(), eciRequest.pcount());
            if (crc != CRYPTO_ERR_OK)
            {
                logf->debug(LOGSERVER) << "Fatal error: unable to encrypt ECI request; error = " << crc << endline;
                return (NULL);
            }

            logf->debug(LOGSERVER) << "ECI_Client::Write(): After encrypt ECI request (len = " << eciRequest.pcount() << ")"  << endline;
        }
        else
        {
            logf->error(LOGSERVER) << "(ECI_Client) ECI Write Timeout" << endline;
            return(NULL);
        }

        logf->debug(LOGSERVER) << "ECI_Client::Write(): After processing ECI request"  << endline;
    }
    else
    {
        arTheRequest->print(ioGCore());
    }
    

    lexc.dynamic_request = NULL;
    lexc.done = lexer_context::Ready;

    return(1);
}


// ***********************************************************************
// *
// * NAME:    IsValidRead               Public Function
// *
// * DESCRIPTION:
// *        Determines if the ECI read is valid
// *
// * INPUT: 
// *        None
// *                                                                  
// * RETURNS: 
// *        TRUE if valid ECI read
// *        FALSE otherwise
// *                                                                  
// ************************************************************************
int ECI_Client::IsValidRead(void)
{
    logf->debug(LOGSERVER) << "(ECI_Client) In ValidRead" << endline;

    return (lexc.done == lexer_context::GotECIReq && lexc.dynamic_request);
}


// ***********************************************************************
// *
// * NAME:    Read                      Public Function
// *
// * DESCRIPTION:
// *        Read ECI from the socket stream and parse it
// *
// * INPUT: 
// *        None
// *                                                                  
// * RETURNS: 
// *        An event containing the response.
// *        
// ************************************************************************
event *ECI_Client::Read(void)
{
    int nerrors;
    
    
    logf->debug(LOGSERVER) << "(ECI_Client) In Read" << endline;

    // ************************************
    // * Wait for a response and parse it *
    // ************************************
    if(ioGCore()->is_readready (coreReceiveTimeOut))
    {
        logf->debug(LOGSERVER) << "Have something to read from GC" << endline;

        if ( (dhdeSn != NULL) && (cryptoSubsystem != NULL) )
        {
            CryptoError crc;
            int eciPlainTextLength; 
            char *eciPlainText = NULL;
            istream& prevIn = lexc.In ();	// Preserve current lexer context input stream setting
            
            crc = cryptoSubsystem->decryptData (dhdeSn, (iosockinet *) &prevIn, &eciPlainText, &eciPlainTextLength);
            if (crc != CRYPTO_ERR_OK)
            {
               logf->debug(LOGSERVER) << "Fatal error: unable to decrypt ECI response; error = " << crc << endline;
               return (NULL);
            }

            // Set the lexer context input stream to input string stream
            // that contains the decrypted stream data.
            istrstream lexerInputStream (eciPlainText, eciPlainTextLength);
            lexc.SetLexerIO (lexerInputStream, lexc.Out());

            nerrors = parse_it (lexc);

            // Reset the lexer context input stream to the previous setting
            lexc.SetLexerIO (prevIn, lexc.Out());

            free (eciPlainText);		// deallocate buffer allocated for plain text data

            logf->debug(LOGSERVER) << "Returned from cryptography parse_it(): " << nerrors << endline;
            logf->debug(LOGSERVER) << "lexc.done == " << lexc.done << endline;
        }
        else
        {
            nerrors = parse_it(lexc);

            logf->debug(LOGSERVER) << "Returned from parse_it(): " << nerrors << endline;
            logf->debug(LOGSERVER) << "lexc.done == " << lexc.done << endline;
        }

        // ********************************
        // * Did we get any parse errors? *
        // ********************************
        if (nerrors)
            return(NULL);

        // ********************************
        // * Is this a valid ECI response *
        // ********************************
        if (!IsValidRead())
        {
            logf->error(LOGSERVER) << "(ECI_Client) Invalid ECI Read" << endline;
            return(NULL);
        }

        return lexc.dynamic_request; 
    }
    else
    {
        logf->error(LOGSERVER) << "(ECI_Client) ECI Read Timeout" << endline;
        return(NULL);
    }
}


// ***********************************************************************
// *
// * NAME:    GetReturnArguments        Public Function
// *
// * DESCRIPTION:
// *        Get the arguments from the returned audit request
// *
// * INPUT: 
// *        arTheReturn - Audit Request returned from the Granite Core.
// *                                                                  
// * RETURNS: 
// *        An event containing the response.
// *        
// ************************************************************************
event *ECI_Client::GetReturnArguments( event *arTheReturn )
{
    logf->debug(LOGSERVER) << "(ECI_Client) In GetReturnArguments" << endline;

	if (!((AuditRequest *) arTheReturn)->arguments)
		return(NULL);

    return(((AuditRequest *) arTheReturn)->arguments);
}

// ***********************************************************************
// *
// * NAME:    Execute                   Public Function
// *
// * DESCRIPTION:
// *        Calls write to print the AuditRequest to the core then calls 
// *        read to get the ECI response.
// *
// * INPUT: 
// *        arTheRequest - Pointer to the AuditRequest to send.
// *                                                                  
// * RETURNS: 
// *        An event containing the response.
// *                                                                  
// ************************************************************************
event *ECI_Client::Execute( event *arTheRequest )
{
    event *arTheReturn;

    logf->debug(LOGSERVER) << "(ECI_Client) In Execute" << endline;

    // ***********************
    // * Performance logging *
    // ***********************
    PerfLogger tmRequest;
    tmRequest.StartTime();

    if (!Write(arTheRequest))
        return(NULL);

    arTheReturn = Read();
    if (arTheReturn == NULL)
       return(NULL);

    // ***********************
    // * Performance logging *
    // ***********************
    tmRequest.EndTime();
    tmRequest.ReportPerf("Request", logf);
        
    return(GetReturnArguments(arTheReturn));
}

