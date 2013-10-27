#include <rw/cstring.h>

#include "Log.h"

/**
 * Implementation of the error log manager.  Writes messages to 
 * the log (stderr by default) if logging is turned on.
 *
 * $Id: Log.cc,v 1.1 1998/11/17 23:42:41 toddm Exp $
 */

/**
 * The level of logging.  Messages of a lower level are not logged.
 */
int Log::level;

/**
 * The file to log to.
 */
RWCString Log::fname;
  
/**
 * The title of the log.
 */
RWCString Log::title;
  
/**
 * The message prefix.
 */
RWCString Log::prefix;

/**
 * The print stream.
 */
ofstream* Log::writer;

/**
 * Sets the logging level based on a 4-char string.
 */
void Log::setLevel (RWCString strLevel)
{
  if (strLevel.compareTo ("DBUG") == 0)
    Log::level = DEBUG;
  else if (strLevel.compareTo ("INFO") == 0)
    Log::level = INFO;
  else if (strLevel.compareTo ("NOTE") == 0)
    Log::level = NOTICE;
  else if (strLevel.compareTo ("ALRT") == 0)
    Log::level = ALERT;
  else if (strLevel.compareTo ("ERRR") == 0)
    Log::level = ERROR;
  else if (strLevel.compareTo ("FATL") == 0)
    Log::level = FATAL;
  else if (strLevel.compareTo ("EMRG") == 0)
    Log::level = EMERGENCY;
}

/**
 * Opens the log file for writing.
 */
void Log::open ()
{
  if (writer != NULL)
  {
    writer->close ();
    delete writer;
    writer = NULL;
  }
  writer = new ofstream (fname);
  (*writer) << title << " -- " << timestamp ();
  (*writer) << endl << endl << flush;
}
  
RWCString Log::levelStr (int iLevel)
{
  switch (iLevel) {
  case DEBUG:
    return "[DBUG]";
  case INFO:
    return "[INFO]";
  case NOTICE:
    return "[NOTE]";
  case ALERT:
    return "[ALRT]";
  case ERROR:
    return "[ERRO]";
  case FATAL:
    return "[FATL]";
  case EMERGENCY:
    return "[EMRG]";
  default:
    return "[UNKN]";
  }
}

/**
 * Returns the current time in printable format.
 */
char* Log::timestamp () {
  static char stamp [80];

  time_t t = time ((long *) 0); 
  strcpy (stamp, ctime (&t));
  stamp [strlen (stamp) - 1] = '\0';
  return stamp;
}

#ifdef _LOG_MAIN

/**
 * For unit testing only!
 */
int main (int argc, char* argv[])
{
  Log::init ();
  Log::setLevel (Log::INFO);
  Log::setFile ("testfile.log", "Tester");
  Log::setMessagePrefix ("System: ");
  Log::println (Log::DEBUG, "DEBUG");
  Log::println (Log::INFO, "INFO");
  Log::println (Log::NOTICE, "NOTICE");
  Log::println (Log::ALERT,"ALERT");
  Log::println (Log::ERROR,"ERROR");
  Log::println (Log::FATAL,"FATAL");
  Log::println (Log::EMERGENCY,"EMERGENCY");
}

#endif
