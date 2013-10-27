// main.cc  the -*- C++ -*- RSL2 test client
// $Id: rslmain.cc,v 1.1 1998/11/17 23:59:18 toddm Exp $

// *******************
// * System Includes *
// *******************
#include <iostream.h>
#include <fstream.h>

#include <FlexLexer.h>	// flex++
#include <sockinet.h>	// socket++
#include <Fork.h>		// socket++
#include <rw/tphset.h>	// rogue wave
#include <stdlib.h>

// ******************
// * Local Includes *
// ******************
#include "runtime.h"
#include "lexer_context.h"
#include "res_class.h"
#include "slog.h"
#include "rsldefaults.h"

static char rcsid[] = "$Id: rslmain.cc,v 1.1 1998/11/17 23:59:18 toddm Exp $";

// global rsl system objects
ofstream rslerr("rslerr");
runtimeRSL runtimeStuff;
RWTPtrHashSet<res_class> ResClasses(res_class::hash);

// global logging objects
slog Logf;
slog *logf = &Logf;

int main(int argc, char **argv)
{

	Logf.Openlog("logf", _to_stdio_file_);
	Logf.SetSubSysMask("abcdefgh");	// everything and then some

	runtimeStuff.rslStartup(argc, argv);

	exit(0);
}

