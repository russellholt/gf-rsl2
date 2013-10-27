// *******************************************************************
// R_templatizer.cc
//
// automatically generated from templatizer.rsl and the template:
// $Id: R_templatizer.cc,v 1.4 1998/12/02 19:18:38 holtrf Exp $
// *******************************************************************


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strstream.h>
#include <strings.h>

#include "destiny.h"
#include "slog.h"
#include "rw_utils.h"
#include "R_templatizer.h"

#define _hCONVERT 102570614	// convert

RWCString R_templatizer::ThingSeparator = ", ";


// R_templatizer static member
rc_templatizer *R_templatizer::rslType = NULL;

extern "C" res_class *Create_templatizer_RC()
{
	return R_templatizer::rslType = new rc_templatizer("templatizer");
}


// Spawn - create a new resource of this type (R_templatizer)
// Called by res_class::New() if there is no object
// to pull off the free list.
Resource* rc_templatizer::spawn(RWCString nm)
{
	return new R_templatizer(nm);	// or other constructor
}

// New
// static convenience function
// See R_String::New(), R_Integer::New(), etc.
// If no functionality beyond res_class::New() is required
// (ie, no special values to be set conveniently), then this
// function simply eliminates the need to cast the result
// of res_class::New().
R_templatizer *R_templatizer::New(RWCString n /*, some value(s) */)
{
	if (!R_templatizer::rslType)
		(void) Create_templatizer_RC();

	if (!R_templatizer::rslType)
		return NULL;

	Resource *r= R_templatizer::rslType->New(n);
//	((R_templatizer *) r)->Set( /* values (add this function if needed) */ );
	return (R_templatizer *) r;
}

// R_templatizer constructor
R_templatizer::R_templatizer(RWCString nm)
	: Resource(nm)
{

}


// *************************************************************
// execute()
// This is the interface between RSL and C++ Resources.
//
// Automatically generated from "templatizer.rsl"
// DO NOT MODIFY !
// *************************************************************

ResStatus R_templatizer::execute(int method, ResList& arglist)
{
	switch(method)
	{
		case _hCONVERT:	// "convert"
			return rsl_convert(arglist);

		default: ;
	}

	return ResStatus(ResStatus::rslFail);
}

// RSL method "convert"
//	/**
//	        templatePath: full path too template kinds
//	        kind: kind of the template, eg "D", "autodoc"
//	        classname: name of rsl class to convert
//	*/
//	convert(String templatePath, String kind, String classname);
ResStatus R_templatizer::rsl_convert(const ResList& arglist)
{
	templatePath = arglist[0].StrValue();
	templateKind = arglist[1].StrValue();

	// There may be another argument which is the
	// file name extension to use on output files.
	if (arglist.entries() > 3)
		fn_extension = arglist[3].StrValue();
	else
		fn_extension = ".h.out";	// the default

	RSL2_to_CPP(arglist[2].StrValue());

	return ResStatus(ResStatus::rslOk, NULL);
}

void R_templatizer::cpp_Init(ResList *constructor_args, ResContext *constructor_context)
{
	notes="";
	templatePath="";
	templateKind="";
	fn_extension="";
}

void R_templatizer::Clear(void)
{
	notes = "";
	templatePath="";
	templateKind="";
	fn_extension="";

	if (copies)
		delete copies;
	copies = NULL;
}

// templateSet()
// a safe convenience function to get the name of the
// template set from the class description.
RWCString R_templatizer::templateSet()
{
	if (translated_class)
		return translated_class->templateSet;
	
	// Hmm... this would be an unusual error.
	logf->info(LOGRSL) << "Templatizer: can't find a template set; defaulting to `D'." << endline;

	return "D";
}

void R_templatizer::RSL2_to_CPP(RWCString classname)
{
	if (setClassTranslations(classname))
		return;

	// start notes over.
	notes = "// Loading templates from directory \"";
	notes += templatePath + templateSet() + "\"\n";

	cerr << "Class `" << classname << "': using `" << templateSet() << "' templates.\n";
	originals.loadfrom(templatePath + templateSet() + "/");
	copies = new template_list;

	methods();
	
	framework();

	write();
	
	Clear();
}

// setClassTranslations()
// Find a res_class and pick which class_description class to use
// for class info translations
// 
// return 1 on failure, 0 on success
int R_templatizer::setClassTranslations(RWCString classname)
{
	res_class *x = runtimeStuff.FindClass(classname);

	if (!x || x->isNull())
	{
		cerr << "Can't find a res_class for `" << classname << "'\n";
		return 1;	// error.
	}

	if (templateKind == "c++")
		translated_class = new cpp_class_description(x);
	else
	{
		translated_class = new simple_class_description(x);

		// simple_class_description does not choose a templateSet
		// so we'll just set it with the value the user asked for.
		// The assumption is that there is a directory of this
		// name with the appropriate template files and stuff,
		// such as, for example, "autodoc".
		//translated_class->templateSet = templateKind;
		if (templateKind != "rsl")
			translated_class->templateSet = templateKind;
	}

	translated_class->set_translations();
	
	if (!translated_class->rcd)
	{
		cerr << "Templatizer: no res_class description for `"
			<< translated_class->name << "'.\n";
		return 1;
	}

	// zero means ok.
	return 0;
}

//
// NOTE! will need class name mapping tables -- from RSL to C++ and back.
// How will this work as a generalization? this could be stored in a text
// file and read by the Table resource - one file per tool; autodoc is
// one to one probably.
// 
//		ds.replace((char *) "RSLNAME", classname.data());
void R_templatizer::methods()
{	
	RWTPtrSlistIterator<method_decl> method_iter(RCD()->methods);
	while(method_iter())
	{
		a_method(method_iter.key());
	}

	RWTPtrSlistIterator<data_decl> data_iter(RCD()->data);
	while(data_iter())
	{
		a_datamember(data_iter.key());
	}
}

RWCString R_templatizer::transformMethodName(RWCString mname)
{
	// This is temporary. It should be replaced by
	// externally stored lookup tables.
	if (!Cident(mname.data()))
	{
		notes += "//   Using hard-coded operator-to-string method.\n";
		return OpToString(mname);
	}

	return mname;
}

void R_templatizer::a_method(method_decl *md)
{
	notes += "// method ";
	char txt[500];
	bzero(txt, 500);
	ostrstream os(txt, 500);
	md->print(os, 0);
	
	notes += txt;

	if (!md->hasFlag(decl::vnative))
	{
		if (md->implementation)
		{
			notes += "//   has RSL implementation (skipping).\n";
			return;
		}
		notes += "//   no RSL implementation, but not declared `native'.";
	}
	notes += "\n//   generating.\n";

	RWCString tName = transformMethodName(md->name);
	RWTPtrSlistIterator<param> iter(md->fparams);

	DRWCString themethod_decl = originals.method_decl_template;
	DRWCString themethod_impl = originals.method_impl_template;
	DRWCString route_case = originals.route_case_template;

// make a string of the parameter list.
	DRWCString paramlist, routeparams;
	int first = 1;
	int n=0;
	char num[32];
	while(iter())
	{
		if (!first) {
			routeparams += ThingSeparator;
			paramlist += ThingSeparator;
		}
		else
			first = 0;

		sprintf(num, "%d\0", n);
	
		paramlist += a_param(iter.key(), num, originals.param_template);
		routeparams += a_param(iter.key(), num, originals.route_param_template);

		n++;
	}

	// insert the parameter list string into the method
	// prototype string
	themethod_decl.replace((char *) "<PARAM_LIST>", paramlist.data());
	themethod_impl.replace((char *) "<PARAM_LIST>", paramlist.data());

	// Method description
	themethod_decl.replace((char *) "<DESCRIPTION>", a_description(md->description.data()) );
	themethod_impl.replace((char *) "<DESCRIPTION>", a_description(md->description.data()) );

	themethod_decl.replace((char *) "<DECL_TEXT>", txt);
	themethod_impl.replace((char *) "<DECL_TEXT>", txt);
	
	route_case.replace((char *) "<ROUTE_PARAM_LIST>", routeparams.data());


	// Case identifier is a hash code on the ACTUAL method name,
	// not the transformed method name!
	DRWCString def = originals.case_def_template;
	sprintf(num, "%u", Resource::theIDHash(md->name.data())); // must use the ACTUAL name!
	def.replace((char *) "<CASE_CODE>", num);

	RWCString mname = tName;
	mname.toUpper();
	mname.prepend("_h");
	route_case.replace((char *) "<ROUTE_CASE>", mname.data());
	def.replace((char *) "<ROUTE_CASE>", mname.data());
	
	
	// Now replace tags at the decl and impl level.
	// this must be done after the param lists are inserted so
	// that they may use these tags as well.
	themethod_decl.replace((char *) "<METHOD_NAME>", md->name.data());
	themethod_decl.replace((char *) "<RETURN_CLASS>", md->returnType.data());

	themethod_impl.replace((char *) "<METHOD_NAME>", md->name.data());
	themethod_impl.replace((char *) "<RETURN_CLASS>", md->returnType.data());
	
	route_case.replace((char *) "<METHOD_NAME>", tName.data());
	route_case.replace((char *) "<DECL_TEXT>", txt);
//	route_case.replace((char *) "<RETURN_CLASS>", md->returnType.data());

	
	// Add to the big collection of method declarations.
	copies->method_decl_template += themethod_decl + "\n";
	copies->method_impl_template += themethod_impl + "\n";
	copies->route_case_template += route_case + "\n";
	copies->case_def_template += def + "\n";
}

DRWCString R_templatizer::a_description(DRWCString descr)
{
	// 1. Break the string into words.
	// 2. If a word is the name of a class, then translate it
	//    using the descr_classname_template
	//
	// There are other possiblities as well..

	return descr;
}

DRWCString R_templatizer::a_param(param *pa, char *num, DRWCString thetemplate)
{
	DRWCString theparam = thetemplate;

	if (pa->type == PARAM_ALL_TYPES)
		theparam.replace((char *) "<PARAM_CLASS>", "Object");
	else
		theparam.replace((char *) "<PARAM_CLASS>", pa->type.data());

	theparam.replace((char *) "<PARAM_NAME>", pa->name.data());

	theparam.replace((char *) "<PARAM_NUMBER>", num);

	return theparam;
}

void R_templatizer::a_datamember(data_decl *dd)
{
	char a[100];
	bzero(a, 100);
	ostrstream os(a, 100);
	dd->print(os, 1); // 2nd arg is 1 for printing semi + newline

	notes += RWCString("// declaration: ") + RWCString(a);
	if (!dd->hasFlag(decl::vnative))
	{
		notes += "//   not declared `native' (skipping).\n";
		return;
	}
	notes += "//   generating.\n";

	// Create a string to hold this data declaration
	// Begin with a copy of the original data decl template.
	DRWCString datadecl = originals.data_template;

	// create a string containing comma separated variable names.
	// what's hard-coded is that variable names are comma separated.
	DRWCString datalist;
	
	
	RWTValSlistIterator<RWCString> iter(dd->varnames);

	int first = 1;
	while(iter())
	{
		if (!first) {
			datalist += ThingSeparator;
		} else
			first = 0;

		datalist += iter.key();
		
		createAndDestroy(iter.key());
	}
	
	// Add the comma separated var name string into the data
	// declaration template copy.
	datadecl.replace((char *) "<DATA_DECL_LIST>", datalist.data());

	// give it a class name.
	datadecl.replace((char *) "<DATA_CLASS>", dd->type.data());
	datadecl.replace((char *) "<DESCRIPTION>", a_description(dd->description.data()) );
	
	// Add to the big string of all data declarations.
	copies->data_template += datadecl + "\n";
}

void R_templatizer::createAndDestroy(RWCString varname)
{
	DRWCString creator = originals.init_template;
	DRWCString destroyer = originals.destroy_template;
	
	creator.replace((char *) "<VAR>", varname.data());
	destroyer.replace((char *) "<VAR>", varname.data());
	
	copies->init_template += creator + "\n";
	copies->destroy_template += destroyer + "\n";
}

void R_templatizer::framework()
{
	copies->main_template = originals.main_template;
	DRWCString& maint = copies->main_template;
	
	maint.replace((char *) "<METHOD_BODY_TEMPLATE>", copies->method_impl_template.data());
	maint.replace((char *) "<METHOD_DECL_TEMPLATE>", copies->method_decl_template.data());
	maint.replace((char *) "<DATA_DECL_TEMPLATE>", copies->data_template.data());
	maint.replace((char *) "<ROUTE_CASE_TEMPLATE>", copies->route_case_template.data());
	maint.replace((char *) "<INIT_TEMPLATE>", copies->init_template.data());
	maint.replace((char *) "<DESTROY_TEMPLATE>", copies->destroy_template.data());
	maint.replace((char *) "<CASE_DEF_TEMPLATE>", copies->case_def_template.data());
	maint.replace((char *) "<NOTES>", notes.data());
	maint.replace((char *) "<CLASS_DESCRIPTION>", a_description(RCD()->description.data()) );
	maint.replace((char *) "<RCS_ID>", "$Id: R_templatizer.cc,v 1.4 1998/12/02 19:18:38 holtrf Exp $");

	// Last!!
	maint.replace((char *) "<SUPERCLASS>", tParentName().data());
	maint.replace((char *) "<CLASSNAME>", tName().data());
//////	maint.replace((char *) "<SUPERCLASS>", RC()->DerivedFrom().data());
//////	maint.replace((char *) "<CLASSNAME>", RC()->Name().data());
	
	char anint[30];
//	sprintf(anint, "%u\0", rc->TypeID());
	sprintf(anint, "%u\0", Resource::theIDHash(RC()->Name()));
	maint.replace((char *) "<CLASS_ID>", anint);
	
}



void R_templatizer::write()
{
	RWCString outfname = RWCString("D") + RC()->Name() + fn_extension; //".h.out";
	ofstream rcout(outfname.data());
	if (!rcout)
	{
		logf->error(LOGRSL)
			<< "Templatizer: unable to open output file `" << outfname << "'" << endline;

		return;
	}
	
	rcout << copies->main_template;
	rcout.close();
}


// Cident
// return true if the given string is a legal C identifier.
int R_templatizer::Cident(const char *s)
{
int len = strlen(s), id=0, i;
char c;

	for(i=0; c=s[i], i<len; i++)
		id += (isalpha(c) || isdigit(c) || c == '_');

	return (id == len && !(isdigit(s[0])));
}


// OpToString
// Convert an operator into a legal C identifier.
// This will be from a mapping table.
RWCString R_templatizer::OpToString(RWCString op)
{
	if (op == ":=")
		return "ASSIGN";
	if (op == "==")
		return "EQ2";
	if (op == "!=")
		return "NE";
	if (op == "+")
		return "Add";
	if (op == "-")
		return "Subt";
	if (op == "*")
		return "Mult";
	if (op == "/")
		return "Div";
	if (op == "<")
		return "LT";
	if (op == ">")
		return "GT";
	if (op == "<=")
		return "LE";
	if (op == ">=")
		return "GE";
	if (op == "&&")
		return "And";
	if (op == "||")
		return "Or";
	if (op == "!")
		return "Not";
	if (op == "++")
		return "PP";
	if (op == "--")
		return "MM";
	if (op == ">=")
		return "GE";
	if (op == "[]")
		return "Brack";
	if (op == "<<")
		return "LS";
	if (op == ">>")
		return "RS";
	if (op == "=")
		return "EQ";
	if (op == "+=")
		return "PE";
	if (op == "-=")
		return "ME";
	if (op == "*=")
		return "TE";
	if (op == "/=")
		return "DE";
	if (op == "%=")
		return "ModE";
	if (op == "/=")
		return "DE";
	if (op == "%")
		return "Mod";

	return "__ERR__";
}


void template_list::loadfrom(RWCString directory)
{

	read(main_template, directory + "main_template");
	read(route_template, directory + "route_template");
	read(route_case_template, directory + "route_case_template");
	read(method_decl_template, directory + "method_decl_template");
	read(method_impl_template, directory + "method_impl_template");
	read(data_template, directory + "data_template");
	read(param_template, directory + "param_template");
	read(route_param_template, directory + "route_param_template");
	read(init_template, directory + "init_template");
	read(destroy_template, directory + "destroy_template");
	read(case_def_template, directory + "case_def_template");
	// in rsl, this could be:
	//	foreach var in (self)
	//		read(var, var_name);
}

void template_list::read(RWCString &into, RWCString name)
{
	ifstream fn(name.data());
	if (fn)
		into.readFile(fn);
	else
		logf->error(LOGRSL) << "Templatizer: unable to open file `"
			<< name << "'" << endline;
}
