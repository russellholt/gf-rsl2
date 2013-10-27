// ******************************************************************
// R_templatizer.h
//
// automatically generated from templatizer.rsl and the template:
// $Id: R_templatizer.h,v 1.3 1998/11/24 14:57:11 holtrf Exp $
// ******************************************************************

#ifndef _R_templatizer_H_
#define _R_templatizer_H_

#include <iostream.h>
#include <rw/cstring.h>
#include "res_class.h"
#include "Resource.h"
#include "runtime.h"

#include "rclass_desc.h"

#define R_templatizer_ID 285416498


// ********************************************
// * rc_templatizer -- the templatizer RSL class
// ********************************************
class rc_templatizer : public res_class {
	Resource *spawn(RWCString aname);
public:
	rc_templatizer(RWCString aname) : res_class(aname)
	{	}
};

class template_list {
  public:
	DRWCString main_template;
	DRWCString route_template, route_case_template, route_param_template;
	DRWCString method_decl_template, method_impl_template;
	DRWCString param_template, data_template;
	DRWCString init_template, destroy_template, case_def_template;

	void loadfrom(RWCString directory);
  private:
	void read(RWCString &into, RWCString name);
};


// *************************************************
// * R_templatizer -- the templatizer Resource
// *************************************************
class R_templatizer : public Resource {
	
	// private class data
	RWCString templatePath, templateKind, fn_extension;
	template_list originals, *copies;
	RWCString notes;
/* 	res_class *rc; */
/* 	res_class_decl *rcd; */
	simple_class_description *translated_class;

public:
	static rc_templatizer *rslType;
	static RWCString ThingSeparator;

// Constructors
	
	R_templatizer(RWCString n);
	
// Resource virtuals

	// Info

	unsigned int TypeID() { return R_templatizer_ID; }
	res_class *memberOf(void) { return rslType; }
	
	// Execution
	
	ResStatus execute(int method, ResList& arglist);
	
	void Clear(void);
	void cpp_Init(ResList *constructor_args, ResContext *constructor_context);
	
	// output
	

// R_templatizer specific
	
	static R_templatizer *New(RWCString n);
	void RSL2_to_CPP(RWCString classname);
	int setClassTranslations(RWCString classname);
	RWCString templateSet();
    inline res_class *RC() { return translated_class->rc; }   // yes, these
    inline res_class_decl *RCD() { return translated_class->rcd; } // are "unsafe"
	inline RWCString tName() { return translated_class->name; }
	inline RWCString tParentName() { return translated_class->parent_name; }

	void methods();
	void a_method(method_decl *md);
	RWCString transformMethodName(RWCString mname);
	DRWCString a_param(param *pa, char *num, DRWCString thetemplate);
	void a_datamember(data_decl *dd);
	DRWCString a_description(DRWCString descr);
	void createAndDestroy(RWCString varname);
	void framework();
	void write();

	RWCString OpToString(RWCString op);
	int Cident(const char *s);

	
// R_templatizer RSL methods
	
	ResStatus rsl_convert(const ResList& arglist);
};



#endif

