#include <stream.h>
#include <sockinet.h>
#include <rw/cstring.h>
#include <rw/ctoken.h>

#include "Broker.h"
#include "SessionId.h"
#include "destiny.h"



// *************************************************************************
// **                      VIRTUAL MEMBER FUNCTION                         *
// **                                                                      *
// **             Cryptography SubSystem of Broker specific                *
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
 * The Process Broker daemon of the Granite Foundation Network and
 * Process Architecture.
 *
 * Usage:
 *    java Broker <config_file> [silent]
 *
 * $Id: Broker.cc,v 1.3 1998/11/30 20:29:16 cking Exp $
 */

/**
 * Registers with the registrar, starts the Granite Core Processes,
 * then listens for requests and services them.
 *
 * Params:
 *    gcpRunScript - the Granite Core run script
 * Throws:
 *    IOException 
 */
void Broker::run (const RWCString& gcpRunScript)
{
    _CryptoSubSystem cryptoSubsystem;		// cryptography support subsystem


    if (_encryption)
    {
        // initialize cryptography sub-system for encryption support.            
        if (cryptoSubsystem.initialize () != CRYPTO_ERR_OK)
        {
            Log::println (Log::INFO, "Fatal error: unable to initialize cryptography sub-system, for encryption support.");
            return;
        }
    
        _regAddr.cryptoSubsystem = &cryptoSubsystem;
        Log::println (Log::INFO, "Granite core encryption support enabled.");
    }
        
    // Register with the registrar.
    registerMe ();

    // Create Granite Core Processes managers and kick them off.
    for (int i = 0; i < _gcpCount; i++)
    {
        _gcpMgrs[i] = new GcpManager (_id, gcpId (i), gcpRunScript);
	_gcpMgrs[i]->run ();
    }

    // Service requests from channels and other brokers.  If one
    // request fails, don't terminate, just continue on to the next one.
    sockinetbuf brkSocket (sockbuf::sock_stream);
    brkSocket.bind (_brkAddr.host, _brkAddr.port);
    brkSocket.listen ();
    Log::println (Log::INFO, "listening on " + _brkAddr.toString ());

    while (!_shutdown)
    {
        try
	{
	    Log::println (Log::NOTICE, "waiting for request ");
	    iosockinet connection (brkSocket.accept());
	    Log::println (Log::NOTICE, "handling request ");
	    Log::println (Log::INFO, "connected to client!");
	    handleRequest (connection);
	}
	catch (RWCString s)
	{
	    Log::println (Log::ERROR, s);
	}
    }
}

/**
 * Registers with the registrar.  Sends request "register <host>:<port>".
 * Gets response "registered <broker_id>".
 *
 * Throws:
 *    IOException
 */
void Broker::registerMe ()
{
    Log::println (Log::INFO, "Broker " + _brkAddr.toString () + 
		 " registering with " + _regAddr.toString ());
    RWCString response = _regAddr.query ("register " + _brkAddr.toString ());
    RWCTokenizer st (response);
    RWCString cmd = st ();

    // If no response, registration failed.  Raise exception.
    if (cmd.isNull ())
    {
        throw new RWCString (
	   "registration failed, registrar not responding");
    }

    if (cmd.compareTo ("registered") == 0)
    {
        _id = st ();
        if (_id.isNull ())
	{
	    throw new RWCString ("registration failed, no identifier received from registrar");
	}
    }	

    // If error returned, registration failed.  Raise exception.
    else if (response.compareTo ("error") == 0)
    {
        throw new RWCString (
	       "registration failed, " + st ());
    }

    // If something else returned, registration failed.  Raise exception.
    else
    {
        throw new RWCString (
	  "registration failed, unexpected response from registrar");
    }
}

/**
 * Removes this broker from the registry.  Sends request 
 * "unregister <bId>".  Gets response "success".
 */
void Broker::unregisterMe ()
{
    try
    {
        Log::println (Log::INFO, "Broker " + _brkAddr.toString () + 
		     " unregistering with " + _regAddr.toString ());
	RWCString response = _regAddr.query ("unregister " + _id);
	
	// If no response, unregistration failed.  Raise exception.
	if (response.compareTo ("success") != 0)
	{
	    Log::println (Log::ERROR,
	       "unregistration failed, registrar not responding");
	}
    }
    catch (RWCString s)
    {
      Log::println (Log::ERROR, "unregistration failed, " + s);
    }
}

/**
 * Sends all active GCPs a shutdown message.
 *
 * Params:
 *    seconds - seconds until shutdown
 */
void Broker::sendShutdownToGcps (int seconds)
{
    char strSecs [10];
    sprintf (strSecs, "%d", seconds);
    RWCString eciCmd = RWCString ("NPServer#Start.Shutdown#reconfig(") +
      RWCString (strSecs) + RWCString (");");
    for (int i = 0; i < _gcpCount; i++) {
      _gcpMgrs[i]->println (eciCmd);
    }
}

/**
 * Handles requests from channels and other brokers.
 *
 * Throws:
 *    IOException 
 */
void Broker::handleRequest (iostream& _stream)
{
    // Read and parse request.
    RWCString line;
    line.readLine (_stream);

    // If nothing to read, log error and quit.
    if (line.isNull ())
    {
        _stream << "error StreamFormatError: no request"
		<< endl << flush;
	Log::println (Log::INFO, RWCString ("sent message \"") + 
	     RWCString ("error StreamFormatError: no request\""));
	return;
    }

    Log::println (Log::INFO, RWCString ("received message \"")
    		  + line + RWCString ("\""));
    RWCTokenizer st (line);

    // Get request.  Format: <request> [<arg1> ...]
    // The request should never be null, but check just in case.
    RWCString request = st ();
    if (request.isNull ())
    {
        _stream << "error StreamFormatError: no request"
		<< endl << flush;
	Log::println (Log::ERROR, RWCString ("sent message \"error ")
		      + RWCString ("StreamFormatError: no request\""));
	return;
    }

    // We have a request, now figure out what it is.

    // Handle "connect session_id chost:cport" request.
    if (request.compareTo ("connect") == 0)
    {
        // Get session id.
        RWCString strSId = st ();
        if (strSId.isNull ())
	{
	    _stream << "error StreamFormatError: missing session id"
		    << endl << flush;
	    Log::println (Log::ERROR, "sent message \"error StreamFormatError: missing session id\"");
	    return;
	}

	SessionId sId (strSId);
	/*
	try
	{
	    sId = new SessionId (strSId);
	}
	catch (RWCString s)
	{
	    _stream << "error " << s << endl << flush;
	    Log::println (Log:ERROR, "sent message \"error " + s + "\"");
	    return;
	}
	*/

	// Get net address.
	RWCString strAddr = st ();
	if (strAddr.isNull ())
	{
	    _stream << "error StreamFormatError: missing host:port"
		    << endl << flush;
	    Log::println (Log::ERROR, "sent message \"error StreamFormatError: missing host:port\"");
	    return;
	}

	NetAddress chAddr (strAddr);
	/*
	NetAddress chAddr;
	try
	{
	    chAddr = new NetAddress (strAddr);
	}
	catch (RWCString s)
	{
	    _stream << "error " + s << endl << flush;
	    Log::println (Log::ERROR, "sent message \"error " + s + "\"");
	    return;
	}
	*/

	// Message received correctly.  Send an acknowledgement
	// even though we don't know whether or not the request
	// is carried out correctly.
	_stream << "success" << endl << flush;
	Log::println (Log::INFO, "sent message \"success\"");

	// Determines if the session is managed by this broker.
	// If so, write message to the appropriate named pipe.
	// Message is in ECI format.
	if (sId.inBroker (_id))
	{
	    char strPort [10];
	    sprintf (strPort, "%d", chAddr.port);
	    RWCString eciCmd = RWCString ("ChannelConnection#1.connect#2(\"")
	      + chAddr.host + RWCString ("\",\"") + strPort + 
	      RWCString ("\");");
	    _gcpMgrs [gcpIndex (sId.gcpId)]->println (eciCmd);
	}

	// If not, forward to appropriate broker.
	else
	{
	    // Try current b-list.
	    NetAddress *destBrkAddr = NULL;
	    if (_blist != NULL)
	        destBrkAddr = &_blist->lookup (sId.brokerId);

	    // If broker is not in current list, get updated list.
	    if (destBrkAddr == NULL)
	    {
	        RWCString response = _regAddr.query ("reqBlist");
		RWCTokenizer st2 (response);
		RWCString cmd = st2 ();
		if (cmd.isNull ())
		{
		    throw RWCString ("IllegalArgumentException: BlistRequestError: missing b-list");
		    return;
		}

		if (cmd.compareTo ("blist") == 0)
		{
		    RWCString strBlist = st2 ();
		    if (strBlist.isNull ())
		    {
		        throw RWCString ("IllegalArgumentException: BlistRequestError: missing b-list");
                        return;
		    }
		    _blist = new RegistrationList (strBlist);
		    destBrkAddr = &_blist->lookup (sId.brokerId);
		}
	    }
		
	    // If broker still not in current list, raise exception.
	    if (destBrkAddr == NULL)
	    {
	        throw new RWCString ("BrokerRefError: invalid broker id");
	    }

	    // Otherwise, connect to broker and send request.
	    // Don't bother checking results, it's too late to
	    // notify channel of error.
	    else
	    {
	        RWCString response = destBrkAddr->query (line);
	    }
	}
    }
    
    // Handle "create" request.
    // Generate a session id and return it.
    else if (request.compareTo ("create") == 0)
    {
	RWCString newId = SessionId::genSessionId (_id, _gcpCount);
	_stream << "session " << newId << endl << flush;
	Log::println (Log::INFO, "sent message \"session " + newId + "\"");
    }
      
    // Handle "shutdown" request.  Tell each GCP to shutdown and
    // wait until they actually do, then unregister and quit.
    else if (request.compareTo ("shutdown") == 0)
    {
        int seconds = 0;
	RWCString strSecs = st ();
	if (!strSecs.isNull ())
	{
	    seconds = atoi (strSecs);
	}

	_stream << "success" << endl << flush;
	Log::println (Log::INFO, "sent message \"success\"");

	// Notify GCPs, prep GCP managers for shutdown, then wait.
	sendShutdownToGcps (seconds);
	for (int i = 0; i < _gcpMgrs.length (); i++)
	    _gcpMgrs[i]->shutdown ();
	Log::println (Log::INFO, "notified GCPs, waiting to end");
	/*
	for (int i = 0; i < _gcpMgrs.length (); i++)
	{
	    try
	    {
	        _gcpMgrs[i]->join ();
	    }
	    catch ( e)
	    {
		Log::println (Log::ERROR, "error " + e);
	    }
	}
	*/
	  
	// Unregister this broker.
	Log::println (Log::INFO, "unregistering");
	unregisterMe ();
	Log::println (Log::INFO, "shutdown in progress...");
	_shutdown = TRUE;
    }

    // Handle "eci" request.  Broadcast to each GCP.
    else if (request.compareTo ("eci") == 0)
    {
        _stream << "success" << endl << flush;
	Log::println (Log::INFO, "sent message \"success\"");

	// Send request to GCPs.
	for (int i = 0; i < _gcpMgrs.length (); i++)
	{
	    _gcpMgrs[i]->println (RWCString (line.data () + 4));
	}
    }

    // Handle "reregister" request.
    else if (request.compareTo ("reregister") == 0)
    {
	RWCString response = _regAddr.query (RWCString ("reregister ")
			    + _id + RWCString (" ") + _brkAddr.toString ());
	if (response.compareTo ("success") == 0)
	{
	    _stream << "success" << endl << flush;
	    Log::println (Log::INFO, "sent message \"success\"");
	}
	else
	{
	    _stream << response << endl << flush;
	    Log::println (Log::ERROR, "error: " + response);
	}
    }

    // Invalid request.  Notify channel and raise exception.
    else
    {
        _stream << "error StreamFormatError: request not understood"
		<< endl << flush;
	Log::println (Log::ERROR, RWCString ("sent message \"error StreamFormatError: request not understood\""));
    }
} 

#ifdef _B_MAIN

#include "BrokerConfigFileParser.h"

/**
 * Application entry point.  Creates a Broker and kicks it off.
 *
 * Params: args the command line arguments
 */
int main (int argc, char *argv[])
{
    Log::init ();

    // Check usage.
    switch (argc)
    {
    case 2:
        Log::setFile ("broker.log", "Broker Log File");
	break;
    case 3:
        Log::setFile (argv[2], "Broker Log File");
	break;
    case 4:
        Log::setFile (argv[2], "Broker Log File");
        Log::setLevel (argv[3]);
	break;
    default:
        cout << "usage: Broker <config_file> [<logfile> [<loglevel>]]" << endl;
        exit (-1);
    }

    // Create broker using configuration file settings.
    try {
        BrokerConfigFileParser bcfp (argv [1]);
	bcfp.open ();
	Broker b (bcfp.getBrokerAddr (), bcfp.getRegistrarAddr (),
		  bcfp.getGcpCount (), bcfp.getUseCryptoGraphy ());
	b.run (bcfp.getGcpRunScript ());
    }

    // If an error parsing the configuration file or creating a broker,
    // print a message and quit.
    catch (RWCString s) {
      Log::println (Log::ERROR, s);
    }
}

#endif
