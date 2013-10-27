// $Id: slog.h,v 1.1 1998/11/17 23:48:53 toddm Exp $
//
// NAME: slog (slog.h)
//
// PURPOSE: Log to a file, stdout, or stderr
//
// USAGE:	(Notes on usage with the System Logger)
//
// #include "../log/slog.h"
// 
//  Your application should define a global pointer to the slog class
//  such as:
//
//  slog *logf; 	// Use of this name is expected in slog.h
//
//  In your initialization logic, you should create the actual logger
//  using one of the constructors outlined below.
//
// 	Logging levels indicate the level of severity of the message.
//	Valid levels are:
//	_EMERGENCY (most severe)
//	_FATAL 
// 	_ERROR 
// 	_ALERT 
// 	_NOTICE 
// 	_INFO 
// 	_DEBUG  (least severe)
//
//	slog(); - Creates a Logger class. Sets level to _DEBUG.
//	slog(int setlevel); - Creates a Logger class. Sets level to setlevel.
//
//	void Openlog(char *ident, int logopt, int facility);
//	    - Calls the system logger's openlog() function. logopt and
//		facility are passed through to openlog(). Ident is the 
//		unique string that will precede all messages.
//		e.g. Openlog("HTTP: ", LOG_PID|LOG_NOWAIT, LOG_LOCAL1);
//
//	void Openlog(char *ident, unsigned short subsys, 
//			int logopt, int facility);
//	    - Calls the system logger's openlog() function. logopt and
//		facility are passed through to openlog(). Ident is the 
//		unique string that will precede all messages.
//		Accepts a subsystem mask to only log certain subsystems.
//		e.g. Openlog("HTTP: ", LOG_PID|LOG_NOWAIT, LOG_LOCAL1);
//
//	void Openlog(char *name, int wh=_to_stdout_);
//	    - Untested call to open a logfile to somewhere other than
//	   	the system logger.  See header below for valid wh values.
//
//	void Openlog(int wh) { where = wh; } - Untested open call.
// 
//	void SetLevel(int setlevel) { level = setlevel; }
//		- Used to set the level of logging. Can be used to change
//		logging levels dynamically.
//
//	void SetSubSysMask(unsigned int ssmask) { subsystem = ssmask; }
//		- Used to set the subsystems to log. Can be used to change
//		logging subsystems dynamically.
//
//	void Log(int lvl, char *message, ...);
//		- Varargs interface to logger.  Level specifies priority
//		of the message. The "message" is a printf-like string
//		which also supports pointers to String class as %S.
//		Also supported - %d, %s, %c and %% (to escape '%').
//
//	void Log(char *s);
//		- Untested - log character string unconditionally.
//
//	void Log(String &s);
//		- Untested - log string unconditionally.
//
//	int Where(void);  - returns logging destination member.
//
//	int SetWhere(int wh);  - sets logging destination member (untested).
//
//	void Closelog(void); - close the logging channel.
//
//
// PROGRAMMING NOTES:
//	Non-System Logger methods remain untested from the original version.
//
// HISTORY: written 9/20 - 9/21 1995 RFH
//	11/09/95 - LJ Shuda - Added "to_syslog" define.
//	11/13/95  LJS - Modified Logging Level definitions to utilize all Caps.
//		- Added SetLevel function.
//	01/11/96 R Holt - added "level" to non-var args Log functions.
//      - made data members protected (for subclass)
//
//  30 Apr 1997 holtrf - transitioning to << and manipulator functions
//
// Copyright 1995 by Destiny Software Corporation.

#include <stdarg.h>
#include <stdio.h>
#include <fstream.h>
#include <stream.h>	// for dec(int, int)

#include "drwcstring.h"
#include "b.h"

#ifndef _LOG_H_
#define _LOG_H_

// log device
#define _to_stdout_ 0
#define _to_stderr_ 1
#define _to_file_ 2
#define _to_stdio_file_ 3
#define _to_syslog_ 4

// log levels
#define _EMERGENCY 0
#define _FATAL 1
#define _ERROR 2
#define _ALERT 3
#define _NOTICE 4
#define _INFO 5
#define _DEBUG 6


// sub-systems
#define ALLSUBSYSTEMS	0xffff

static double *endline;

class slog {
protected:
	ofstream *logfile;
	FILE *fl;
	int where;
	int level;
	int logfacility;
	unsigned short subsystem;
	void Init(int setlevel, int wh)
		{ where = wh; level = setlevel; }

	// current locked state data
	int currentLevel, currentSys;
	DRWCString OutBuf;

	void syslog_write(int how, DRWCString what, DRWCString prefix);
	void do_write(DRWCString out, DRWCString prefix);
	
public:
	int integrity_check;

	slog(void) { Init(_DEBUG, _to_stdout_); }
	slog(int setlevel, int wh = _to_stdout_) { Init(setlevel, wh); }
	void Openlog(const char *ident, int logopt, int facility);
	void Openlog(const char *ident, unsigned short subsys, int logopt, 
		int facility);
	void Openlog(const char *name, int wh=_to_stdout_);
	void Openlog(int wh) { where = wh; }
	void SetLevel(int setlevel) { level = setlevel; }

	const char *Prefix(int lvl);

	void Log(int lvl, unsigned short subsysMask, char *message, ...);

//	void Log(int lvl, char *message, ...);	// obsolete
//	void Log(int lvl, const char *s);	// obsolete
//	void Log(int lvl, String &s);	// obsolete

	int Where(void)  { return where; }
//	int SetWhere(int wh)  { where = wh; }
        void SetWhere(int wh)  { where = wh; }
	void Closelog(void);

	// sub-system stuff

	void SetSubSysMask(unsigned int ssmask) { subsystem = ssmask; }
	void SetSubSysMask(RWCString s);
	enum { subsys1=0x01, subsys2=0x02, subsys3=0x04, subsys4=0x08,
		subsys5=0x10, subsys6=0x20, subsys7=0x40, subsys8=0x80,
		subsys9=0x0100, subsys10=0x0200, subsys11=0x0400, subsys12=0x0800,
		subsys13=0x1000, subsys14=0x2000, subsys15=0x4000, subsys16=0x8000};

	//
	// New as of April 29 1997: synchronous blocking
	// state calls. This will require thread synchronization
	// or one slog object per thread / one logfile per thread.
	// if the logfile is desired to be serial anyway, it
	// could be a real bottleneck.
	//


	friend slog& operator<< (slog&, const DRWCString &s);
	friend slog& operator<< (slog&, const RWCString &s);
	friend slog& operator<< (slog&, const char *sptr);
	friend slog& operator<< (slog&, const char c);
	friend slog& operator<< (slog&, const int i);
	friend slog& operator<< (slog&, event *e);
	friend slog& operator<< (slog&, const double *d);	// special

	inline slog& debug(int subsys)
	{
		SetLog(_DEBUG, subsys);
		return *this;
	}
	inline slog& info(int subsys)
	{
		SetLog(_INFO, subsys);
		return *this;
	}
	inline slog& notice(int subsys)
	{
		SetLog(_NOTICE, subsys);
		return *this;
	}
	inline slog& alert(int subsys)
	{
		SetLog(_ALERT, subsys);
		return *this;
	}
	inline slog& error(int subsys)
	{
		SetLog(_ERROR, subsys);
		return *this;
	}
	inline slog& fatal(int subsys)
	{
		SetLog(_FATAL, subsys);
		return *this;
	}
	inline slog& emergency(int subsys)
	{
		SetLog(_EMERGENCY, subsys);
		return *this;
	}


	void DoStateWrite();

	inline void ClearState()
	{
		OutBuf="";
		currentLevel = 0;
		currentSys = 0;
	}

	inline void SetLog(int currlevel, int currsubsys)
	{
		currentLevel = currlevel;
		currentSys = currsubsys;
	}
};

/////////////////////////
// global manipulators //
/////////////////////////

// log level and subsystem

inline slog& writelog(slog& log)
{
	log.DoStateWrite();
	log.ClearState();
	return log;
}

inline slog& setlog(slog& log, int level, int subsys)
{
	log.SetLog(level, subsys);
	return log;
}

inline slog& operator<< (slog& sl, const DRWCString &s)
{
	if (sl.currentLevel <= sl.level)
		sl.OutBuf += s;
	return sl;
}

inline slog& operator<< (slog& sl, const RWCString &s)
{
	if (sl.currentLevel <= sl.level)
		sl.OutBuf += s;
	return sl;
}

inline slog& operator<< (slog& sl, const char *sptr)
{
	if (sl.currentLevel <= sl.level)
		sl.OutBuf += sptr;
	return sl;
}

inline slog& operator<< (slog& sl, const char c)
{
	if (sl.currentLevel <= sl.level)
		sl.OutBuf += c;
	return sl;
}

inline slog& operator<< (slog& sl, const int i)
{
	if (sl.currentLevel <= sl.level)
		sl.OutBuf += dec(i, 0);
	return sl;
}

inline slog& operator<< (slog& sl, const double *lf)
{
	if (lf == endline)
		return writelog(sl);
	return sl;
}

extern slog& operator<< (slog& sl, event *e);	// in slog.cc

extern slog *logf;		// Globally available Logging class.
extern slog Logf;

#endif

