#include <rw/cstring.h>
#include <stdlib.h>

#include "Log.h"
#include "Registrar.h"
#include "RegistrarRequestHandler.h"
#include "destiny.h"



// *************************************************************************
// **                      VIRTUAL MEMBER FUNCTION                         *
// **                                                                      *
// **             Cryptography SubSystem of Registrar specific             *
// *************************************************************************

void _CryptoSubSystem::logMessage (CryptoLogMsgType msgType, char *pstrMessage)
{
 
    switch (msgType)
    {
        case INFO:
            Log::println (Log::INFO, pstrMessage);
            break;

        case ERR:
            Log::println (Log::ERROR, pstrMessage);
            break;

        case DBUG:
        default:
            Log::println (Log::DEBUG, pstrMessage);
            break;
    }

}    /* end logMessage */


/**
 * Implementation of the Registrar daemon that listens on the 
 * specified port, waiting for requests.
 *
 * Usage:
 *    Registrar <config_file>
 *
 * $Id: Registrar.cc,v 1.4 1998/12/17 21:44:25 cking Exp $
 */

/**
 * Constructs a registrar.
 *
 * Params:
 *    address the host and port
 * Throws:
 *    IOException
 */
Registrar::Registrar (NetAddress nAddress, int encryptionSupport) : shutdownReq (FALSE),
    socket (sockbuf::sock_stream)
{
    encryption = encryptionSupport;
    socket.bind (nAddress.host, nAddress.port);
    socket.listen ();
    Log::println (Log::INFO, "listening on " + nAddress.toString ());
}

/**
 * Listens for connection requests and handles each in a
 * separate thread.
 */
void Registrar::listen ()
{
    CryptoError crc;
    CryptoDHDEsession  *dhdeSn = NULL;		    // diffie-hellman & digital hellman session descriptor
    _CryptoSubSystem cryptoSubsystem;	    // cryptography support subsystem


    if (encryption)
    {
            // initialize cryptography sub-system for encryption support.            
    
        if (cryptoSubsystem.initialize () != CRYPTO_ERR_OK)
        {
            Log::println (Log::INFO, RWCString ("Fatal error: unable to initialize cryptography sub-system, for encryption support."));
            return;
        }
    
            // Generate Diffie-Hellman parameters for peer nodes requesting the parameters.
            // This process can be time consuming, therefore, the generation of the is only 
            // done once, and stored for future requests.
        Log::println (Log::INFO, RWCString ("Before initializing DiffieHellman Parameters"));
        if (cryptoSubsystem.initializeDiffieHellmanParameters () != CRYPTO_ERR_OK)
        {
            Log::println (Log::INFO, RWCString ("Fatal error: unable to initialize generate Diffie-Hellman parameters for peer node requests."));
            return;
        }
        Log::println (Log::INFO, RWCString ("After initializing DiffieHellman Parameters"));
    }
    
    while (!shutdownReq || !_rlist.isEmpty ())
    {
        try
	{
	    iosockinet connection (socket.accept());
	    Log::println (Log::INFO, RWCString ("connected to client!"));

            if (encryption)
            {
                dhdeSn = new CryptoDHDEsession ();		    // diffie-hellman & digital hellman session descriptor
	        Log::println (Log::INFO, RWCString ("Before open cryptography session"));
                dhdeSn->setNodeType (CRYPTO_NODE_CENTRAL_AUTHORITY);		// set node type       
                dhdeSn->setPeerNodeType (CRYPTO_PEER_NODE_USER_ACTIVE);	// set node type       
                dhdeSn->setPeerNodeConnection (&connection);  // set socket connection to peer node

                    // Open cryptography session.
                if ((crc = cryptoSubsystem.openSession (dhdeSn)) != CRYPTO_ERR_OK)
                {
	            Log::println (Log::INFO, RWCString ("Fatal error: unable to open cryptography session"));
                    return;
                }
	        Log::println (Log::INFO, RWCString ("After open cryptography session"));
	    }

	    RegistrarRequestHandler handler (connection, _rlist);
	    handler.run ();

            if (encryption)
            {
	        Log::println (Log::INFO, RWCString ("Before close cryptography session"));
                cryptoSubsystem.closeSession (dhdeSn);
                delete (dhdeSn);
                dhdeSn = NULL;
	        Log::println (Log::INFO, RWCString ("After close cryptography session"));
	    }

	    if (handler._shutdownReq)
	    {
	        shutdownReq = TRUE;
	    }
	}

	// Log error and service next request.
	catch (RWCString s) {
	    Log::println (Log::ERROR, s);
	}
    }
    Log::println (Log::INFO, RWCString ("shutdown in progress..."));
}

#ifdef _R_MAIN

#include "RegistrarConfigFileParser.h"

/**
 * Application entry point.
 *
 * Params: args the command line arguments
 */
int main (int argc, char *argv[])
{
    // Check usage.
    switch (argc)
    {
    case 2:
        Log::setFile ("registrar.log", "Registrar Log File");
	break;
    case 3:
        Log::setFile (argv[2], "Registrar Log File");
	break;
    case 4:
        Log::setFile (argv[2], "Registrar Log File");
	Log::setLevel (argv[3]);
        break;
    default:
        cerr << "usage: Registrar <config_file> [silent]" << endl;
        exit (-1);
    }

    // Create and run registrar.
    try
    {
        RegistrarConfigFileParser rcfp (argv [1]);
	rcfp.open ();
	Registrar r (rcfp.getAddress (), rcfp.getUseCryptoGraphy ());
	r.listen ();
    }

    catch (RWCString s)
    {
        Log::println (Log::ERROR, s);
        exit (-1);
    }
}

#endif

