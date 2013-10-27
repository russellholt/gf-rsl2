#include <rw/collint.h>
#include <rw/cstring.h>
#include <rw/tvhdict.h>
#include <stream.h>
#include <signal.h>
#include <wait.h>
#include <stdlib.h>

#include "Fork.h"
#include "ProcessMgr.h"

/**
 * Implements a general process manager.  Creates child processes
 * via fork-exec.  Keeps track of child process status.  Uses
 * singleton pattern.
 *
 * $Id: ProcessMgr.cc,v 1.1 1998/11/17 23:42:47 toddm Exp $
 */

/**
 * Creates a child process.
 *
 * Params:
 *    cmd - the program to run
 *    args - the command's arguments
 *    mode - the monitoring mode
 */
void ProcessMgr::newChild (const RWCString& cmd, const RWCString& args,
			   int mode)
{
  // Fork
  Fork *fork = new Fork ();
  if (!fork)
  {
    cerr << "ProcessMgr: fork failed, can't exec " << cmd << endl << flush;
    return;
  }
    
  // The child does the real work.
  if (fork->is_child ())
  {
    execlp (cmd.data (), cmd.data (), args.data (), 0);	
    cerr << "ProcessMgr: child failure, can't exec \"" << cmd 
	 << "\"" << endl << flush;
    exit (-1);
  }

  // The parent monitors the child.
  if (mode == KeepAlive)
  {
    (*signal)(SIGCLD, sigcldHandler);
    ExecCommand command;
    command.cmd = cmd;
    command.args = args;
    monitors.insertKeyAndValue (fork->process_id (), command);
  }

  // Delete fork object.
  delete (fork);
}

/**
 * A hash function for RWCollectableInt.
 */
unsigned ProcessMgr::hash (const RWCollectableInt& cint)
{
  return cint.hash ();
}

void ProcessMgr::sigcldHandler (int i)
{
  cout << "received signal!" << endl << flush;
  int status, pid;
  pid = wait (&status);
  cout << pid << endl << flush;
  (*signal)(SIGCLD, sigcldHandler);
  if (manager.monitors.contains (pid))
  {
    ExecCommand& command = manager.monitors [pid];
    manager.createChild (command.cmd, command.args, KeepAlive);
  }
}

/**
 * The single process manager.
 */
ProcessMgr ProcessMgr::manager;

//--------------------------- TESTING ------------------------------
#ifdef _FE_MAIN

/**
 * For unit testing only!
 *
 * Usage:  ProcessManager
 */
main () {
  ProcessMgr::createChild ("man", "grep", ProcessMgr::KeepAlive);
  ProcessMgr::createChild ("man", "ls", ProcessMgr::KeepAlive);
  while (1);
}

#endif
