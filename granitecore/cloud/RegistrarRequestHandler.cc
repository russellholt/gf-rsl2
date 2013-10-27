#include <rw/cstring.h>
#include <rw/ctoken.h>

#include "Log.h"
#include "RegistrarRequestHandler.h"
#include "destiny.h"

/**
 * Implementation of the handler for Registrar requests.  It
 * reads a request from a socket connection, determines the type
 * of request and handles it.  The two types are registration
 * requests and broker-list requests.
 *
 * $Id: RegistrarRequestHandler.cc,v 1.1 1998/11/17 23:42:57 toddm Exp $
 */

/** 
 * Main body of thread's execution.  Reads a request from the
 * stream, interprets it and services the request. 
 */
void RegistrarRequestHandler::run ()
{
    try
    {
        // Read and parse line.
        RWCString line;
	line.readLine (_stream);

	// If nothing to read, log error and quit.
	if (line.isNull ())
        {
	    _stream << "error StreamFormatError: no request"
		    << endl << flush;
	    Log::println (Log::ERROR, RWCString ("sent message \"") + 
		  RWCString ("error StreamFormatError: no request\""));
	    return;
	}

	Log::println (Log::INFO, "received message \"" + line + "\"");
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

	// Handle "register host:port" request.
	// Register the host:port and return the newly assigned
	// broker id.
	if (request.compareTo ("register") == 0)
	{
	    // Get address.
	    RWCString addr = st ();
	    if (addr.isNull ())
	    {
	        _stream << "error StreamFormatError: missing host:port"
		        << endl << flush;
		Log::println (Log::ERROR, RWCString ("sent message \"error ")
		   + RWCString ("StreamFormatError: missing host:port\""));
		return;
	    }
	    
	    // Add to registration list.
	    RWCString bid = _rlist.add (NetAddress (addr));
	    _stream << "registered " << bid << endl << flush;
	    Log::println (Log::INFO, RWCString ("sent message \"registered ")
			  + bid + RWCString ("\""));
	}	    
		
	// Handle "unregister bid" request.
	// Remove the broker from the registry.
	else if (request.compareTo ("unregister") == 0)
	{
	    // Get broker id.
	    RWCString bid = st ();
	    if (bid.isNull ())
	    {
	        _stream << "error StreamFormatError: missing id" 
			<< endl << flush;
		Log::println (Log::ERROR, RWCString ("sent message \"error ")
			+ RWCString ("StreamFormatError: missing id\""));
		return;
	    }

	    // Delete from registration list.
	    if (_rlist.remove (bid))
	    {
	        _stream << "success" << endl << flush;
		Log::println (Log::INFO, RWCString ("sent message \"")
			      + RWCString ("success\""));
	    }
	      
	    // If unregister fails, post warning.
	    else
	    {
	        _stream << "warning OpFailWarning: broker not found"
			<< endl << flush;
		Log::println (Log::ERROR, RWCString ("sent mesage \"warning ")
			+ RWCString ("OpFailWarning: broker not found\""));
		return;
	    }
	}

	// Handle "reqBlist" request.
	// Return a randomly ordered list of brokers.
	else if (request.compareTo ("reqBlist") == 0)
	{
	    _stream << "blist " << _rlist.toString () << endl << flush;
	    Log::println (Log::INFO, RWCString ("sent message \"blist ")
			  + _rlist.toString () + RWCString ("\""));
	}

	// Handle "shutdown" request.
	else if (request.compareTo ("shutdown") == 0)
	{
	    _shutdownReq = TRUE;
	    _stream << "success" << endl << flush;
	    Log::println (Log::INFO, RWCString ("sent message \"success")
			  + RWCString ("\""));
	}

	// Handle "reregister bId host:port" request.
	// Re-register the named broker.
	else if (request.compareTo ("reregister") == 0)
	{
	    // Get broker id.
	    RWCString bId = st ();
	    if (bId.isNull ())
	    {
	        _stream << "error StreamFormatError: missing bId"
		        << endl << flush;
		Log::println (Log::ERROR, RWCString ("sent message \"error ")
			 + RWCString ("StreamFormatError: missing bId\""));
		return;
	    }

	    // Get net address.
	    RWCString addr = st ();
	    if (addr.isNull ())
	    {
	        _stream << "error StreamFormatError: missing host:port"
		        << endl << flush;
		Log::println (Log::ERROR, RWCString ("sent message \"error ")
		   + RWCString ("StreamFormatError: missing host:port\""));
		return;
	    }
	    
	    _rlist.add (bId, NetAddress (addr));
	    _stream << "success" << endl << flush;
	    Log::println (Log::INFO, RWCString ("sent message \"success\""));
	}
	
	// Invalid request.  Log error and give up.
	else
	{
	    _stream << "error StreamFormatError: request not understood"
		    << endl << flush;
	    Log::println (Log::ERROR, RWCString ("sent message \"error ") + 
		RWCString ("StreamFormatError: request not understood\""));
	}
    } 

    // If request invalid for other reasons, log error and quit.
    catch (RWCString s) {
      _stream << "error " << s << endl << flush;
      Log::println (Log::INFO, RWCString ("sent message \"error ")
		    + s + RWCString ("\""));
    }
}


