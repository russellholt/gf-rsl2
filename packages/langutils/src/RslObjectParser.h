// RslObjectParser
// $Id: RslObjectParser.h,v 1.2 1998/12/15 16:24:20 toddm Exp $
// *******************
// * System Includes *
// *******************
#include <rw/cstring.h>

// ******************
// * Local Includes *
// ******************
#include "Resource.h"
#include "b.h"
#include "rslEvents.h"

class RslObjectParser {
	RWCString& s;
	RWCString intermediate;
	int /*index,*/ slen;
	elistArg *complex_object;
	RWCString newObjectName;

	static int count;
	int thisone;
	
	bool gone;	// can only "go" once.

protected:
//	void newObject();
	Resource *charString(int& i);

	event *parseFrom(int& i);
	void complexObject(int& i);
	void addCOElement(event *e);
	event *analyzeIntermediate();
	event *newTerminal(Resource *r);
	void nameArg(Argument *a);
	void endLogLine(ostream& out);
    void killComplexObject(elist *elEventList);
	
public:

	RslObjectParser(RWCString& in);
	~RslObjectParser();

	void go();
	event *EventResult();
	ResReference ResourceResult();
    void cleanUp();
};



