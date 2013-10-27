#include <rw/cstring.h>
#include <fstream.h>

#include "destiny.h"

#ifndef _GcpManager_H_
#define _GcpManager_H_

/**
 * The Granite Core Process manager.  Creates a GCP and an associated
 * named pipe, watches the GCP and restarts it if it crashes.
 *
 * $Id: GcpManager.h,v 1.1 1998/11/17 23:43:45 toddm Exp $
 */
class GcpManager {

  //--------------------------- PUBLIC -------------------------------
  public:

    /**
     * Constructs a GCP manager.
     *
     * Params:
     *    brokerId - the managing broker's identifier
     *    gcpId - the id of the Core Process to create
     */
    GcpManager (const RWCString& brkId, const RWCString& gcpId,
		const RWCString& gcpRunScript):
        _brkId (brkId), _gcpId (gcpId), _shutdown (FALSE), _fifo (NULL),
        _gcpRunScript (gcpRunScript) {}

    /**
     * Destroys a GCP manager.
     */
    ~GcpManager () { if (_fifo) _fifo->close (); }

    /**
     * Gets the FIFO name corresponding to the given broker and GCP.
     *
     * Params:
     *    brokerId - the broker's id
     *    gcpId - the GCP's id
     * Returns:
     *    the FIFO name
     */
    static RWCString fifoName (const RWCString& brokerId, 
			       const RWCString& gcpId)
        { return brokerId + RWCString ("/") + gcpId; }

    /**
     * Creates a child process that executes the specified command.
     *
     * Params:
     *    cmd - the command to execute
     *    args - the arguments to the command.
     */
    void forkExec (const RWCString& cmd, const RWCString& args);

    /**
     * Creates a FIFO named <bId>/<gcpId>, e.g., "B1/G1" and a
     * corresponding Granite Core Process.  Watches the Process,
     * restarting it if necessary.  Returns when its GCP gracefully
     * shuts down, or on error.
     */
    void run ();

    /**
     * Writes a line to the FIFO.
     *
     * Params: 
     *    obj - the object to write
     */
    void println (RWCString s);

    /**
     * Notifies this manager that its GCP is being stopped.
     */
    void shutdown () { _shutdown = TRUE; }

  //--------------------------- PRIVATE ------------------------------
  private:

    /**
     * The Granite Core Process identifier.
     */
    RWCString _gcpId;

    /**
     * The managing Broker's identifier.
     */
    RWCString _brkId;

    /**
     * The Granite Core Process run script.
     */
    RWCString _gcpRunScript;

    /**
     * Flag signifying that the GCP is being shutdown.
     */
    RWBoolean _shutdown;

    /**
     * The FIFO associated with the GCP being managed.
     */
    ofstream *_fifo;
};

#endif
