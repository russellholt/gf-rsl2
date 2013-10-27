/*
 $Header: /dest/razor/RAZOR_UNIVERSE/DOMAIN_01/foundation25/Archive/RZ_VCS/granite/granitecore/dutil/src/slog.cc,v 1.1 1998/11/17 23:47:28 toddm Exp $
*/

static char rcsid[] = "$Id: slog.cc,v 1.1 1998/11/17 23:47:28 toddm Exp $";

//----------------------------------------------------------------------------
// NAME:	slog.cc
// 
// BRIEF DESCRIPTION	
//
// PURPOSE: System Logger class interface.
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
//	Valid levels are:		Output in the log as:
//	_EMERGENCY (most severe)	EMG
//	_FATAL 				FTL
// 	_ERROR 				ERR
// 	_ALERT 				ALR
// 	_NOTICE 			NOT
// 	_INFO 				INF
// 	_DEBUG  (least severe)		DBG
//
//----------------------------------------------------------------------------
//		  	   PRIVATE MEMBER FUNCTIONS
//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------
//		  	   PROTECTED MEMBER FUNCTIONS
//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

// SYSTEM INCLUDES

#include <stdio.h>			// Standard I/O
#include <stddef.h>			// Standard Definitions
#include <stdlib.h>			// Standard Library
#include <stream.h>
#include <strstream.h>

extern "C" {
#include <syslog.h>
#include <stdio.h>
#include <sys/time.h>
}

// LOCAL INCLUDES

#include "slog.h"


// CONSTANTS


//----------------------------------------------------------------------------
//		  	   CONSTRUCTORS AND DESTRUCTORS
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//		  	   	PUBLIC OPERATORS
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//		  	   PUBLIC MEMBER FUNCTIONS
//----------------------------------------------------------------------------


void slog::Openlog(const char *ident, int logopt, int facility)
{
	where = _to_syslog_;		// To system Logger.
	logfacility = facility;		// e.g. LOG_LOCAL1
	subsystem = ALLSUBSYSTEMS;	// Accept logging from everywhere.
	openlog(ident, logopt, logfacility);
}

void slog::Openlog(const char *ident, unsigned short subsys, 
		   int logopt, int facility)
{
	where = _to_syslog_;		// To system Logger.
	logfacility = facility;		// e.g. LOG_LOCAL1
	subsystem = subsys;		// Only log from specified subsystems.
	openlog(ident, logopt, logfacility);
}

void slog::Openlog(const char *name, int wh)
{
	where = wh;
	switch(where)
	{
		case _to_file_:
			logfile = new ofstream;
			logfile->open(name, ios::app);
			break;
		case _to_stdio_file_:
			fl = fopen(name, "a");
			break;
		default: ;
	}
}


// Log with subSysMask
void slog::Log(int lvl, unsigned short subsysMask, char *message, ...)
{
	if (lvl > level)       // Only log messages of high enough priority.
		return;

	//	if (!(subsysMask & subsystem))	// Only log authorized subsystems.

	if (!(subsysMask & subsystem)	// Only log authorized subsystems
		&& lvl > _FATAL)	// except EMERGENCY and FATAL log levels.
		return;

	va_list ap;
	DRWCString prefix = Prefix(lvl);

//	String Sfor, Rwith;


	DRWCString out;
	auto char typec, *p = message;

	va_start(ap, message);

	while(*p)
	{
		if (*p != '%')	// regular chars
			out += *p;
		else
		{
			p++;
			if (!(*p)) break;
			typec = *p;

			out += "<data>";

			switch(typec)
			{
				case 'B':
				{
					DRWCString *str = va_arg(ap, DRWCString *);
					// if (str) out += str->data();
				}
					break;

				case 'S':
					{
//						String * str = va_arg(ap, String *);
						// out += (
						cerr << "slog: (%S) GNU STRING NO LONGER SUPPORTED\n";
					}
					break;

				case 's': 
					va_arg(ap, char *);
					// out +=
					break;

				case 'c':
					out += (char) va_arg(ap, int);
					break;

				case 'd':
					out += dec(va_arg(ap, int));
					break;

				case '%':
					out += '%';
					break;

				case 'W':	// search for
//					Sfor = *va_arg(ap, String *); break;
//					cout << "Search for \"" << Sfor << "\"\n";
					cerr << "slog: (%W) GNU STRING NO LONGER SUPPORTED\n";
					break;

				case 'X':	// replace with
//					Rwith= *va_arg(ap, String *); break;
//					cout << "Replace with \"" << Rwith << "\"\n";
					cerr << "slog: (%X) GNU STRING NO LONGER SUPPORTED\n";
					break;

// POSSIBLE FUTURE DIRECTION:
//
//				case 'R':
//					{
//						resource *r = va_arg(ap, resource *);
//						if (r)	// necessary ???
//							out += r->Value();
//					}
//					break;
//--------------------------------------------------

				default: ;	// ignore
			}
		}
		p++;
	}
	
	va_end(ap);
	out += '\n';

	
//	if (Sfor.length() > 1 && Rwith.length() > 1)
//		out.gsub(Regex(Sfor), BigString(Rwith.chars()));

	/*
	 * final conversion -- replace each "%" with "%%"
	 * so that syslog doesn't try to substitute
	 * parameters!
	 */

//	out.gsub("%", "%%");
	out.replace("%", "%%");

	do_write(out, prefix);
}


void slog::DoStateWrite(void)
{
	if (!(currentSys & subsystem)	// Only log authorized subsystems
		&& currentLevel > _FATAL)	// except EMERGENCY and FATAL log levels.
		return;

	if (currentLevel > level) // Only log messages of high enough priority.
		return;

	DRWCString prefix = Prefix(currentLevel);
	OutBuf.replace("%", "%%");
	do_write(OutBuf, prefix);
}

// do_write
// Write to the appropriate place.
// But not if it is zero length, y'know.
void slog::do_write(DRWCString out, DRWCString prefix)
{
static char datebuf[80];

	if (out.length() <= 0)
		return;


	// Switch based on logging output in effect.
	switch(where)  {
		case _to_syslog_:
			syslog_write(LOG_DEBUG|logfacility, out, prefix);
			break;
		case _to_stdout_:
			cout << prefix << out;
			break;
		case _to_stderr_:
			cerr << prefix << out;
			break;
		case _to_file_:
			if (logfile)
				logfile->write((prefix+=out,prefix.data()), out.length());
			break;
		case _to_stdio_file_:
		{
			if (fl)
			{
				/* get the time & date */
				datebuf[0] = '\0';	// clean it out
				time_t t = time(0);
				struct tm *tmtime = localtime(&t);    // convert it to a time struct
			    int r = strftime(datebuf, 80, "%b %d %H:%M:%S", tmtime);

			    /* subsitute carriage return with newline */
			    out.replace("\r", "\n");
			    /* may want to strip all control chars out here, or replace
					them with hex values -- add an option for this? */
			
				if (integrity_check)
				{
					fprintf(fl, "<%04d> %s %s %s\n", integrity_check,
						datebuf, prefix.data(), out.data());
					integrity_check++;
					if (integrity_check > 9999)
						integrity_check = 1;
				}
				else
					fprintf(fl, "%s %s %s\n", datebuf,
						prefix.data(), out.data() );
				fflush(fl);	// will slow us down but ensure data output
			}
			else
				cerr << "NO LOG FILE for " << prefix << '\n';
			break;
		}
		default: ;
	}
//	cout << "Done with do_write()..\n" << flush;
}

// syslog_write
// write the string `what' to the syslog, breaking on cr's & lf's,
// as separate writes, each prefixed with `prefix'
void slog::syslog_write(int how, DRWCString what, DRWCString prefix)
{
static RWCRegexp srx("[\n\r]");
int size = what.freq("\n") + what.freq("\r");

	if (size > 0)
	{
//		cerr << "Attempting to split into " << size << " pieces....\n";
//		cerr << what << '\n';
//		cerr.flush();

		DRWCString *sl = new DRWCString[size+1];
		int newsize = what.split(sl, size+1, srx);
		if (newsize > 0)
		{
			int i;
			for (i=0; i<newsize; i++)
			{
				if (sl[i].length() > 0)
				{
					sl[i].prepend(prefix);
					if (integrity_check)
					{
						sl[i].prepend("<%04d> ");
						syslog(how, sl[i].data(), integrity_check);
						integrity_check++;
						if (integrity_check > 9999)
							integrity_check = 1;
					}
					else
						syslog(how, sl[i].data()); 
				}
			}
			delete [] sl;
			return;
		}
		delete [] sl;
	}

	// no cr's or lf's

//	cout << "Prepend the prefix code \"" << prefix << "\"..\n" << flush;
	what.prepend(prefix);
//	cout << "call syslog()..\n" << flush;
	syslog(how, what.data()); 
//	cout << "Done: returning...\n" << flush;
}


//// the code is duplicated so that the functions are faster
//// OBSOLETE OBSOLETE OBSOLETE OBSOLETE
//void slog::Log(int lvl, const char *msg)
//{
//	if (lvl > level)       // Only log messages of high enough priority.
//		return;
//
//	RWCString out	=  Prefix(lvl);
//	
//	out += msg;
//
//
//	switch(where)  {
//		case _to_syslog_: syslog(LOG_DEBUG|logfacility, out.data()); break;
//		case _to_stdout_: cout << out << '\n'; break;
//		case _to_stderr_: cerr << out << '\n'; break;
//		case _to_file_:	if (logfile) *logfile << msg; break;
//		case _to_stdio_file_: if (fl) fprintf(fl, "%s", out.data()); 
//			break;
//		default: ;
//	}
//}

const char *slog::Prefix(int lvl)
{
	RWCString out;
	switch( lvl )
	{
		case _DEBUG:		return "DBG ";
		case _INFO:			return "INF ";
		case _NOTICE:		return "NOT ";
		case _ALERT:		return "ALR ";
		case _ERROR:		return "ERR ";
		case _FATAL:		return "FTL ";
		case _EMERGENCY:	return "EMG ";
	}

	return "UKN ";
}

//void slog::Log(int lvl, String &msg)
//{
//	if (lvl > level)       // Only log messages of high enough priority.
//		return;
//	
//	String out	= Prefix(lvl);
//	out += msg;
//
//	switch(where)  {
//		case _to_syslog_: syslog(LOG_DEBUG|logfacility, out.chars()); 
//				  break;
//		case _to_stdout_: cout << out << '\n'; break;
//		case _to_stderr_: cerr << out << '\n'; break;
//		case _to_file_:	if (logfile) *logfile << out; break;
//		case _to_stdio_file_: if (fl) fprintf(fl, "%s", out.chars()); 
//			cerr << "write " << out << "to logfile.\n"; fflush(fl); break;
//		default: ;
//	}
//}


void slog::Closelog(void)
{
	switch(where)
	{
		case _to_syslog_: closelog(); break;
		case _to_file_: if (logfile) logfile->close(); break;
		case _to_stdio_file_: if (fl) fclose(fl); break;
		default: ;
	}
}



// SetSubSysMask (String s)
// take a string like "abe" to determine which subsystems
// to log. Each letter a-f corresponds to a subsystem.
// Duplicates are ignored. Letter<->system correspondence
// is as follows:
//	a: sub system 1
//	b: sub system 2
//	c: sub system 3
// ...
//  h: sub system 8
//
// this may generalize in the future...
void slog::SetSubSysMask(RWCString s)
{
	if (!s)
		return;

	subsystem = 0;
	RWCString which = toLower(s);
	int i, l = which.length();

	for(i=0; i<l; i++)
	{
		switch(which[i])	// much cooler: // tolog |= pow(2, s[i]-'a');
		{
			case 'a':	subsystem |= subsys1; break;
			case 'b':	subsystem |= subsys2; break;
			case 'c':	subsystem |= subsys3; break;
			case 'd':	subsystem |= subsys4; break;
			case 'e':	subsystem |= subsys5; break;
			case 'f':	subsystem |= subsys6; break;
			case 'g':	subsystem |= subsys7; break;
			case 'h':	subsystem |= subsys8; break;
			case 'i':	subsystem |= subsys9; break;
			case 'j':	subsystem |= subsys10; break;
			case 'k':	subsystem |= subsys11; break;
			case 'l':	subsystem |= subsys12; break;
			case 'm':	subsystem |= subsys13; break;
			case 'n':	subsystem |= subsys14; break;
			case 'o':	subsystem |= subsys15; break;
			case 'p':	subsystem |= subsys16; break;
			default: ;
		}
	}

}

// slog &operator<< (slog& sl, const event *e)
// This is the general << logging for events, which could be huge,
// so a large static buffer is allocated.
// This is a friend of class slog.
slog &operator<< (slog& sl, event *e)
{
static char request_buf[65536];	// 64k ought to be enough (uh, huh huh)

	if (sl.currentLevel <= sl.level)
	{
//		bzero(request_buf, 65536);	// clear the buffer
		ostrstream ostr(request_buf, 16384);
		e->print(ostr);
		ostr << '\0';	// will this work?
		sl.OutBuf += request_buf;
	}
	return sl;
}

