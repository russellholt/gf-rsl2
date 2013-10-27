// res_param.cc
// $Id: res_param.cc,v 1.2 1998/11/24 19:28:13 toddm Exp $

#include <fstream.h>
#include <rw/tpslist.h>

#include "res_param.h"
#include "Resource.h"
#include "rslEvents.h"
#include "rsl_html.h"
#include "rsldefaults.h"

extern ofstream rslerr;

#ifdef RSL_MEMORY_INFO
int param::NParams;
#endif

// ********************************************
// * param
// * A formal parameter, defined by a type
// * and an parameter name.
// ********************************************

param InfinityParam("...");


param::param(RWCString nm)
	: name(nm)
{
	kind = paramKind;

#ifdef RSL_MEMORY_INFO
	NParams++;
#endif
	typeID=0;
}

int param::operator==(const param& that)
{
	return ( name == that.name && that.isType(typeID));
}

int param::equals(param *p)
{
	return (kind == p->kind && p->isType(typeID) && name == p->name);
}

void param::SetType(RWCString t)
{
	type = t;
	typeID = Resource::theIDHash(t);
}

void param::print(ostream& out)
{
	out << type << ' ' << name;
}

void param::html(ostream& out)
{
	linkClassName(type, out);
	out << "  " << name;
}

int param::Match(ResReference& ref)
{
	// *********************************************************
	// Match by type.
	// match if
	//	1. exact type match.
	//	2. formal type is unspecified,
	//	3. the argument type is derived from the parameter type
	// *********************************************************
	
	if (typeID == ref.TypeID()		// most likely to match
		|| type == PARAM_ALL_TYPES	// (defined in res_param.h)
		|| ref.HierarchyContains(typeID))	// if ref is-a-kind-of
//		|| type == "Resource"		// obsolete: least likely
//		|| type == ref.ClassName()	// obsolete
	{
		
#ifdef RSL_DEBUG_EXEDETAIL
		rslerr << "\tmatched by TYPE `" << type << "'..\n";
#endif

		// ********************************************************
		// C. Match by name (Requires match by type)
		// This implies a named argument, as in a:5.
		// Match if argument name is unspecified or if the argument
		// name matches the formal name exactly.
		// ********************************************************
		
		if (ref.Name() == UNNAMED_ARG)
		{
			// Match, but we have to name the argument as
			// indicated by the declaration so that it can be
			// used within the method body!
			// Set the name of the ResReference (not the Resource!)

//#ifdef RSL_DEBUG_EXEDETAIL
//			rslerr << "Setting name to `" << name << "'\n";
//#endif

			ref.SetName(name);
		}
		else if (name != ref.Name())
		{
//#ifdef RSL_DEBUG_EXEDETAIL
//			rslerr << "\t\tname mismatch: rejected.\n";
//#endif
			return rejected;
		}

//#ifdef RSL_DEBUG_EXEDETAIL
//		rslerr << "\t\tmatched by name: accepted.\n";
//#endif

	}	// type match
	else
	{
//#ifdef RSL_DEBUG_EXEDETAIL
//		rslerr << "\ttype mismatch: rejected.\n";
//#endif

		return rejected;
	}
	
	//--- check for c++ subobject substitution case
	// If we have a C++-implemented
	Resource *r = ref();
	if (r && r->memberOf()->hasParentCPPImplementation() && r->isRSLStruct())
	{
		ResReference subref = ((ResStructure *) r)->GetDataMember(CPP_SUBOBJECT_NAME);

		// if there is a sub-object of the type declared by the parameter,
		// then use that object for the parameter instead of the given.
		if (subref.isValid() && typeID == subref.TypeID())
				ref.Set(ref.Name(), subref());
	}

	return notRejected;
}

// ********************************************
// * param_preset
// * A formal parameter with an expected value
// * (an event handler)
// ********************************************

int param_preset::equals(param *p)
{
	if (what)
		return (param::equals(p) && what->IsEqual( ((param_preset *) p)->what));

	return param::equals(p);
}

// param_preset::Match
// Match param as well as the expected resource (param_preset::what)
// with the given argument.
int param_preset::Match(ResReference &ref)
{
#ifdef RSL_DEBUG_EXEDETAIL
	rslerr << "param_preset::Match()\n";
#endif

	if (param::Match(ref) != rejected && what)
	{
		if (what->IsEqual( ref() ))
			return notRejected;
#ifdef RSL_DEBUG_EXEDETAIL
		else
			rslerr << "param_preset::Match(): Resources are not equal.";
#endif

	}

#ifdef RSL_DEBUG_EXEDETAIL
	else
		rslerr << "param_preset::Match(): NULL resource.";

	rslerr << "	Rejected.\n";
#endif

	return rejected;
}

void param_preset::print(ostream& out)
{
	param::print(out);
	if (what)
	{
		out << ": ";
		what->print(out);
	}
#ifdef RSL_DEBUG_EXEDETAIL
	else
		rslerr << "param_preset::print() : NULL resource.\n";
#endif
}

void param_preset::html(ostream& out)
{
	param::html(out);
	if (what)
	{
		out << ": <PRE>";
		what->print(out);	// print the resource
		out << "</PRE>";
	}
#ifdef RSL_DEBUG_EXEDETAIL
	else
		rslerr << "param_preset::print() : NULL Resource.\n";
#endif
}


// ***********************
// * param_presetIR
// ***********************

void param_presetIR::SetType(RWCString t)
{
	param::SetType(t);
	if (toEval && toEval->isA(event::argListKind))
		((ListArg *) toEval)->argType = t;
}

// param_presetIR::print()
// Two cases: before evaluation and after.
// Before: just print the event (actually ListArg).
// After: `toEval' will have been transformed into a resource,
//		so it's identical to param_preset::print().
void param_presetIR::print(ostream& out)
{
#ifdef RSL_DEBUG_EXEDETAIL
	rslerr << "param_presetIR::print()\n";
#endif

	if (toEval)
	{
		param::print(out);	// type name
		out << ": ";
		toEval->print(out);
	}
	else
	if (what)
		param_preset::print(out);
#ifdef RSL_DEBUG_EXEDETAIL
	else
		rslerr << "param_presetIR::print() -- NULL resource.\n";
#endif
}

// param_presetIR::html()
// html version of param_presetIR::print()
void param_presetIR::html(ostream& out)
{
#ifdef RSL_DEBUG_EXEDETAIL
	rslerr << "param_presetIR::html()\n";
#endif

	if (toEval)
	{
		param::html(out);	// type name
		out << ":<PRE> ";
		toEval->print(out);
		out << "</PRE>";
	}
	else
	if (what)
		param_preset::html(out);

#ifdef RSL_DEBUG_EXEDETAIL
	else
		rslerr << "param_presetIR::print() -- NULL resource.\n";
#endif
}


// ***********************
// * decl
// ***********************

decl::decl(void)
{
	flags = undefined;
}

// ***********************
// * data_decl
// ***********************

data_decl::data_decl(RWCString initialVar)
{
	flags = data;
	type = "unset";
	AddVar(initialVar);
}

// Print a class data declaration list, C++ style
// <type> <var1>, ..., <varn>
void data_decl::print(ostream& out, int printScope)
{
RWTValSlistIterator<RWCString> varIter(varnames);
int prevExists=0;

	if (hasFlag(vconst))
		out << "const ";

	out << type << ' ';
	while(varIter())
	{
		if (prevExists)
			out << ", ";
		out << varIter.key();
		prevExists= 1;
	}

	if (printScope)
		out << ";\n";
}

// print data declaration as html
void data_decl::html(ostream& out, int printScope)
{
RWTValSlistIterator<RWCString> varIter(varnames);
int prevExists=0;

	if (hasFlag(vconst))
		out	<< "<font color=0000ff>"
			<< "const"
			<< "</font> ";

	while(varIter())
	{
		out << "<tr><td width=10%>";

		linkClassName(type, out);
		out << "</td><td>";

		out << "<b>" << varIter.key() << "</b>";

		out << "</td></tr>";
	}

	out << "<tr><td width=10%> </td><td>";
	if (description.length())
		//out << "<UL><P>" << description << "</P>\n</UL>\n";
		out << description << "<P>\n";
	out << "</td></tr>\n";

}

// data_decl::install()
// Create instance variables as defined by the declaration
// and add them to the given ResContext.
int data_decl::install(ResContext *context)
{
	if (!context)
	{
#ifdef RSL_DEBUG_CREATE
		rslerr << "data_decl::install(): null context\n";
#endif
		return 1;
	}

#ifdef RSL_DEBUG_CREATE
	rslerr << "data_decl::install() " << varnames.entries() << " items of type `"
		<< type << "'\n";
#endif

	

	// Find class
	res_class findrc(type);
	res_class *rc = ResClasses.find(&findrc);
	if (rc)
	{
		// Create a new resource for name in list varnames
		RWTValSlistIterator<RWCString> viter(varnames);
		RWCString vname;
		while(viter())
		{
			vname = viter.key();

#ifdef RSL_DEBUG_CREATE
			rslerr << "Create `" << vname << "':  ";
#endif

			// Question: should we check to see if the named object
			// already exists? ResContext is a HashSet, so more than
			// one object can't be instantiated anyway.
		
#ifdef RSL_DEBUG_CREATE
			ResStatus stat = context->Find(vname);
			if (stat.status == ResStatus::rslOk)
			{
				cerr << "data_decl::install(): object for `"
					<< vname << "' already exists.\n";
			}
#endif



			// Create the new object and call it's constructor
			// (if there is one)
			Resource *r = rc->New(
					vname,	// new object name
					(ResList *) NULL,		// no args to possible constructor
					context		// context for constructor
				);
			
			if (flags >= vprivate)
			{
				access_t vis = (flags & vprivate) ? vPrivate
					: ( (flags & vprotected) ? vProtected : vPublic);

				cerr << "declaring data member with access control.\n";
					
				context->AddResource(r, vis);
//				context->AddReferenceTo(vname, r, vis);
			}
			else
				context->AddResource(r);
//				context->AddReferenceTo(vname, r);

#ifdef RSL_DEBUG_CREATE
			if (r)
			{
				rslerr << "`" << r->Name() << "'\t";
				r->print(rslerr);
			}
			else
				rslerr << "(NULL ??)";
			rslerr << endl;
#endif

		}
	}
	else
	{
		cerr << "Error: In declaration `";
		print(cerr, 1);
		cerr << "', class `" << type << "' undefined.\n";
		return 1;
	}

	return 0;
}


// ***********************
// * method_decl
// ***********************

method_decl::method_decl(void)
{
	flags = method;
	implementation=NULL;
	infinity=0;
}

// method_decl::operator==()
// "Equality" is everything identical
int method_decl::operator==(const method_decl& obj)
{
#ifdef RSL_DEBUG_LINK
	rslerr << "// method_decl::operator==() : ("
		<< name << ", " << memberOf << ") == ("
		<< obj.name << ", " << obj.memberOf << ") ??\n";
#endif

	return IsIdentical(obj);
}

// "Equality" by matching everything
int method_decl::IsIdentical(const method_decl& obj)
{
	// basic check: name and class
	if (	name != obj.name
		||	memberOf != obj.memberOf
	) return 0;

	// basic check 2: number of parameters
	
	if (fparams.entries() != obj.fparams.entries())
		return 0;

	// Parameter-by-parameter check.

RWTPtrSlistIterator<param> thisParam(fparams);
RWTPtrSlistIterator<param> thatParam((RWTPtrSlist<param>) (obj.fparams));
param *l=NULL, *r=NULL;
int ok=0, total=0;	// assume true

	while ((++thisParam == TRUE) && (++thatParam == TRUE))
	{
		l = thisParam.key();
		r = thatParam.key();
		if (l && r)
			if (l->equals(r))	// virtual function for different param subclasses
				ok++;
			else
				return 0;
		total++;
	}
#ifdef RSL_DEBUG_EXEDETAIL
	rslerr << "\tmatched " << ok << " args of " << total << '\n';
#endif
	return (ok == total);
}


// Sorting by alpha name
int method_decl::operator<(const method_decl& obj)
{
	return (name < obj.name);	//	Name());
}

void method_decl::print(ostream& out, int printScope)
{
RWTPtrSlistIterator<param> paramIter(fparams);
param *pa=NULL;
int prevExists=0;

	if (returnType != "")
		out << returnType << ' ';
	
	if (printScope)
		out << memberOf << "::";
		
	out << name << "( ";

	while(paramIter())
	{
		if (prevExists)
			out << ", ";
		pa = paramIter.key();
		prevExists=1;
		pa->print(out);
	}

	out << ')';

	if (hasFlag(vconst))
		out << " const";
	
	if (!printScope)
		out << ";";

	if (implementation)
		out << "\t// RSL method";
}

// print a method as html, colored keywords and bold return types.
void method_decl::html(ostream& out, int printScope)
{
RWTPtrSlistIterator<param> paramIter(fparams);
param *pa=NULL;
int prevExists=0;


	beginTargetDef(methodTargetName(memberOf, name), out);
	out << endAnchor;

	out << "<tr><td width=10%>";

	if (returnType != "" && returnType != "void")
	{
		linkClassName(returnType, out);
		out << " ";
	}

	out << "</td><td>";
	
	if (printScope)
	{
		linkClassName(memberOf, out);
		out << "::";
	}
		
	out << " <b>" << name << "</b> ( ";

	while(paramIter())
	{
		if (prevExists)
			out << ", ";
		pa = paramIter.key();
		prevExists=1;

		pa->html(out);
	}

	out << " )";	// end method prototype


	// const flag between closing ) and ;
	if (hasFlag(vconst))
		out << " <font color=0000ff>const</font>";
	
	if (!printScope)
		out << ';';


	if (implementation)
		out << "\t<i><font color=ff0000>RSL implementation</font></i>";

	out << "</td></tr><tr><td width=10%> </td><td>";

	// print the method description
	// in the future, the description will be parsed for
	// special hyperlink commands as for javadoc,
	// eg @see <classname>

	if (description.length() > 0)
		//out << "<UL><P>" << description << "</P>\n</UL>\n";
		out << description << "<P>\n";
	//else
	//	out << "<BR>\n";

	out << "</td></tr>";
}


// method_decl::Match()
// Match each formal parameter with each argument.
// Position is important at the moment. Returns 1 if matched, 0 if not.
//
// Future-- default and expected values in formal parameters.
// Default: if a formal is not matched with an actual, but the
// formal is of type param_preset, then this is a formal with
// a default value: create a resource.
//
// Expected: if a formal is a param_preset as an expected value, it
// must be matched by *value* with the argument (event handler).
int method_decl::Match(int iMethod, ResList& arglist)
{
#ifdef RSL_DEBUG_EXEDETAIL
	rslerr << "method_decl::Match() for `" << name << "' against # " << iMethod << '\n';
#endif

	// First rejection: method name
	if (iMethod != Resource::theIDHash(name.data()))
		return 0;
	
	// More arguments than declarations AND the method does
	// not take infinite parameters ("...")
	if (arglist.entries() > fparams.entries() && !infinity)	
	{
#ifdef RSL_DEBUG_EXEDETAIL
		rslerr << "Rejected -- too many arguments for non-infinite parameter list\n";
#endif
		return 0;
	}
	
	// compare parameter lists.
	
	RWTPtrSlistIterator<param> param_iter(fparams);
	param *pa;
	int argiter=0;
	Resource *r=NULL;
	
	// ********************************************
	// * Walk through the list of formal parameters
	// ********************************************
	while(param_iter())
	{
		pa = param_iter.key();
		
#ifdef RSL_DEBUG_EXEDETAIL
		rslerr << "Formal: ";
		pa->print(rslerr);
#endif

		if (arglist.isNull(argiter))
		{
#ifdef RSL_DEBUG_EXEDETAIL
			rslerr << "Actual: (NULL)\n";
#endif
			return 0;
		}

#ifdef RSL_DEBUG_EXEDETAIL
		rslerr << "Actual: " << (arglist[argiter].ClassName()) << "\tname=`"
			<< (arglist[argiter].Name()) << "'\n";
#endif

		// *******************
		// * Match arguments *
		// *******************
			
		// *************************************************
		// A. Elipses "..." means match anything as a group.
		// *************************************************
		if (pa == &InfinityParam)	// Yes, by pointer address
		{
#ifdef RSL_DEBUG_EXEDETAIL
			rslerr << "Matched infinity parameter `...'\n";
#endif
			return 1;
		}

		
#ifdef RSL_DEBUG_EXEDETAIL
		rslerr << "Comparing formal parameter `" << pa->name
			<< "' with argument `";
		arglist[argiter].print(rslerr);
		rslerr << "':\n";
#endif


		if (param::rejected == pa->Match(
				*( (ResReference *) (arglist.getref(argiter))) ) )
			return 0;

		argiter++;
	}
	
	return 1;
}
