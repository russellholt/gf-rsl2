// $Id: DRef.h,v 1.2 1998/11/24 14:56:12 holtrf Exp $

#include "D.h"

#ifndef DREF_DESTINY
#define DREF_DESTINY


class DRef : public D {
	D *that;

public:
	DRef(D *d=0);
	DRef(D *d, ref_t kind);
	DRef(const DRef& dref);
	DRef(const DRef& dref, ref_t kind);
	virtual ~DRef();

	inline D *unsafe_get() const { return that; }
	
	// convenience pass-throughs
	void init();
	void destroy();
	DR_String toString();
	BOOLEAN toBoolean() const;
	unsigned dtypeid();
	dcompare_t compare(const DRef& d) const;
	void assign(const DRef& obj);
	DRef Class();
	DRef route(DR_Message m);

	// A variety of ways to set the pointer
	virtual void dump(D* d=0, ref_t ref_kind=primary_ref);
	inline void replace(D* d, ref_t ref_kind = primary_ref) { dump(d, ref_kind); }
	inline void replaceWithPrimary(D* d) { dump(d, primary_ref); }
	inline void replaceWithSecondary(D* d) { dump(d, secondary_ref); }
	inline void setPrimary(D* d) { dump(d, primary_ref); }
	inline void setSecondary(D* d) { dump(d, secondary_ref); }

	inline BOOLEAN isValid() const { return that != DNULL; }

	inline ref_t RefKind() const { return refKind; }
	
	DRef& operator=(const DRef& dref);
	DRef& operator=(D *d);
	int operator==(const DRef& dref);
	int operator==(const D* d);
	int operator!=(const DRef& dref);

	// Don't support recycling of DRef objects themselves by denying decRef()
	inline int decRef() { return refCount(); }
	inline int newRef() { return ++ref_count; }
	
protected:
	inline void _unsafe_clear() { that = DNULL; }
	virtual void Recycle();

	ref_t refKind; // D.h
};

// killer_ref
//	kills when refcount <= 0
// used by some recyclers or in use managers
class killer_ref : public DRef {
	public:
		killer_ref(D* d=0);
		virtual ~killer_ref();
		void Recycle();	// deletes.
};


#define DR_null (DNULL)
#define DR_void DRef

#endif
