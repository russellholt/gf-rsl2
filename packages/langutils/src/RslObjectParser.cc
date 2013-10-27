// RslObjectParser.cc
// $Id: RslObjectParser.cc,v 1.3 1999/01/12 15:36:11 toddm Exp $

#include <strstream.h>
#include <stdlib.h>
#include <ctype.h>

#include "RslObjectParser.h"
#include "R_String.h"
#include "R_Integer.h"
#include "R_Boolean.h"
#include "runtime.h"
#include "destiny.h"

#include "killevents.h"

int RslObjectParser::count = 0;


RslObjectParser::RslObjectParser(RWCString& in) : s(in)
{
	count++;
	thisone = count;

#ifdef DEBUG_PARSE
	endLogLine(cout);
	cout << "---- " << count << " ----" << flush;
	endLogLine(cout);
#endif

	intermediate = "";
	slen = s.length();
	complex_object = NULL;
	newObjectName="";
	
	gone = FALSE;
}

RslObjectParser::~RslObjectParser()
{
//  	if (complex_object)        
//  	{                          
//  		delete complex_object; 
//  	}                          
}


Resource *RslObjectParser::charString(int& i)
{
	char c;
	RWCString thestring;
	bool esc=FALSE;

	for (++i; i<slen; i++)
	{
		c = s[i];
		switch (c) {
			case '\"':	// end of string.
				if (!esc)
					return R_String::New(newObjectName, thestring);
				break;

			case 'n':
				if (esc)
				{
					thestring += '\n';
					continue;
				}
				break;
				
			case 'r':
				if (esc)
				{
					thestring += '\r';
					continue;
				}
				break;

			case '\\':
				if (!esc)
				{
					esc = TRUE;
					continue;
				}
				break;
		}

		thestring += c;
		esc = FALSE;
	}

#ifdef DEBUG_PARSE
	cout << "RslObjectParser::charString(): end of string??";
	endLogLine(cout);
#endif

	intermediate = "";

	return R_String::New(newObjectName, thestring);
}


// parse a new object.
event *RslObjectParser::parseFrom(int& i)
{
	char c;
	newObjectName = "";
//	event *anevent = NULL;

#ifdef DEBUG_PARSE
	cout << "parseFrom(" << i << "), length " << slen;
	endLogLine(cout);
#endif
	
	for(; i<slen; i++)
	{
		c = s[i];
		switch(c) {
			case ':':
				newObjectName = intermediate;
				intermediate = "";
#ifdef DEBUG_PARSE
				cout << "New object named `" << newObjectName << "'";
				endLogLine(cout);
#endif
				break;

			case '{':
				complexObject(i);
				break;

			case '}':
#ifdef DEBUG_PARSE
				cout << "closing object."; endLogLine(cout);
#endif
				addCOElement(analyzeIntermediate());

				return complex_object;

			case ',':
				{
#ifdef DEBUG_PARSE
					cout << "more events to come: ";
#endif

					addCOElement(analyzeIntermediate());
				}
				break;
			
			case '\"':	
				addCOElement(newTerminal(charString(i)));
				break;

			// Others: skip blanks, tabs, etc.
			case ' ':
			case '\t':
			case '\r':
				break;
			default:
				intermediate += c;

		}
	}
}

void RslObjectParser::endLogLine(ostream& out)
{

#ifdef DEBUG_PARSE
	out << '\n';	//	"  [" << thisone << "]\n";
	int i;
	for (i=0; i< thisone; i++)
		out << ".  " << flush;
#endif
}

// adds an event to the elist, and creates the elist if it doesn't
// already exist.
void RslObjectParser::addCOElement(event *e)
{
	if (!e)
	{ 
#ifdef DEBUG_PARSE
		cout << "addCOElement(null)"; endLogLine(cout);
#endif
		return;
	}

#ifdef DEBUG_PARSE
	cout << "addCOElement(`"; e->print(cout); cout << "')"; endLogLine(cout);
#endif

	if (!complex_object)
		complex_object = (elistArg *) Remember(new elistArg);
	complex_object->add(e);
	
#ifdef DEBUG_PARSE
	cout << "complex_object is now: `";
	complex_object->print(cout);
	cout << "'"; endLogLine(cout);
#endif
}

// creates a sub parser to parse a complex object,
// that is, the 'i' points to 
void RslObjectParser::complexObject(int& i)
{
#ifdef DEBUG_PARSE
	cout << "parse object of type `" << intermediate << "':";
	endLogLine(cout);
#endif

	RslObjectParser subParser(s);
	event *e = subParser.parseFrom(++i);
	
#ifdef DEBUG_PARSE
	cout << "back in ----- " << thisone << " -----";
	endLogLine(cout);
#endif

	if (!e)
	{
#ifdef DEBUG_PARSE
		cout << "complexObject() for type `" << intermediate
		<< "': no returned event."; endLogLine(cout);
#endif
		return;
	}

//	event *first = ((elist *) e)->evtl.first();
//	if (first && first->isA(event::resArgKind))
//		addCOElement(first);
//	else
//	{
		// e will be an elist (elistArg actually).
		ListArg *la = (ListArg *) Remember(new ListArg(intermediate.data(), (elist *) e));
		nameArg(la);
		addCOElement(la);
//	}

	intermediate = "";
}

// Intermediate will be either an integer or a boolean.
// quoted strings are handled by knowing that they begin
// with the double-quote character, and are dealt with
// in the main switch statement explicitly.
event *RslObjectParser::analyzeIntermediate()
{
	if (intermediate.length() == 0)
		return NULL;
		
	char c = intermediate[0];
	
	Resource *newR = NULL;

	if (isdigit(c))	// Integer
		newR = R_Integer::New(newObjectName, atoi(intermediate.data()));

	// otherwise, assume boolean
	else
	if (isalpha(c))
		newR = R_Boolean::New(newObjectName, (intermediate == "true"));
#ifdef DEBUG_PARSE
	else
	{
		cout << "Parse error at token: `" << intermediate << "'";
		endLogLine(cout);
	}
#endif
		
	intermediate = "";

	if (newR)
		return newTerminal(newR);
		
	return NULL;
}

void RslObjectParser::nameArg(Argument *a)
{
	if (newObjectName == "")
		a->argName = UNNAMED_ARG;	// rslEvents.h, at class Argument.
	else
		a->argName = newObjectName;
//		newObjectName = "";
}

// given a resource, embed it in an event named
// by the object name, and return it.
event *RslObjectParser::newTerminal(Resource *r)
{
	if (!r)
		return NULL;

#ifdef DEBUG_PARSE
	cout << "New terminal: `";
	r->print(cout);
	cout << "', named `" << newObjectName << "'";
	endLogLine(cout);
#endif
	
	ResArg *ra = (ResArg *) Remember(new ResArg(r));
	nameArg(ra);

	return ra;
}

void RslObjectParser::go()
{
	int i=0;

	if (!gone)
		parseFrom(i);
	
	gone = TRUE;
}

// EventResult
// returns the event in the "complex_object" list.
// complex_object will *always* be a list surrounding
// "the" object.
event *RslObjectParser::EventResult()
{
	if (complex_object)
	{
#ifdef DEBUG_PARSE
		cout << "RslObjectParser::EventResult(): complex_object is `" << flush;
		((elistArg *) complex_object)->print(cout);

		cout << "'.";
		endLogLine(cout);
		cout << "First list element is: `" << flush;
		((elistArg *) complex_object)->evtl.first()->print(cout);
		cout << "'";
		endLogLine(cout);
#endif

		return complex_object->evtl.first();
	}

	return NULL;
}

// --------------------------------------------------------
// transforms "complex_object" into a real Resource
// by executing the event in the global context.
// Note that variable references and expressions and
// message passing is illegal in this reduced data format.
ResReference RslObjectParser::ResourceResult()
{
	event *e = EventResult();
	
	event *result = e->execute(runtimeStuff.SysGlobals);
	if (!result)
	{
#ifdef DEBUG_PARSE
		cout << "NULL returned from execute().";
		endLogLine(cout);
#endif
		return ResReference(NULL);
	}
	
	return ((ResArg *) result)->ref;
}

void RslObjectParser::cleanUp()
{
//    killComplexObject(complex_object);
}

// *********************************************************
// * This method is called by the destructor to delete the *
// * events that were created while parsing                *
// *********************************************************
void RslObjectParser::killComplexObject(elist *elEventList)
{
    event *thisEvent;

	if (elEventList)
	{
        // **************************************
        // * While there are events in the list *
        // **************************************
        while(!elEventList->evtl.isEmpty())
        {
            thisEvent = elEventList->evtl.removeLast();

            // ******************************************
            // * if event is a ListArg we must kill     *
            // * the list of events contained in the    *
            // * ListArg event                          *
            // ******************************************
            if (thisEvent->isA(event::argListKind))
            {
                killComplexObject(((ListArg *)thisEvent)->events);
            }

            delete thisEvent;
        }
	}
}
