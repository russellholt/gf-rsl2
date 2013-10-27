// R_Status.cc
// $Id: R_Status.cc,v 1.1 1998/11/17 23:53:37 toddm Exp $

#include <stdlib.h>         // Standard Library

#include "R_Status.h"
#include "R_Integer.h"
#include "R_String.h"
#include "R_Boolean.h"
#include <fstream.h>
#include "slog.h"
#include "rw_utils.h"
#include <rw/tvslist.h>
#include <rw/regexp.h>
#include <fstream.h>

extern ofstream rslerr;

extern "C" res_class *Create_Status_RC()
{
	return &(R_Status::rslType);
}

// R_Status static member
rc_Status R_Status::rslType("Status");

StatusMessageMap R_Status::stat_messages;

// R_Status.cc
// R_Status method name hash definitions
#define _hLOADFILE 168168705    // LoadFile
#define _hOpASSIGN 14909    // :=
#define _hOpEQ 61   // =	(OBSOLETE)
#define _hSETNUMBER 1409816107  // SetNumber
#define _hSEVERITY 554435100    // Severity
#define _hTAG 5529959   // Tag
#define _hMESSAGE 738334323 // Message
#define _hNUMBER 721907042  // Number
#define _hOpEQ2 15677   // ==


// Spawn - create a new resource of this type (R_Status)
// Called by res_class::New() if there is no object
// to pull off the free list.
Resource* rc_Status::spawn(RWCString nm)
{
	return new R_Status(nm);
}

R_Status::R_Status(int v)
{
	number = v;
}

R_Status::R_Status(RWCString n, int v) : Resource(n)
{
	number = v;
}

// Static utility creator
R_Status *R_Status::New(RWCString n, int v)
{
	Resource *r = R_Status::rslType.New(n);
	((R_Status *) r)->Set(v);
	return (R_Status *) r;
}

Resource *R_Status::clone(void)
{
//	return new R_Status(number);
	return (Resource *) R_Status::New("clone", number);
}

// Set 1
// Since severity, message, tag, etc. are a function of number,
// then there are two cases. If number is unset (statusMessage::statusUnset)
// then we directly assign the given severity and tag. Otherwise, ignore
// the arguments s and t (severity and tag) and assign severity, tag from
// the status messages database.
void R_Status::Set(int num, int s, RWCString t)
{
	number = num;
	if (num == statusMessage::statusUnset)
	{
		severity = s;
		tag = t;
	}
	else
		Set();
}

// Set 2
// Set the values of severity and tag from the database
// based on the current value of num. If number
// is unset (statusMessage::statusUnset) then severity and
// tag will be unset as well (STATUS_UNSET_TAG)
void R_Status::Set(void)
{
	statusMessage m = stat_messages.Find(number);
	severity = m.severity;
	tag = m.tag;
}

void R_Status::rslprint(ostream &out)
{
	out << stat_messages[number];
}

void R_Status::print(ostream &out)
{
	int prev=0;

	out << "Status {";
	if (number != statusMessage::statusUnset)
	{
		out << " Number: " << number;
		prev=1;
	}
	if (severity != statusMessage::statusUnset)
	{
		if (prev)
			out << ", ";
		out << " Severity: " << severity;
		prev=1;
	}
	if (tag != STATUS_UNSET_TAG)
	{
		if (prev)
			out << ", ";
		out << " Tag: \"" << tag << '\"';
	}
	out << " }";
}

ResStatus R_Status::execute(int method, ResList& arglist)
{
	switch(method)
	{
		case _hLOADFILE:	// "LoadFile"
			return rsl_LoadFile(arglist);
			break;
		case _hOpASSIGN:	// ":="
		case _hOpEQ:	// "="	(OBSOLETE)
			return OpEQ(arglist);
			break;
		case _hOpEQ2:	// "=="
			return OpEQ2(arglist);
			break;
		case _hSETNUMBER:	// "SetNumber"
			return rsl_SetNumber(arglist);
			break;
		case _hNUMBER:	// "Number"
			return rsl_Number(arglist);
			break;
		case _hSEVERITY:	// "Severity"
			return rsl_Severity(arglist);
			break;
        case _hTAG: // "Tag"
            return rsl_Tag(arglist);
            break;
		case _hMESSAGE:	// "Message"
			return rsl_Message(arglist);
			break;
		default: ;
	}

	return ResStatus(ResStatus::rslFail);
}

ResStatus R_Status::rsl_LoadFile(const ResList& arglist)
{
#ifdef RSLERR
	rslerr << "Status::LoadFile() for " << arglist[0].StrValue() << "\n";
#endif
	RWCString s = arglist[0].StrValue();
	stat_messages.LoadMessages(s.data());
	return ResStatus(ResStatus::rslOk);
}

// Assignment/conversion
ResStatus R_Status::OpEQ(const ResList& arglist)	// "="
{
	Assign(arglist.get(0));
	return ResStatus(ResStatus::rslOk, this);
}

ResStatus R_Status::rsl_SetNumber(const ResList& arglist)
{
	number = ((R_Integer *) arglist[0]())->intval();
	return ResStatus(ResStatus::rslOk);
}

ResStatus R_Status::rsl_Number(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_Integer::New("tempStatNumber", number)
	);
}

ResStatus R_Status::rsl_Severity(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_Integer::New("tempInt", severity)
	);
}

ResStatus R_Status::rsl_Tag(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_String::New("tempStatMessage", tag));
}

ResStatus R_Status::rsl_Message(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_String::New("tempStatMessage", stat_messages[number]));
}

ResStatus R_Status::OpEQ2(const ResList& arglist)	// "=="
{
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("tempStatBool", IsEqual(arglist.get(0)))
	);
}




// SetFromInline
// Accepted inline tags are: Number, Severity, Priority.
void R_Status::SetFromInline(RWTPtrSlist<Resource>& inliner)
{
#ifdef RSLERR
	rslerr << "R_Status::SetFromInline()\n";
#endif

	RWTPtrSlistIterator<Resource> iter(inliner);
	Resource *r =NULL;
	int theval, valWasChanged=0;
	RWCString thename;

	// Set from inline implies setting all values. Those which
	// are not specified inline should be set to nil or "unset".
	
	severity = number = statusMessage::statusUnset;
	tag = STATUS_UNSET_TAG;
	
	while(iter())
	{
		r = iter.key();
		if (!r)
			continue;

#ifdef RSLERR
		rslerr << "SetFromInline: resource name `" << r->Name() << "', == `";
		r->print(rslerr);
		rslerr << "'\n";
#endif
		
		// no name defaults to the status number so that
		// Status{1} is valid without having to do
		// Status{Number:1}. Note that the following are
		// all VALID:
		// Status{Number:1, Tag: ""}
		// Status{Severity: 3}
		// Status{Priority: 7, Number: 3}

		thename = r->Name();
		if (thename== "Number" || thename == "" )
		{
			number = R_Integer::IntFromResource(r);
			valWasChanged = 1;
		}
		else
		if (thename == "Severity")
			severity = R_Integer::IntFromResource(r);
		else
		if (thename == "Tag")
			tag = r->StrValue();
		else
			cerr << "Error: class `Status' has no data member named `"
				<< thename << "'. Valid members are Number, Severity, Tag.\n";
	}

	if (!valWasChanged)
		number = statusMessage::statusUnset;
}

void R_Status::Assign(Resource *r)
{
	if (!r)
		return;
	
	switch(r->TypeID())
	{
		case R_Integer_ID:
//			Set(((R_Integer *)r)->intval());
			number = ((R_Integer *)r)->intval();
			Set();
			break;

		case R_Status_ID:
		{
			R_Status *ps = (R_Status *) r;
//			Set(ps->Number(), ps->Severity(), ps->Tag());
			number = ps->Number();
			Set();
		}
			break;

		default:
			cerr << "Error: Unable to assign to class 'Status' from class `"
				<< r->ClassName() << "'\n";
	}
}

// IsEqual
// Two status objects are "equal" in RSL if all of their fields match.
// Fields match when they are identical, or one of them is "unset"
// (that is, matches anything).
int R_Status::IsEqual(Resource *r)
{
#ifdef RSLERR
	rslerr << "R_Status::IsEqual().";
#endif
	if (!r)
	{
#ifdef RSLERR
		rslerr << "NULL argument. Not equal.\n";
#endif
		return 0;
	}

#ifdef RSLERR
	rslerr << "Comparing `";
	print(rslerr);
	rslerr << "' with `";
	r->print(rslerr);
	rslerr << "'...\n";
#endif


	if (r->TypeID() == R_Status_ID
			&& matchField(number, ((R_Status *) r)->Number())
			&& matchField(severity, ((R_Status *) r)->Severity())
			&& matchField(tag, ((R_Status *) r)->Tag()))
		return 1;
	else
		if (r->TypeID() == R_Integer_ID)
			return (((R_Integer *) r)->intval() == number);
	
	return 0;
}


int R_Status::matchField(int left, int right) const
{
	if (left == statusMessage::statusUnset || right == statusMessage::statusUnset)
		return 1;
	
	return (left == right);
}

int R_Status::matchField(RWCString left, RWCString right) const
{
	if (left == STATUS_UNSET_TAG || right == STATUS_UNSET_TAG)
		return 1;

	return (left == right);
}

RWCString R_Status::StrValue(void)
{
	return stat_messages[number];
}

