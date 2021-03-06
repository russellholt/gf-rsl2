#include "DLibrary.h"
#include "DDictionary.h"
#include "DInt.h"
#include "DString.h"
#include "DBool.h"

#include "DMessage.h"
#include "DSortedBinaryTree.h"
#include "DBinaryTree.h"
#include "DECI_OStream.h"
#include "Dcomparator.h"

#include "DClass.h"

#include "R_D.h"

extern "C" res_class *Create_libD_rsl_RC()
{
	rc_D::dclasses->addClass(DO_Int::Intclass);
	rc_D::dclasses->addClass(DO_Bool::Boolclass);
	rc_D::dclasses->addClass(DO_String::Stringclass);
	rc_D::dclasses->addClass(DO_Dictionary::Dictionaryclass);
	rc_D::dclasses->addClass(DO_BinaryTree::BinaryTreeclass);
	rc_D::dclasses->addClass(DO_SortedBinaryTree::SortedBinaryTreeclass);
	rc_D::dclasses->addClass(DO_ECI_OStream::ECI_OStreamclass);
	rc_D::dclasses->addClass(DO_comparator::comparatorclass);
	rc_D::dclasses->addClass(DO_Message::Messageclass);
	rc_D::dclasses->addClass(DO_Class::Classclass);
	
	return new rc_D("libD_rsl");
}

