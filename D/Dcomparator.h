/************************************************************************


comparator is
	DR_comparator - the "smart pointer" reference to the implementation
	DO_comparator - the implementation

  and probably
	DOC_comparator - class object (referred to through DR_Class)


$Id: Dcomparator.h,v 1.1 1998/11/12 18:53:50 holtrf Exp $
************************************************************************/


#include "D.h"

#include "DList.h"
#include "DDictionary.h"
#include "DComposite.h"

#ifndef _D_comparator_
#define _D_comparator_

#define _D_comparator_ID 1835994116

class DR_comparator;

// a DO_Object class - the guts
class DO_comparator : public DO_Object {

	DR_List fieldNames;
	DR_Dictionary subcomparators;

public:
	int followSecondaryRefs;

	DO_comparator();
	DO_comparator(DRef r); // casting constructor
	virtual ~DO_comparator();

	static DR_Class comparatorclass;
	static DR_comparator New();
	DRef Class();

	DRef route(DR_Message m);
	void init();
	void destroy();
	DR_String toString();
	dcompare_t compare(const DRef& d) const;

	// Methods generated from comparator.rsl
	DR_void addName ( const DR_String& fieldName );
	DR_void addName ( const char *fieldName);
	DR_void setNameList ( DR_List names, ref_t = primary_ref);

	DR_void addSubComparator ( const DR_String& className, DR_comparator c );

	DR_void clearNames (  );
	dcompare_t compare (const DR_Composite& l, const DR_Composite& r );
	DRef findSubCFrom(DRef& ref);


};

// a DR_Object:DRef class - a strongly-typed smart pointer
class DR_comparator : public DR_Object {
public:
	DR_comparator (D *d=0);
	DR_comparator (const DRef& ref);
	virtual ~DR_comparator();

	DO_comparator *const_get() const;
	DO_comparator *safe_get();
	DO_comparator *safe_set(D* d);
	DO_comparator *New();

	inline DO_comparator *operator->() { return safe_get(); }

	inline DR_comparator& operator<< ( const DR_String& fieldName) { safe_get()->addName(fieldName); return *this; }
	inline DR_comparator& operator<< ( const char *fieldName) { safe_get()->addName(fieldName); return *this; }
};
	
#endif

