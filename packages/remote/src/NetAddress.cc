#include "NetAddress.h"
#include <stream.h>
#include <rw/ctoken.h>
#include <stdlib.h>
#include "sockinet.h"
#include <time.h>

/**
 * NetAddress implementation.
 *
 * $Id: NetAddress.cc,v 1.2 1998/11/23 19:59:05 cking Exp $
 */

/**
 * Constructs a network address from an address string.
 *
 * Params:
 *     addrStr - the address string, e.g., "host:port"
 */
NetAddress::NetAddress (const RWCString& addrStr)
{
  RWCTokenizer st (addrStr);
  host = st (":");
  RWCString portStr = st (":");
  port = atoi (portStr.data ());
  cryptoSubsystem = NULL;
}

/**
 * Sends a message to the server at this address and gets the response.
 *
 * Params:
 *     message - the query
 *     timeout - the timeout in seconds when reading the response
 *               (-1 = block, 0 = poll)
 * Returns:
 *     the response (or the null string if the connection fails)
 */
RWCString NetAddress::query (const RWCString& message, 
			     const int timeout) const
{
  // Connect.  Return null string if failure.
  iosockinet sio (sockbuf::sock_stream);
  if (sio->connect (host, port) != 0)
    return RWCString ();

    CryptoError crc;
    CryptoDHDEsession dhdeSn;		    		// diffie-hellman & digital hellman session descriptor
    int cryptography = FALSE;
    
    if (cryptoSubsystem != NULL)
    {
        RWCTokenizer st (message);

	    // Get request.  Format: <request> [<arg1> ...]
	    // The request should never be null, but check just in case.
        RWCString request = st ();

        if ( (request.compareTo ("register") == 0) ||
             (request.compareTo ("unregister") == 0) ||
             (request.compareTo ("reqBlist") == 0) ||
             (request.compareTo ("shutdown") == 0) ||
             (request.compareTo ("reregister") == 0) )
        {
            cryptography = TRUE;        
            dhdeSn.setNodeType (CRYPTO_NODE_USER_PARTY);			// set node type       
            dhdeSn.setPeerNodeType (CRYPTO_PEER_NODE_CENTRAL_AUTHORITY);	// set peer node type       
            dhdeSn.setPeerNodeConnection (&sio);				// set socket connection to remote peer
            dhdeSn.setCentralAuthorityNodeConnection (&sio);	        	// set socket connection to CA        

                // Open cryptography session.
            if ((crc = cryptoSubsystem->openSession (&dhdeSn)) != CRYPTO_ERR_OK)
            {
//              printf ("Fatal error: unable to open cryptography session\n");
                return RWCString ();
            }
        }
    }

  // Sets the read timeout, sends message and reads one line response.
  sio << message << endl;
  RWCString response;
  int oldTO = sio->recvtimeout (timeout);
  response.readLine (sio);
  sio->recvtimeout (oldTO);

  if (cryptography)
  {
      cryptoSubsystem->closeSession (&dhdeSn);
  }

  return response;
}

#ifdef _NA_MAIN_1

/**
 * For unit testing only!
 */
void makeQuery (char *address) {
  NetAddress addr (address);
  long start, end, diff, min = 100000, max = 0;
  while (true) {
    start = time (NULL);
    RWCString request ("reqBlist");
    for (int i = 0; i < 100; i++) {
      addr.query (request);
    }
    end = time (NULL);
    diff = end - start;
    if (diff > max) {
      max = diff;
      cout << "Min: " << min << ", Max: " << max << endl;
    }
    if (diff < min) {
      min = diff;
      cout << "Min: " << min << ", Max: " << max << endl;
    }
    sleep (1);
  }
}

main (int argc, char *argv[]) {
  if (argc != 3) {
    cerr << "usage: NetAddress host:port <no_of_processes>" << endl;
    exit (-1);
  }
  int count = atoi (argv[2]);
  for (int i = 0; i < count; i++) {
    pid_t pid = fork ();
    if (pid < 0) {
      cerr << "error forking" << endl;
      exit (-1);
    }
    else if (pid == 0) {
      makeQuery (argv[1]);
      exit (0);
    }
  }
}

#endif

#ifdef _NA_MAIN_2

/**
 * For unit testing only!
 */
main (int argc, char *argv[]) {

  if (argc != 2) {
    cerr << "usage: NetAddress host:port" << endl;
    exit (-1);
  }

  NetAddress addrtmp (argv[1]);
  NetAddress addr (addrtmp);
  cout << "Using " << addr.toString () << endl;
  long start, end, diff, min = 100000, max = 0;
  while (true) {
    time (&start);
    RWCString request ("");
    cout << "Enter query" << endl;
    request.readLine (cin);
    cout << addr.query (request) << endl;
    time (&end);
    diff = end - start;
    if (diff > max) {
      max = diff;
      cout << "Min: " << min << ", Max: " << max << endl;
    }
    if (diff < min) {
      min = diff;
      cout << "Min: " << min << ", Max: " << max << endl;
    }
  }
}

#endif
