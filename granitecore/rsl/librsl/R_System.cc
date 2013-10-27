// R_System.cc
// $Id: R_System.cc,v 1.1 1998/11/17 23:53:43 toddm Exp $

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fstream.h>
#include <unistd.h>

#include "R_System.h"
#include "R_String.h"
#include "R_Integer.h"

extern ofstream rslerr;
//static ofstream rslerr("rslerr");

extern "C" res_class *Create_System_RC()
{
	return &(R_System::rslType);
}

// R_System static member
rc_System R_System::rslType("System");

// R_System method name hash definitions
#define _hDATE 1684108389	// date
#define _hTIME 1953066341	// time
#define _hPROCID 422997859  // ProcID
#define _hPRINT 74606958	// print


// Spawn - create a new resource of this type (R_System)
// Called by res_class::New() if there is no object
// to pull off the free list.
Resource* rc_System::spawn(RWCString nm)
{
	return new R_System(nm);
}

R_System::R_System(void)
{	}

R_System::R_System(RWCString n) : Resource(n)
{	}

R_System *R_System::New(RWCString n)
{
	Resource *r = R_System::rslType.New(n);
	return (R_System *) r;
}

Resource *R_System::clone(void)
{
//	return new R_System();
	return (Resource *) R_System::New("clone");
}

ResStatus R_System::execute(int method, ResList& arglist)
{
	switch(method)
	{
		case _hDATE:	// "date"
			return rsl_date(arglist);

		case _hTIME:	// "time"
			return rsl_time(arglist);

		case _hPROCID:	// "ProcID"
			return ResStatus(ResStatus::rslOk, R_Integer::New("newint", getpid()));

		case _hPRINT:	// "print"
			return rsl_print(arglist);

		default: ;
	}
	
	return ResStatus(ResStatus::rslFail);
}

// R_System::date() RSL method
// Return a String containing today's date in mm/dd/yyyy format
ResStatus R_System::rsl_date(const ResList& arglist)
{
	time_t now = ::time(0);
	struct tm *tmtime = localtime(&now);
	char buf[80];
	int r = strftime(buf, 80, "%m/%d/%Y", tmtime);

	return ResStatus(ResStatus::rslOk, R_String::New("", buf));
}

// R_System::time() RSL method
// Return a String containing the current time
ResStatus R_System::rsl_time(const ResList& arglist)
{
	time_t now = ::time(0);
    struct tm *ltime = localtime(&now);
    return ResStatus(ResStatus::rslOk, R_String::New("", asctime(ltime)));
}

ResStatus R_System::rsl_print(const ResList& arglist)
{
register int i=0,len=arglist.entries();

	// loop through the arglist. ResList::operator[] returns a
	// (ResReference *) and ResReference::print() is a safe passthru
	// to the underlying Resource::print().
	for(; i<len; i++)
		arglist[i].rslprint(cout);

	return ResStatus(ResStatus::rslOk, NULL);
}

