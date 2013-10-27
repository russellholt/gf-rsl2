#include <rw/cstring.h>
#include <sys/times.h>

#ifndef _SessionId_H_
#define _SessionId_H_

/**
 * A session identifier, consisting of a broker id, a granite core
 * process id and a local session id.  For example, B1G23S892948.
 *
 * $Id: SessionId.h,v 1.1 1998/11/17 23:11:43 toddm Exp $
 */
class SessionId {

  //--------------------------- PUBLIC -------------------------------
  public:

    /**
     * Creates a session identifier from a string.
     *
     * Params:
     *    sId - the session id string
     * Throws:
     *    IllegalArgumentException
     */
    SessionId (const RWCString& sId);

    /**
     * Returns the string representation of this session id.
     *
     * Returns:
     *    the id as a string
     */
    RWCString toString () const { return brokerId + gcpId + localsId; }

    /**
     * Determines if this session is managed by the specified broker.
     *
     * Params:
     *    bId - the broker's id
     * Returns:
     *    true if the session is managed by the broker
     */
    RWBoolean inBroker (const RWCString& bId) const
                 { return (brokerId.compareTo (bId) == 0); }

    /**
     * The broker id.
     */
    RWCString brokerId;

    /**
     * The GCP id.
     */
    RWCString gcpId;

    /**
     * The local session id.
     */
    RWCString localsId;

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
    static RWCString genSessionId (const RWCString& brkId, int GCPcount);
};

#endif



