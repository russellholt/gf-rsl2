// R_RemoteConnection.cc
// $Id: R_RemoteConnection.cc,v 1.1 1998/11/17 23:10:38 toddm Exp $
#include "R_RemoteConnection.h"
#include "R_Server.h"
#include <stdlib.h>
#include "sockinet.h"

extern ofstream rslerr;

static char rcsid[] = "$Id: R_RemoteConnection.cc,v 1.1 1998/11/17 23:10:38 toddm Exp $";

#define _hCONNECT 101456494	// connect


// R_RemoteConnection static member
rc_RemoteConnection R_RemoteConnection::rslType("RemoteConnection");



// Spawn - create a new resource of this type (R_RemoteConnection)
// Called by res_class::New() if there is no object
// to pull off the free list.
Resource* rc_RemoteConnection::spawn(RWCString nm)
{
	return new R_RemoteConnection(nm);	// or other constructor
}

// New
// static convenience function
// See R_String::New(), R_Integer::New(), etc.
// If no functionality beyond res_class::New() is required
// (ie, no special values to be set conveniently), then this
// function simply eliminates the need to cast the result
// of res_class::New().
R_RemoteConnection *R_RemoteConnection::New(RWCString n /*, some value(s) */)
{
	Resource *r= R_RemoteConnection::rslType.New(n);
	return (R_RemoteConnection *) r;
}

// R_RemoteConnection constructor
R_RemoteConnection::R_RemoteConnection(RWCString nm)
	: Resource(nm)
{

}

ResStatus R_RemoteConnection::execute(int method, ResList& arglist)
{
	switch(method)
	{
		case _hCONNECT:	// "connect"
			return rsl_connect(arglist);

		default: ;
	}

	return ResStatus(ResStatus::rslFail);
}

// RSL method "connect"
ResStatus R_RemoteConnection::rsl_connect (const ResList& arglist)
{
        // Extract host and port.
	const RWCString &host = arglist[0].StrValue ();
	int port = atoi (arglist[1].StrValue().data());

	// Establish connection.
        iosockinet connection (sockbuf::sock_stream);
	if (connection->connect (host, port) != 0)
	{
	        rslerr << "RemoteConnection: Connection failed" 
		       << endl << flush;
	        return ResStatus(ResStatus::rslFail, NULL);
	}
	rslerr << "Connected to " << host << ":" << port 
	       << endl << flush;
	cout << "Connected to " << host << ":" << port 
	       << endl << flush;

	// Hand off to server.
	R_Server server ("RemoteConnection");
	server.NewConnection (connection);

	// Close connection and exit.
	connection->close ();
	return ResStatus(ResStatus::rslOk, NULL);
}
