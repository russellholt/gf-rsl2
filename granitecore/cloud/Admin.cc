#include <rw/cstring.h>
#include <rw/ctoken.h>
#include "Admin.h"
#include "Log.h"

/**
 * The Granite Foundation administration utility.
 *
 * Usage:
 *    Admin <registrar_address>
 *
 * $Id: Admin.cc,v 1.1 1998/11/17 23:42:27 toddm Exp $
 */

/**
 * Continually prompts the user to enter Admin commands.
 */
void Admin::prompt ()
{
    RWBoolean quit = false;
    while (!quit) {
	// Read user command.
	cout << "Enter one of the following commands:" << endl;
	cout << "shutdown <seconds>     - shuts down all processes"
	     << endl;
	cout << "logsystems <subs>      - subsystems to log (a-g)" 
	     << endl;
	cout << "loglevel <level>       - 1-6; 1 = Fatal, ..., 6 = Debug" 
	     << endl;
	cout << "stat <seconds>         - seconds between statistics logging" 
	     << endl;
	cout << "maxusers <max>         - maximum users.  Set to 0 to disallow new logins." 
	     << endl;
	cout << "totalapps <total>      - max sessions that the server will process before" 
	     << endl;
	cout << "                         shutting down. set to 0 for unlimited apps." 
	     << endl;
	cout << "eci <eci_cmd>          - sends ECI message to all GCPs" 
	     << endl;
	cout << "reregister <brk_addr>  - re-registers the specified broker" 
	     << endl;
	cout << "quit                   - exits the Admin utility" << endl;
        cout << endl << '>' << flush;

	RWCString line;
	line.readLine (cin);

	// Parse command.
	RWCTokenizer st (line);
	RWCString cmd = st ();

	RWCString strArg = st ();
	if (strArg.isNull ())
	  strArg = "0";

	// Handle "shutdown" request.
	if (cmd.compareTo ("shutdown") == 0)
	  broadcast (line, true);

	// Handle "logsystems" request.
	else if (cmd.compareTo ("logsystems") == 0)
	  broadcast ("eci NPServer#Start.SetLogWhichSystems#reconfig(\"" +
		     strArg + "\");", false);

	// Handle "loglevel" request.
	else if (cmd.compareTo ("loglevel") == 0)
	  broadcast ("eci NPServer#Start.SetLogLevel#reconfig(" + 
		     strArg + ");", false);

	// Handle "stat" request.
	else if (cmd.compareTo ("stat") == 0)
	  broadcast ("eci NPServer#Start.SetLogStatInterval#reconfig(" +
		     strArg + ");", false);

	// Handle "maxusers" request.
	else if (cmd.compareTo ("maxusers") == 0)
	  broadcast ("eci NPServer#Start.SetMaxUsers#reconfig(" +
		     strArg + ");", false);

	// Handle "totalapps" request.
	else if (cmd.compareTo ("totalapps") == 0)
	  broadcast ("eci NPServer#Start.SetTotalAppsAllowed#reconfig(" +
		     strArg + ");", false);

	// Handle "eci" request.
	else if (cmd.compareTo ("eci") == 0)
	  broadcast (line, false);

	// Handle "reregister" request.
	else if (cmd.compareTo ("reregister") == 0) {
	  try {
	    NetAddress(strArg).query ("reregister");
	  }
	  catch (RWCString s) {
	    cout << "reregister: " << s << endl;
	  }
	}

	// Handle "quit" request.
	else if (cmd.compareTo ("quit") == 0)
	  quit = true;

	// Handle anything else.
	else
	  cout << "unknown command, ignoring" << endl;
    }
    cout << "exiting..." << endl;
}

/**
 * Gets the list of brokers from the registrar, sends them the
 * request then optionally sends the request to the registrar.
 *
 * Params:
 *    message - message to broadcast
 *    includeReg - whether or not registrar should be sent the message
 */
void Admin::broadcast (const RWCString& message, RWBoolean includeReg)
{
    try
    {
        RegistrationList *rList = getBrokers ();
	for (int i = 0; i < rList->size (); i++)
	{
	    (*rList)[i].query (message);
	}
	if (includeReg)
	    _regAddr.query (message);
	delete rList;
    }

    catch (RWCString s)
    {
        Log::println (s);
    }
}

/**
 * Gets the list of registered brokers.
 *
 * Returns:
 *    broker list
 * Throws:
 *    IllegalArgumentException
 *    IOException
 */
RegistrationList *Admin::getBrokers ()
{
    // Send 'reqBlist' request.
    RWCString response = _regAddr.query ("reqBlist");

    // Parse response.
    RWCTokenizer st (response);
    
    RWCString blist = st ();
    if (blist.isNull ())
      throw new RWCString ("IllegalArgumentException: BlistRequestError: missing b-list");

    if (blist.compareTo ("blist") != 0)
      throw new RWCString ("IllegalArgumentException: BlistRequestError: invalid registrar response");

    // Return registration list.
    return new RegistrationList (st ());
}

/**
 * Application entry point.  Creates an administrator and kicks
 * it off.
 *
 * Params: args the command line arguments
 */
int main (int argc, char *argv[])
{
    // Check usage.
    switch (argc) {
      case 2:
	Log::setOn ();
	break;
      default:
        Log::println ("usage: java Admin <registrar_address>");
        exit (-1);
    }

    // Create broker using configuration file settings.
    try {
      Admin* a = new Admin (NetAddress (argv[1]));
      a->prompt ();
    }

    // If an error creating or running the adminstrator,
    // print a message and quit.
    catch (RWCString s) {
      Log::println (s);
    }
}
