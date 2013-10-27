// $Id: ResStream.h,v 1.1 1998/11/17 23:48:32 toddm Exp $

// ****************************************************************************
// *
// *  NAME:              ResStream.h
// *                                                                    
// *  RELATED CLASSES:   Resource, ResObj
// *                                                                    
// *  DESCRIPTION:                                                      
// *    This file provides common macros and function declarations to
// *    perform resource streaming and unstreaming.  The ResStructure
// *    (and ResObj) class provide un/streaming functions within the
// *    class.  However, these functions only work for data members that
// *    are declared in RSL, not in C++.  The tools provided in this file
// *    are to ease the task of adding un/streaming into resources that
// *    have data members declared in C++.  The macros and functions
// *    follow the rules for un/streaming that are encoded in ResStructure
// *    and R_List.  So, any changes in un/streaming that occur in
// *    ResStructure and R_List will also have to be replicated here.
// *    However, if all other classes that need this type of
// *    un/streaming functionality use the tools defined here, the
// *    code duplication is limited to 2 places, and not spread out
// *    across innumerable places.
// *
// *    See R_DateTime and the finance resources for sample usage
// *    of ResStream macros and functions.
// *
// * Copyright (c) 1998 by Destiny Software Corporation
// *
// ****************************************************************************

#include "Resource.h"

#ifndef __RESSTREAM_H
#define __RESSTREAM_H

// C prototypes for streaming functions found in ResStream.cc

extern void  StreamString( RWCString str, ostream &out=cout);
extern void  StreamList( RWTValSlist<ResReference>& the_list, 
                         ostream &out=cout);


// The Stream header consists of the ClassName followed by a space.
// For example, an Account stream begins with 'Account '.  However, if
// a resource is inherited, the ClassName is the name of inheriting
// resource.  The stream header is only needed once - for the inheriting
// resource, not the inherited.  For example, the CCAccount resource
// inherits from Account.  The CCAccount stream begins with 'CCAccount ',
// and the stream does not contain 'Account ' anywhere within it.
// This is handled by the macro below.

#define StreamHeader(out, id)    ( (TypeID() == id) ? \
                                   out << ClassName() << " { " : out << ", ") 

// The stream trailer consists of a closing brace. The inheritance
// issue discussed above for StreamHeader applies to the trailer
// also & is handled by the macro below.

#define StreamTrailer(out, id)   if (TypeID() == id) out << "}" 

// Macros to assist in streaming a RogueWave list data structure

#define StreamListHeader(out)    ( out << "List" << " { " )
#define StreamListTrailer(out)   ( out << "} " )
#define StreamListSeparator(out) ( out << ", " )


// Unstream a string resource by assigning 'str' with the value of the
// resource 'r'.

#define UnStreamString(str, r)        ( str = (r->StrValue()).data() )

#define UnStreamInteger(i, r)         ( i = r->LogicalValue() )

// UnStreamList macro exists privately in finance library.


#endif          /* _RESSTREAM_H */
