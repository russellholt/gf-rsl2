// *****************************************************************************
// *
// * Performance Logger
// *
// * History: Created - TGM 5/11/98
// * 
// * $Id: PerfLog.h,v 1.1 1998/11/17 23:48:14 toddm Exp $
// * 
// * $Log: PerfLog.h,v $
// * Revision 1.1  1998/11/17 23:48:14  toddm
// * Initial revision
// *
// * Revision 2.4  1998/11/12 21:29:13  toddm
// * Add new subsystems
// *
// * Revision 2.3  1998/06/11 18:41:46  toddm
// * Move all includes outside exclusion symbol
// *
// * Revision 2.2  1998/06/08 17:46:17  toddm
// * Fix logging
// *
// * Revision 2.1  1998/05/13 23:45:36  toddm
// * Bump version number
// *
// * Revision 1.1  1998/05/13 23:40:38  toddm
// * Initial revision
// *
// * 
// * Copyright 1998 by Destiny Software Corporation.
// * 
// *******************************************************************************

// *******************
// * System Includes *
// *******************
#include <sys/times.h>
#include <strstream.h>
#include <strings.h>
#include <unistd.h>

// ******************
// * Local Includes *
// ******************
#include "slog.h"
#include "destiny.h"

#ifndef _PerfLog_H_
#define _PerfLog_H_

class PerfLogger
{

private:
	// ************************
	// * Private Data Members *
	// ************************
	clock_t ctStartTime;
  	clock_t ctEndTime;
	long Tick;
	struct tms tmsStruct;

public:
    // ***********************
    // * Public Data Members *
    // ***********************

    // ****************
    // * Constructors *
    // ****************
    PerfLogger() {Tick = sysconf(_SC_CLK_TCK);}

	// ********************************
	// * Public Member Functions	 **
	// ********************************
	void StartTime() { ctStartTime = times(&tmsStruct); }
	void EndTime() { ctEndTime = times(&tmsStruct); }
	double ElapsedTime() { return ((times(&tmsStruct) - ctStartTime)/ (double) Tick); }
	void ReportPerf(RWCString strMessage, slog* pLog=logf)
	{
        double TimeDiff;
        char szTimeBuff[20];

		bzero((void *) szTimeBuff, (size_t) 20);
        ostrstream ostr(szTimeBuff, 20);
        TimeDiff = (ctEndTime - ctStartTime) / (double) Tick;
        ostr << TimeDiff;
        
		pLog->notice(LOGPERFORMANCE) << "(**Performance Logging**) " << strMessage << '\t'
								   << szTimeBuff << endline;
	};
};

#endif
