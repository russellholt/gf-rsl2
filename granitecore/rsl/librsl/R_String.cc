// R_String.cc
// $Id: R_String.cc,v 1.2 1998/11/24 19:27:52 toddm Exp $
//#include <typeinfo>

#include "R_String.h"
#include "drwcstring.h"
#include "R_Boolean.h"
#include "R_Integer.h"
#include "R_List.h"
#include "rsldefaults.h"
#include <rw/regexp.h>

static char rcsid[] = "$Id: R_String.cc,v 1.2 1998/11/24 19:27:52 toddm Exp $";

int totalR_Strings = 0;

extern "C" res_class *Create_String_RC()
{
	return &(R_String::rslType);
}

// R_String static member
rc_String R_String::rslType("String");

// R_String method name hash definitions
#define _hOpASSIGN 14909    // :=
#define _hOpEQ 61   // =	(OBSOLETE)
#define _hOpAdd 43  // +
#define _hOpPE 11069    // +=
#define _hPREPEND 354156912 // prepend
#define _hAPPEND 252997733  // append
#define _hOpBrack 23389 // []
#define _hOpEQ2 15677   // ==
#define _hOpNE 8509 // !=
#define _hOpGT 62   // >
#define _hOpGE 15933    // >=
#define _hOpLT 60   // <
#define _hOpLE 15421    // <=
#define _hCONTAINS 33947655 // contains
#define _hCONTAINSREGEX 677603170   // containsRegex
#define _hMATCHESREGEX 1617125961   // matchesRegex
#define _hISNUMERIC 1729510428  // isNumeric
#define _hISCASH 437994337  // isCash
#define _hLENGTH 403533415  // length
#define _hSQUEEZE 369823845 // squeeze
#define _hPREPAD 286680400  // prePad
#define _hPOSTPAD 537794420 // postPad
#define _hOpMult 42 // *
#define _hREPEAT 319909989  // repeat
#define _hREPLACE 319165804 // replace
#define _hREPLACEREGEX 1986097222   // replaceRegex
#define _hSPLIT 124808297   // split 
#define _hUPCASE 102064993  // upcase
#define _hDOWNCASE 118359051    // downcase
#define _hBEFORE 268461679  // before
#define _hBEFOREREGEX 2003127306    // beforeRegex
#define _hAFTER 325481573   // after
#define _hAFTERREGEX 1984696578 // afterRegex
#define _hSTRIPCONTROL 2001039979   // stripControl

const RWCRegexp RXcash("\\$?\\([0-9]?[0-9]?[0-9]\\)\\(,?[0-9][0-9][0-9]\\)*\\.[0-9][0-9]");
const RWCRegexp RXwhite(" +");
const RWCRegexp RXnumeric("[0-9]+");

// temporary non-member function
// till I move it into class DRWCString or something.
void split(RWCString val, RWTValSlist<RWCString>& list, RWCString sep);


// Spawn - create a new resource of this type (R_String)
// Called by res_class::New() if there is no object
// to pull off the free list.
Resource* rc_String::spawn(RWCString nm)
{
	totalR_Strings++;
	return new R_String(nm, "");
}

R_String::R_String(RWCString v) : DR_String(v) { }

R_String::R_String(char* v) : DR_String(v) { }

R_String::R_String(RWCString n, RWCString v) : Resource(n), DR_String(v) { }

void R_String::rslprint(ostream &out)
{
	out << val();
}

int R_String::LogicalValue()
{
	return (val().length() > 0);
}

void R_String::Assign(Resource *r)
{
	if (r)
		val() = r->StrValue();
}

int R_String::IsEqual(Resource *r)
{
	if (r)
		return (val() == r->StrValue());
}



// Static convenience New() -- set both a name and a value
// Get a new R_String from the res_class (either by new() or from
// the R_String freelist).
R_String *R_String::New(RWCString nm, RWCString val)
{
	Resource *r= R_String::rslType.New(nm);
	((R_String *) r)->Set(val);

	return (R_String *) r;
}

// Static convenience New() -- set a value (blank name)
R_String *R_String::New(RWCString val)
{
	Resource *r= R_String::rslType.New("");
	((R_String *) r)->Set(val);

	return (R_String *) r;
}

Resource *R_String::clone(void)
{
	return (Resource *) R_String::New(val());
}

void R_String::Set(RWCString v)
{
	this->val() = v;
}

RWCString R_String::StrValue(void)
{
	return val();
}

void R_String::Clear()
{
	dump();
}

void R_String::print(ostream &out)
{
	out << '\"' << OutEscape() << '\"';
}

// ConvertEscapes()
// Translate literal escapes such as "\n" into control characters.
// Right now just replaces char strings, though may evolve into
// the more C-like `/' escape-indicator.
void R_String::ConvertEscapes()
{
	DRWCString s(val());
	s.replace("\\n", "\n");
	s.replace("\\t", "\t");
	s.replace("\\r", "\r");
	//s.replace("\\\"", "\"");
	s.replace("\\\"", "|\"|");

	s = s.strip(RWCString::both, '\"');

	s.replace("|\"|", "\"");

	//(*this) = s;
	this->val() = s;
}

RWCString R_String::OutEscape()
{
	DRWCString s(val());
	s.replace("\n", "\\n");
	s.replace("\t", "\\t");
	s.replace("\r", "\\r");
	s.replace("\"", "\\\"");

	return RWCString(s.data());
}

void R_String::SetFromInline(RWTPtrSlist<Resource>& inliner)
{
	if (inliner.entries() > 0)
		Assign(inliner.first());
}

// matches()
// return 1 if the regex matches this string.
int R_String::matches(const RWCRegexp& reg)
{
	RWCSubString sub = val()(reg);
	return (sub.start() == 0 && sub.length() == val().length());
}

// stripControl()
// remove control chars.
// Sometimes we want to leave the newlines, though,
// so they will be left if stripNewlines == 0.
void R_String::stripControl(int stripNewlines)
{
RWCString& dv = val();
int i, j=0, len = dv.length();
char c, *nw = new char[len];

	for(i=0; i<len; i++)
	{
		c = dv[i];
		if ((c > 0x1f) || (stripNewlines && c == 0x0a))
			nw[j++] = c;
	}
	nw[j] = '\0';

	val() = nw;
	delete[] nw;
}

// non-member function for now.
// Will roll into class DRWCString when I have a chance.
void split(RWCString str, RWTValSlist<RWCString>& list, RWCString sep)
{
	int i=0, val_len = str.length(), index=0, prev_index=0,
		sep_len = sep.length();
	RWCString sub;

#ifdef RSLERR
	cerr << "splitting `" << str << "' with `" << sep << "':\n";
	cerr << "sep_len == " << sep_len << endl;
#endif

	for(; i < val_len; i++)
	{
		index = str.index(sep, sep_len, prev_index, RWCString::exact);

		// seperator not found: we're done.
		if (index == RW_NPOS)
		{

#ifdef RSLERR
			cerr << "split: sep no longer found.\n";
#endif
			break;
		}

		// separator found at the start: ignore
		if (prev_index == 0 && index == 0)
		{

#ifdef RSLERR
			cerr << "split: (prev_index == index == 0) - continue.\n";
#endif

//			prev_index = 1;
			prev_index += sep_len;
			continue;
		}

		sub = str(prev_index, index - prev_index);
		list.append(sub);

#ifdef RSLERR
		cerr << "Substring extracted: `" << sub << "'\n";
#endif

		prev_index = index + sep_len;
	}

	if (prev_index < val_len)
	{
		sub = str(prev_index, val_len - prev_index);
		list.append(sub);
	}
}


ResStatus R_String::execute(int method, ResList& arglist)
{

	switch(method)
	{
		case _hOpASSIGN:	// ":="
		case _hOpEQ:	// "="	(OBSOLETE)
			return OpEQ(arglist);

		case _hOpAdd:	// "+"
			return OpAdd(arglist);

		case _hOpPE:	// "+="
			return OpPE(arglist);

		case _hPREPEND:	// "prepend"
			return rsl_prepend(arglist);

		case _hAPPEND:	// "append"
			return rsl_append(arglist);

		case _hOpBrack:	// "[]"
			return OpBrack(arglist);

		case _hOpEQ2:	// "=="
			return OpEQ2(arglist);

		case _hOpNE:	// "!="
			return OpNE(arglist);

		case _hOpGT:	// ">"
			return OpGT(arglist);

		case _hOpGE:	// ">="
			return OpGE(arglist);

		case _hOpLT:	// "<"
			return OpLT(arglist);

		case _hOpLE:	// "<="
			return OpLE(arglist);

		case _hCONTAINS:	// "contains"
			return rsl_contains(arglist);

		case _hCONTAINSREGEX:	// "containsRegex"
			return rsl_containsRegex(arglist);
 
		case _hMATCHESREGEX:	// "matchesRegex"   
		return rsl_matchesRegex(arglist);
 
		case _hISNUMERIC:	// "matchesRegex"   
		return rsl_isNumeric(arglist);
 
		case _hISCASH:	// "matchesRegex"   
		return rsl_isCash(arglist);

		case _hLENGTH:	// "length"
			return rsl_length(arglist);

		case _hSQUEEZE:	// "squeeze"
			return rsl_squeeze(arglist);

		case _hPREPAD:  // "prePad"
            return rsl_prePad(arglist);
 
        case _hPOSTPAD: // "postPad"
            return rsl_postPad(arglist);
 
        case _hOpMult:  // "*"
            return OpMult(arglist);
 
        case _hREPEAT:  // "repeat"
            return rsl_repeat(arglist);

		case _hREPLACE:	// "replace"
			return rsl_replace(arglist);

		case _hREPLACEREGEX:	// "replaceRegex"
			return rsl_replaceRegex(arglist);

		case _hSTRIPCONTROL:	// "stripControl"
			return rsl_stripControl(arglist);

		case _hSPLIT:	// "split"
			return rsl_split(arglist);

		case _hUPCASE:	// "upcase"
			return rsl_upcase(arglist);

		case _hDOWNCASE:	// "downcase"
			return rsl_downcase(arglist);

		case _hBEFORE:	// "before"
			return rsl_before(arglist);

		case _hBEFOREREGEX:	// "beforeRegex"
			return rsl_beforeRegex(arglist);

		case _hAFTER:	// "after"
			return rsl_after(arglist);

		case _hAFTERREGEX:	// "afterRegex"
			return rsl_afterRegex(arglist);

		default: return Resource::execute(method, arglist);
	}

	return ResStatus(ResStatus::rslFail, NULL);
}

// operator + returns a new string
ResStatus R_String::OpAdd(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_String::New("", val() + arglist[0].StrValue())
	);
}

// operator += is same as append, though with only one argument.
ResStatus R_String::OpPE(const ResList& arglist)
{
	return rsl_append(arglist);
}

// prepend all arguments in order.
ResStatus R_String::rsl_prepend(const ResList& arglist)
{
register int i=0, len=arglist.entries();

	for (; i<len; i++)
	{
		// assignemnt as to be done because RWCString::prepend()
		// wants a const RWCString& argument! ugg!
		RWCString s = arglist[i].StrValue();
		safe_get()->prepend(s);
	}
	
	return ResStatus(ResStatus::rslOk);
}

// append all arguments in order.
ResStatus R_String::rsl_append(const ResList& arglist)
{
register int i=0, len=arglist.entries();

	for (; i<len; i++)
	{
		// assignemnt as to be done because RWCString::prepend()
		// wants a const RWCString& argument! ugg!
		RWCString s = arglist[i].StrValue();
		safe_get()->append(s);
	}
	
	return ResStatus(ResStatus::rslOk);
}

// RSL method "[]"
ResStatus R_String::OpBrack(const ResList& arglist)
{
	int loc = R_Integer::IntFromResource(arglist.get(0));

	if (loc >= 0 && loc < val().length())
	{
		return ResStatus(ResStatus::rslOk,
			R_String::New("", val()[loc]));
	}

	return ResStatus(ResStatus::rslOk);
}

// Assignment
ResStatus R_String::OpEQ(const ResList& arglist)
{
	val() = arglist[0].StrValue();
	return ResStatus(ResStatus::rslOk, this);
}

ResStatus R_String::OpEQ2(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("newbool", arglist[0].StrValue() == val()) );
}

ResStatus R_String::OpNE(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("newbool", arglist[0].StrValue() != val()) );
}

ResStatus R_String::OpGT(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("newbool", arglist[0].StrValue() > val()) );
}

ResStatus R_String::OpGE(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("newbool", arglist[0].StrValue() >= val()) );
}

ResStatus R_String::OpLT(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("newbool", arglist[0].StrValue() < val()) );
}

ResStatus R_String::OpLE(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("newbool", arglist[0].StrValue() <= val()) );
}

ResStatus R_String::rsl_length(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_Integer::New("newint", val().length())
	);
}

// RSL method "squeeze"
// Strips spaces from the ends of the string and
// compresses multiple adjacent spaces into one.
ResStatus R_String::rsl_squeeze(const ResList& arglist)
{
	// 1. strip spaces off ends and create a DRWCstring from it.
	DRWCString dval( val().strip(RWCString::both) );

	// 2. compress spaces
	dval.replace(RXwhite, " ");
	
	val() = dval; //.data();
	
	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "replace"
ResStatus R_String::rsl_replace(const ResList& arglist)
{
	RWCString what(arglist[0].StrValue());
	RWCString with(arglist[1].StrValue());

	if (what != with)
	{
		DRWCString dval(val());
		dval.replace(what.data(), with.data());
		val() = dval; //.data();
	}

	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "replaceRegex": what, with
ResStatus R_String::rsl_replaceRegex(const ResList& arglist)
{
	DRWCString dval(val());
	dval.replace(RWCRegexp(arglist.get(0)->StrValue()),
		arglist[1].StrValue());

	//this->DR_String::operator= (dval);	//.data();
	this->val() = dval;	//.data();

	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "stripControl"
// strips control chars from s.
// stripControl();
ResStatus R_String::rsl_stripControl(const ResList& arglist)
{
	RWCString& str = val();

	if (str.length() > 0)
	{
		int i, j=0, len = str.length();
		char c, *nw = new char[len];
		int nl = 0;	// don't strip newlines by default
	
		// If there's an argument, then it is a boolean indicating
		// whether to strip newlines also.
		if (arglist.entries() > 0)
			nl = R_Boolean::Bool(arglist.get(0));
	
		for(i=0; i<len; i++)
		{
			c = str[i];
			if ((c > 0x1f) || (nl && c == 0x0a))
				nw[j++] = c;
		}
		nw[j] = '\0';
		
		//(*this) = nw;
		this->val() = nw;
		delete[] nw;
	}
	
	return ResStatus(ResStatus::rslOk, NULL);
}

ResStatus R_String::rsl_split(const ResList& arglist)
{
	RWTValSlist<RWCString> list;

	split(val(), list, arglist.get(0)->StrValue() );

	RWTValSlistIterator<RWCString> iter(list);
	RWCString s;

//	R_List *rl = new R_List;
	R_List *rl = R_List::New("splitlist");

	while(iter())
		rl->append(R_String::New(iter.key()) );
	
	return ResStatus(ResStatus::rslOk, rl);
}

ResStatus R_String::rsl_upcase(const ResList& arglist)
{
	val().toUpper();
	return ResStatus(ResStatus::rslOk);
}

ResStatus R_String::rsl_downcase(const ResList& arglist)
{
	val().toLower();
	return ResStatus(ResStatus::rslOk);
}

// before
// arguments are: int or string.
ResStatus R_String::rsl_before(const ResList& arglist)
{
	ResReference ref = arglist[0];
	RWCString s;
	RWCString& str=val();

	if (ref.TypeID() == R_Integer_ID)
		s = str(0, R_Integer::Int(ref()));
	else
	{
		DRWCString what(ref.StrValue());
		s = DRWCString(str).before(what).data();
	}

	return ResStatus(ResStatus::rslOk, R_String::New("before", s.data()));
}

// RSL method "beforeRegex"
ResStatus R_String::rsl_beforeRegex(const ResList& arglist)
{
	RWCRegexp what(arglist[0].StrValue());
	return ResStatus(ResStatus::rslOk,
		R_String::New("beforeRegex", DRWCString(val()).before(what).data())
	);
}

ResStatus R_String::rsl_after(const ResList& arglist)
{
	ResReference ref = arglist[0];
	RWCString s;
	RWCString& str=val();

	if (ref.TypeID() == R_Integer_ID)
	{
		int i = R_Integer::Int(ref()) + 1;
		if (i >= str.length())
			s = "";
		else
			s = str(i, str.length() - i);
	}
	else
	{
		DRWCString what(ref.StrValue());
		s = DRWCString(str).after(what).data();
	}

	return ResStatus(ResStatus::rslOk, R_String::New("after", s.data()) );
}

// RSL method "afterRegex"
ResStatus R_String::rsl_afterRegex(const ResList& arglist)
{
	RWCRegexp what(arglist[0].StrValue());
	return ResStatus(ResStatus::rslOk,
		R_String::New("afterRegex", DRWCString(val()).after(what).data())
	);
}

// RSL method "prePad"
//    prePad(String str, Integer finalLength);
ResStatus R_String::rsl_prePad(const ResList& arglist)
{
	RWCString str = arglist[0].StrValue();
	int final_len = R_Integer::Int(arglist.get(1));
	int len, i;
	
	len = final_len - val().length();
	if (len > 0)
	{
		for (i=val().length(); i < final_len; i++)
			val().prepend(str);
	}

   return ResStatus(ResStatus::rslOk, this);
}
 
// RSL method "postPad"
//    postPad(String str, Integer finalLength);
ResStatus R_String::rsl_postPad(const ResList& arglist)
{
	RWCString str = arglist[0].StrValue();
	RWCString& valref = val();
	int final_len = R_Integer::Int(arglist.get(1));
	int len, i;
	
	len = final_len - valref.length();
	if (len > 0)
	{
		for (i=valref.length(); i < final_len; i++)
			valref += str;
	}
    return ResStatus(ResStatus::rslOk, NULL);
}
 
// RSL method "*"
// short for "repeat". Integer argument, returns
// a new string repeated n times.
//    String * (Integer count);
ResStatus R_String::OpMult(const ResList& arglist)
{
    return rsl_repeat(arglist);
}
 
// RSL method "repeat"
// a new string repeated n times.
//    String repeat(Integer count);
ResStatus R_String::rsl_repeat(const ResList& arglist)
{
	int i = R_Integer::Int(arglist.get(0));
	RWCString s = val();

	for(i--; i > 0; i--)
		s += val();

   return ResStatus(ResStatus::rslOk, R_String::New("repeat", s.data()));
}


// RSL method "contains"
// return true if the given string pattern is contained
// anywhere within this string.
ResStatus R_String::rsl_contains(const ResList& arglist)
{
	RWCString what(arglist[0].StrValue());
	
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("contains", val().contains(what) )
	);
}

// RSL method "contains"
// return true if the given regular expression pattern is contained
// anywhere within this string.
ResStatus R_String::rsl_containsRegex(const ResList& arglist)
{
	RWCRegexp what(arglist[0].StrValue());
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("containsRegex", !(val()(what).isNull())  )
	);
}
 
// RSL method "matchesRegex"
// return true if the entire string matches the regex
ResStatus R_String::rsl_matchesRegex(const ResList& arglist)
{
	RWCRegexp what(arglist[0].StrValue());

	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("", matches(what))
	);
}


ResStatus R_String::rsl_isNumeric(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("", matches(RXnumeric))
	);
}

ResStatus R_String::rsl_isCash(const ResList& arglist)
{
	return ResStatus(ResStatus::rslOk,
		R_Boolean::New("", matches(RXcash))
	);
}
