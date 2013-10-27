// templatization class descriptions
// Russell, Oct 14 1998
//
// $Id: rclass_desc.h,v 1.1 1998/11/17 23:25:53 toddm Exp $

#include "res_class.h"

#ifndef class_and_template_junk
#define class_and_template_junk


class simple_class_description {
  public:
	simple_class_description(res_class *c);

	res_class *rc;
	res_class_decl *rcd;
	RWCString name, parent_name;
	RWCString templateSet;

	virtual void set_translations();
};

class cpp_class_description : public simple_class_description {
public:
	enum cpp_subtype_t { regular, composite, collection };
	cpp_subtype_t cpp_subtype;

	cpp_class_description(res_class *c);
	void set_translations();
};


#endif


