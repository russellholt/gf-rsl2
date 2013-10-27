#include <Fork.h>
#include <rw/cstring.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include "GcpManager.h"
#include "Log.h"
#include "ProcessMgr.h"

/**
 * The Granite Core Process manager.  Creates a GCP and an associated
 * named pipe, watches the GCP and restarts it if it crashes.
 *
 * $Id: GcpManager.cc,v 1.1 1998/11/17 23:42:39 toddm Exp $
 */

/**
 * Creates a FIFO named <bId>/<gcpId>, e.g., "B1/G1" and a
 * corresponding Granite Core Process.  Watches the Process,
 * restarting it if necessary.  Returns when its GCP gracefully
 * shuts down, or on error.
 */
void GcpManager::run ()
{
    RWCString fname = fifoName (_brkId, _gcpId);

    // Create FIFO directory.
    mkdir (_brkId, 0775);
    
    // Create a FIFO.
    if (mkfifo (fname, 0666) == 0)
    {
	Log::println (Log::INFO, "made fifo");
	Log::println (Log::INFO, "starting GCP " + _gcpId + ".");
	ProcessMgr::createChild (_gcpRunScript, fname, 
				 ProcessMgr::KeepAlive);
    }
    else
    {
        Log::println (Log::ERROR, "failed making fifo" + fname);
    }

    // Regardless of result of mkfifo, open the fifo
    // for reading.
    _fifo = new ofstream (fname);
    if (_fifo == NULL)
    {
        Log::println (Log::ERROR, "error opening fifo " + fname);
	return;
    }
}

/**
 * Writes a line to the FIFO.
 *
 * Params:
 *    string - the string to write
 */
void GcpManager::println (RWCString s)
{
    if (_fifo != NULL)
    {
        Log::println (Log::INFO, "writing to " + _gcpId + 
		     " FIFO \"" + s + "\"");
	(*_fifo) << s << endl << flush;
    }
    else
    {
        Log::println (Log::ERROR, "Error writing to fifo!");
    }
}
