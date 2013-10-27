#include <rw/collint.h>
#include <rw/cstring.h>
#include <rw/tvhdict.h>
#include <stream.h>

#ifndef _ForkExec_H_
#define _ForkExec_H_

/**
 * A general process manager.  Creates child processes via fork-exec.
 * Keeps track of child process status.  Uses singleton pattern.
 *
 * $Id: ProcessMgr.h,v 1.1 1998/11/17 23:44:00 toddm Exp $
 */
class ProcessMgr
{
//--------------------------- PUBLIC -------------------------------
public:

  /**
   * Monitors child process and restart if necessary.
   */
  enum { KeepAlive = 1 };

  /**
   * Creates a child process.
   *
   * Params:
   *    cmd - the program to run
   *    args - the command's arguments
   *    mode - the monitoring mode (no monitoring by default)
   */
  static void createChild (const RWCString& cmd, 
			   const RWCString& args, int mode = 0)
      { manager.newChild (cmd, args, mode); }

//--------------------------- PRIVATE ------------------------------
private:

  /**
   * Child program to run.
   */
  typedef struct
  {
    RWCString cmd;
    RWCString args;
  } ExecCommand;
  
  /**
   * Constructs a process manager.
   */
  ProcessMgr (): monitors (ProcessMgr::hash) {}

  /**
   * Creates a child process.
   *
   * Params:
   *    cmd - the program to run
   *    args - the command's arguments
   *    mode - the monitoring mode
   */
  void newChild (const RWCString& cmd, const RWCString& args, int mode);

  /**
   * The list of child processes being monitored.
   */
  RWTValHashDictionary<RWCollectableInt, ExecCommand> monitors;

  /**
   * A hash function for RWCollectableInt.
   */
  static unsigned hash (const RWCollectableInt& cint);

  /**
   * A handler for the SIGCLD signal.
   */
  static void sigcldHandler (int i);

  /**
   * A single process manager.
   */
  static ProcessMgr manager;
};

#endif
