#include <iostream.h>

#include "rclass_desc.h"


// ********************************************************************
// * Class descriptions
// ********************************************************************

simple_class_description::simple_class_description(res_class *c)
{
	rc = c;

	if (rc)
		rcd = rc->getImplementation();
	else
		rcd = (res_class_decl *) 0;
}

void simple_class_description::set_translations()
{
	if (rc)
	{
		name = rc->Name();
		parent_name = rc->DerivedFrom();
	}
}

cpp_class_description::cpp_class_description(res_class *c)
	: simple_class_description(c)
{
	cpp_subtype = regular;
}

void cpp_class_description::set_translations()
{
	if (!rc)
		return;
	
	simple_class_description::set_translations();
	
	if (!rcd)
		return;

	RWCString colname="Collection", compname = "Composite";
	
	// If we have data declarations, the we are a composite object
	if (rcd->data.entries() > 0)
	{
		cpp_subtype = composite;
		
		// RSL2 doesn't have an explicit "Composite" type
		// because all objects are, in a sense, composites.
		// The default superclass is Object, so if we are
		// a composite, and our superclass is Object, we must
		// declare the C++ class to be a subclass of Composite,
		// meaning DR_Composite and DO_Composite.
		// If the parent's name 

		if (parent_name == "Object")
			parent_name = compname;

		// if we inherit from Composite, use the composite template set.
		if (parent_name == compname || rc->HierarchyContains(compname))
			templateSet = compname;
		else
		// if we inherit from Collection, use the collection template set.
		if (parent_name == colname || rc->HierarchyContains(colname))
			templateSet = colname;

		else
			templateSet = compname;
	}
	else
	{
		// if we inherit from Collection, use the collection template set.
		if (parent_name == colname || rc->HierarchyContains(colname))
			templateSet = colname;
		else
			templateSet = "D";
	}
	
}

