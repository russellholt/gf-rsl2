#include <rw/cstring.h>
#include <stream.h>
#include <fstream.h>
#include <time.h>

#ifndef _Log_H_
#define _Log_H_

/**
 * An error log manager.  Writes messages to the log (stderr by
 * default) if logging is turned on.
 *
 * $Id: Log.h,v 1.1 1998/11/17 23:43:50 toddm Exp $
 */
class Log {

  //--------------------------- PUBLIC -------------------------------
  public:

    /**
     * The debug level.
     */
    enum { DEBUG = 1 };

    /**
     * The info level.
     */
    enum { INFO = 2 };
  
    /**
     * The notice level.
     */
    enum { NOTICE = 3 };
  
    /**
     * The alert level.
     */
    enum { ALERT = 4 };
  
    /**
     * The error level.
     */
    enum { ERROR = 5 };
  
    /**
     * The fatal level.
     */
    enum { FATAL = 6 };
  
    /**
     * The emergency level.
     */
    enum { EMERGENCY = 7 };
  
    /**
     * The level of logging.  Messages of a lower level are not logged.
     */
    static int level;

    /**
     * The file to log to.
     */
    static RWCString fname;
  
    /**
     * The title of the log.
     */
    static RWCString title;
  
    /**
     * The message prefix.
     */
    static RWCString prefix;

    /**
     * Initializes the log.
     */
    static void init () { level = ALERT; fname = "system.log"; 
                         writer = NULL; title = ""; prefix = ""; }

    /**
     * Sets the logging level.
     */
    static void setLevel (int iLevel) { Log::level = iLevel; }
  
    /**
     * Sets the logging level based on a 4-char string.
     */
    static void setLevel (RWCString level);

    /**
     * Sets the log file.
     */
    static void setFile (RWCString strFname, RWCString strTitle)
            { Log::fname = strFname; Log::title = strTitle; }
  
    /**
     * Sets the substring to be added to each log message.
     */
    static void setMessagePrefix (RWCString strPrefix)
            { Log::prefix = strPrefix; }
  
    /**
     * Logs the message if not in silent mode.
     *
     * Params:
     *    level - the reporting level
     *    message - the message
     */
    static void println (int iLevel, const RWCString& message)
       { if (writer == NULL) open ();
         if (iLevel >= Log::level) (*writer) << prefix << levelStr (iLevel)
	   << " [" << timestamp () << "] " << message << endl << flush; }

  //--------------------------- PRIVATE ------------------------------
  private:

    /**
     * Returns the current time in printable format.
     */
    static char* timestamp ();

    /**
     * Opens the log file for writing.
     */
    static void open ();
  
    /**
     * Returns the string level in printable format.
     * 
     * Params:
     *   level - the log level
     */
    static RWCString levelStr (int level);

    /**
     * The print stream.
     */
    static ofstream* writer;
};

#endif



