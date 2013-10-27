#include <rw/cstring.h>
#include <stream.h>
#include <rw/tvslist.h>
#include "StatMessages.h"
#include "rw_utils.h"
#include "drwcstring.h"
#include <fstream.h>
#include <stdlib.h>

#ifdef RSLERR
extern ofstream rslerr;
#endif

StatusMessageMap::StatusMessageMap(int thesize) : dict(hash, thesize)
{

}

StatusMessageMap::~StatusMessageMap()
{
	
}

void StatusMessageMap::AddMessage(int key, int severity,
	RWCString tag, RWCString msg)
{
	statusMessage m;
	m.message = msg;
	m.severity = severity;
	m.tag = tag;
	
	dict.insertKeyAndValue(key, m);
	
}

// [] operator
// Return status message for a given message number
RWCString StatusMessageMap::operator[](int num)
{
#ifdef RSLERR
	rslerr << "StatusMessageMap: find " << num << ":\n";
#endif
	statusMessage m = Find(num);
#ifdef RSLERR
	rslerr << "\tSeverity: " << m.severity << "\n";
	rslerr << "\tMessage: " << m.message << "\n";
	rslerr << "\tTag: " << m.tag << "\n";
#endif
	return m.message;
}

unsigned StatusMessageMap::hash(const int& v)
{
	return v;
}

statusMessage StatusMessageMap::Find(int what)
{
	statusMessage m;
	dict.findValue(what, m);
	return m;
}

// LoadMessages
// Reads status messages from named file and stores them
// in the StatMessages object.
// File consists of lines in the format:
//		<status number> <text>
// status numbers are expected to be sorted from least to greatest.
void StatusMessageMap::LoadMessages(const char *filename)
{
	if (!filename)
		return;

	ifstream f(filename);
	if (!f)
	{
		cerr << "Status: Unable to read messages from file `"
			<< filename << "'\n";
		return;
	}
	
	// ********************************
	// * Read the status messages file
	// ********************************

	RWTValSlist<RWCString> lines;

	f >> lines;
	
	
	
	if (lines.entries() > 0)
	{
		if (dict.isEmpty())
			dict.resize(lines.entries());
	}
	else	// no lines
	{
		cerr << "Status: No messages found in message file `" << filename << "'\n";
		return;
	}

	// *****************************************
	// * Parse each line read & insert a record
	// *****************************************

	RWTValSlistIterator<RWCString> iter(lines);
	DRWCString sline, elements[5];

	while(iter())
	{
		sline = iter.key();
		sline.split(elements, 5, "\t");
		
#ifdef RSLERR
		rslerr << "Status: " << elements[0]
			<< " Severity: " << elements[1]
			<< " Logical tag: `" << elements[2]
			<< "' Message: `" << elements[3]
			<< "'\n";
#endif

		AddMessage(atoi(elements[0].data()), atoi(elements[1].data()),
			elements[2].data(), elements[3].data());
	}
}
