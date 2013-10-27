// R_Log.cc
// $Id: R_Log.cc,v 1.1 1998/11/17 23:47:03 toddm Exp $

#define _hDEBUG 56975989	// debug
#define _hINFO 1768842863	// info
#define _hNOTICE 218788969	// notice
#define _hALERT 359425394	// alert
#define _hERROR 393376367	// error
#define _hFATAL 174158945	// fatal
#define _hEMERGENCY 2064124689	// emergency

#include "R_Log.h"
#include "slog.h"
#include "destiny.h"

// R_Log static member
rc_Log R_Log::rslType("Log");

extern "C" res_class *Create_Log_RC()
{
	return &(R_Log::rslType);
}

// Spawn - create a new resource of this type (R_Log)
// Called by res_class::New() if there is no object
// to pull off the free list.
Resource* rc_Log::spawn(RWCString nm)
{
	return new R_Log(nm);
}

// New
// static convenience function
// See R_String::New(), R_Integer::New(), etc.
// If no functionality beyond res_class::New() is required
// (ie, no special values to be set conveniently), then this
// function simply eliminates the need to cast the result
// of res_class::New().
R_Log *R_Log::New(RWCString n)
{
	Resource *r= R_Log::rslType.New(n);
	return (R_Log *) r;
}

// R_Log constructor
R_Log::R_Log(RWCString nm)
	: Resource(nm)
{

}

// StrValue()
// Return a String version of this Resource, only if applicable.
RWCString R_Log::StrValue(void)
{
	return "Log";
}

// LogicalValue()
// Evaluate the "trueness" of this Resource (1 for true, 0 for false)
// Used in logical comparisons.
int R_Log::LogicalValue()
{
	return 1;
}

// IsEqual()
// Test for equality with another Resource.
int R_Log::IsEqual(Resource *r)
{
	return (r == this);
}


// Assign
// set this resource equal to r.
// (ResStructure provides a default version)
void R_Log::Assign(Resource *r)
{

}

// Clear()
// Memory management - called to restore an object
// to a "just created" state, for free-list management.
// (ResStructure provides a default version)
void R_Log::Clear()
{
	
}

// print()
// ECI syntax
// (ResStructure provides a default version)
void R_Log::print(ostream &out)
{
	out << "Log { }";
}

// rslprint()
// Printing from within RSL
// (ResStructure provides a default version)
void R_Log::rslprint(ostream &out)
{
	out << "Log\n";
}

// logArgs
// called by all the rsl_ methods in R_Log. The log level
// has already been established in those functions, and here
// we simply send each argument to the internal buffer in logf,
// and end the line.
void R_Log::logArgs(const ResList& arglist)
{
	int i=0, len = arglist.entries();

	ResReference ref;
	for (; i<len; i++)
	{
		ref = arglist[i];
		Logf << (ref.StrValue());
	}
	Logf << endline;
}

ResStatus R_Log::execute(int method, ResList& arglist)
{
	switch(method)
	{
		case _hDEBUG:	// "debug"
			return rsl_debug(arglist);

		case _hINFO:	// "info"
			return rsl_info(arglist);

		case _hNOTICE:	// "notice"
			return rsl_notice(arglist);

		case _hALERT:	// "alert"
			return rsl_alert(arglist);

		case _hERROR:	// "error"
			return rsl_error(arglist);

		case _hFATAL:	// "fatal"
			return rsl_fatal(arglist);

		case _hEMERGENCY:	// "emergency"
			return rsl_emergency(arglist);

		default: ;
	}

	return ResStatus(ResStatus::rslFail);
}

// RSL method "debug"
ResStatus R_Log::rsl_debug(const ResList& arglist)
{
	logf->debug(LOGAPPENV);
	logArgs(arglist);	// calls endline;
	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "info"
ResStatus R_Log::rsl_info(const ResList& arglist)
{
	logf->info(LOGAPPENV);
	logArgs(arglist);	// calls endline;
	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "notice"
ResStatus R_Log::rsl_notice(const ResList& arglist)
{
	logf->notice(LOGAPPENV);
	logArgs(arglist);	// calls endline;
	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "alert"
ResStatus R_Log::rsl_alert(const ResList& arglist)
{
	logf->alert(LOGAPPENV);
	logArgs(arglist);	// calls endline;
	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "error"
ResStatus R_Log::rsl_error(const ResList& arglist)
{
	logf->error(LOGAPPENV);
	logArgs(arglist);	// calls endline;
	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "fatal"
ResStatus R_Log::rsl_fatal(const ResList& arglist)
{
	logf->fatal(LOGAPPENV);
	logArgs(arglist);	// calls endline;
	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "emergency"
ResStatus R_Log::rsl_emergency(const ResList& arglist)
{
	logf->emergency(LOGAPPENV);
	logArgs(arglist);	// calls endline;
	return ResStatus(ResStatus::rslOk, NULL);
}
