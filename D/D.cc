#include "D.h"
#include <iostream.h>
#include "DClass.h"

int RefCount::newReference(ref_t t)
{
	return t==secondary_ref ? newSecondary() : newPrimary() ;
}

int RefCount::decReference(ref_t t)
{
	return t==secondary_ref ? decSecondary() : decPrimary() ;
}


DRef D::doesNotUnderstand(DR_Message m)
{
#ifdef DMEMORY
	cerr << "D::doesNotUnderstand() message `"
		 << m->message << "'.\n";
#endif

	return DR_null;
}

DR_String D::cpp_classname()
{
//	const type_info& ti = typeid (*this);
	DR_String s = "no rtti"; //ti.name();
	return s;
}

void D::assign(const DRef& obj)
{
	DR_Class cl = Class();
	if (cl.isValid())
		cerr << "assign() not implemented for class `"
			<< DR_Class(Class())->className() << "'\n";
	else
		cerr << "D::assign() not implemented.\n";
}

//DRef D::execute(DR_Message m)
//{
//	return DR_null;
//}

void D::_init()
{
	ref_count.init();
	init();
}

void D::_destroy()
{
	destroy();
}


D::~D()
{
	
}

DRef D::deepCopy() const
{
	cerr << "D::deepCopy(): not implemented.\n";
	return DR_null;
}


DRef D::Class()
{
	return d_class;
}

dcompare_t D::compare(const DRef& d) const
{
	return c_less;
}

// This is a DRef pointing to a DO_Class;
// therefore, to access it as a DR_Class,
// you must do DR_Class(D::d_class)
DRef D::d_class = new DO_Class("Object");


// pointer comparison
int D::operator==(const D& d)	// pointer comparison
{
	return (this == &d);
}

// ****************************
// Global comparison operators!
// ****************************

int operator< (const DRef& left, const DRef& right)
{
	return (left.compare(right) == c_less);
}

int operator<= (const DRef& left, const DRef& right)
{
	dcompare_t dt = left.compare(right);
	return (dt == c_less || dt == c_equal);
}

int operator> (const DRef& left, const DRef& right)
{
	return (left.compare(right) == c_greater);
}

int operator>= (const DRef& left, const DRef& right)
{
	dcompare_t dt = left.compare(right);
	return (dt == c_greater || dt == c_equal);
}


