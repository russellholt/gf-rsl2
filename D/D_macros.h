// **********************************************
// DR macrobatics!
//
// macros for DR_* classes:
//		safe_get(), safe_set(), and New()
//
// $Id: D_macros.h,v 1.1 1998/11/12 18:53:31 holtrf Exp $
// russell...
// **********************************************

#ifndef _D_MACROBATICS
#define _D_MACROBATICS

//
// Defines the body of DR_*::safe_get() in most cases.
//
#define SAFE_GET(T) \
	if (!unsafe_get()) return New(); \
	T *x = dynamic_cast<T *> (unsafe_get()); \
	if (x) \
		return x; \
	return New()

//
// Defines the body of DR_*::safe_set() in most cases.
//
#define SAFE_SET(T,d) \
	T *ds = dynamic_cast<T *> (d); \
	replace(ds);\
	return ds


//
// Defines the body of DR_*::New() in most cases.
//
#define DO_NEW(T) \
	T *dobj = T::New().const_get(); \
	replace(dobj); \
	return dobj


#define toString_OPEN "{ "
#define toString_SEP ", "
#define toString_CLOSE " }"


#endif

