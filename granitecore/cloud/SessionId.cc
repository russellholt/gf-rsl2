#include <rw/cstring.h>
#include <rw/ctoken.h>
#include <stdlib.h>
#include <stdio.h>
#include "SessionId.h"

/**
 * Implementation of a session identifier, consisting of a broker id,
 * a granite core process id and a local session id.  For example, 
 * B1G23S892948.
 *
 * $Id: SessionId.cc,v 1.1 1998/11/17 23:43:02 toddm Exp $
 */

/**
 * Creates a session identifier from a string.
 *
 * Params:
 *    sId - the session id string
 * Throws:
 *    IllegalArgumentException
 */
SessionId::SessionId (const RWCString& sId)
{
    RWCTokenizer st (sId);
    
    // Extract broker id.
    brokerId = st ("GS");
    if (brokerId.isNull ())
    {
        throw RWCString ("IllegalArgumentException: no broker id");
    }

    // Extract GCP id.
    gcpId = "G" + st ("GS");
    if (gcpId.isNull ())
    {
	throw RWCString ("IllegalArgumentException: no gcp id");
    }

    // Extract session id.
    localsId = "S" + st ("GS");
    if (localsId.isNull ())
    {
	throw RWCString ("IllegalArgumentException: no session id");
    }
}


/**
 * Generates a new, unique session identifier.  A GCP is selected
 * at random and a local session id is based on a timestamp.
 * 
 * Params: 
 *    brkId - the managing broker's id
 * Params: 
 *    GCPcount - the number of GCP's managed by the broker
 * Returns:
 *    the session id
 */
RWCString SessionId::genSessionId (const RWCString& brkId, 
					  int GCPcount)
{
    struct tms timeInfo;
    char gsId [100];
    sprintf (gsId, "G%dS%ld", 1 + rand () % GCPcount, times (&timeInfo));
    return brkId + RWCString (gsId);
}

#ifdef _SI_MAIN_

#include <stream.h>
main (int argc, char *argv[])
{
    if (argc == 1)
    { 
    char strSid [100];
    cout << "Enter sId >";
    cin >> strSid;
    SessionId sid (strSid);
    cout << "SID: ***" << sid.toRWCString () << "***" << endl;
    cout << "BID: ***" << sid.brokerId << "*** GID: ***"
	 << sid.gcpId << "*** LSID: ***" << sid.localsId << "***" << endl;
    }
    else if (argc == 3)
    {
    while (1)
    {
    SessionId sid (genSessionId (argv[1], atoi (argv[2])));
    cout << "SID: ***" << sid.toRWCString () << "***" << endl;
    cout << "BID: ***" << sid.brokerId << "*** GID: ***"
	 << sid.gcpId << "*** LSID: ***" << sid.localsId << "***" << endl;
    char strSid [100];
    cin >> strSid; // ignored
    }
    }
}

#endif
