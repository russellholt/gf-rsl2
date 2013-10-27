// tech/rsl/rsldefaults.h
//
// a centralized location to define various default configuration
// values such as number of hash table buckets. Some of these
// will eventually be figured out by the parser on an
// individual basis (like BUCKETS_IN_METHOD_CONTEXT),
// for greater efficiency.
//
// $Id: rsldefaults.h,v 1.1 1998/11/17 23:56:14 toddm Exp $
#ifndef _RSLDEFAULTS_H_
#define _RSLDEFAULTS_H_

// default option "-server" is shorthand for
// -start <something> where <something> is:
//#define DEFAULT_SERVER "Server.Internal_StartServer"
#define DEFAULT_SERVER "Server.StartServer"

// Name of the environment variable holding a list of
// directories to search for rsl package directories.
// Colon-separated directory list, like $PATH.
#define RSL_CLASSPATH "RSL_CLASSPATH"

// Name of the shared lib res_class creator function symbol (C linkage).
// the full name is formed by prefix + resource name + suffix,
// eg for "String", Create_String_RC
#define CREATE_FN_PREFIX "Create_"
#define CREATE_FN_SUFFIX "_RC"	/* append "__Fv" for gnu C++ mangling */

// size of the free list for each type
// (not in use at this time)
#define FREELIST_MAX 30

// hash buckets for namespaces
//#define BUCKETS_FOR_NAMESPACES 128
#define BUCKETS_FOR_SESSIONS 127

// hash buckets in each namespace
#define BUCKETS_IN_NAMESPACE 10

// hash buckets in SysGlobals context
#define BUCKETS_IN_SYSGLOBALS 50

// hash buckets in local context in executing method
#define BUCKETS_IN_METHOD_CONTEXT 8

// hash buckets for import
// (about expected number of libraries to import)
#define BUCKETS_FOR_IMPORT 5

// when A shares B, a private reference to B is inserted into A's
// context, named by REFNAME_TO_SHARED_OBJ
#define REFNAME_TO_SHARED_OBJ "SharedResource"

// RSL classes (implemented by ResObj) which extend Resources implemented
// in C++ have a subobject for the C++ Resource, inserted into the context
// of the ResObj as a private reference named by CPP_SUBOBJECT_NAME.
// Its name is not directly refernece-able in RSL so that it cannot be
// set or replaced by accident. '\a' is the ASCII char BEL, an esc sequence
// which RSL doesn't support.
//#define CPP_SUBOBJECT_NAME "\aCppSubResource\a"
#define CPP_SUBOBJECT_NAME "_CppSubResource_"

#endif




