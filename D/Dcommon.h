// ********************************************
// * Destiny D class common headers           *
// ********************************************

/********************
    Sun compiler
*********************/
#ifdef __SUNPRO_CC

#ifdef rtti
#include <typeinfo.h>
#endif

#define BOOLEAN int
#define DR_TRUE 1
#define DR_FALSE 0

#else
/********************
   Others (ie GNU)
*********************/

// this could also be std/typeinfo.h
#include <std/typeinfo.h> 

#define BOOLEAN bool
#define DR_TRUE true
#define DR_FALSE false

#endif


#define DNULL ((D *) 0)

