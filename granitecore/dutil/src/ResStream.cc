// $Id: ResStream.cc,v 1.1 1998/11/17 23:47:12 toddm Exp $

// ****************************************************************************
// *
// *  NAME:              ResStream.cc
// *                                                                    
// *  RELATED CLASSES:   Resource, ResObj
// *                                                                    
// *  DESCRIPTION:                                                      
// *    This file provides common functions to  perform 
// *    resource streaming and unstreaming.  
// *
// *    See ResStream.h for more complete documentation.
// *
// * Copyright (c) 1998 by Destiny Software Corporation
// *
// ****************************************************************************

static char rcsid[] = "$Id:";

// SYSTEM INCLUDES

// LOCAL INCLUDES
#include "drwcstring.h"
#include "ResStream.h" 
#include "destiny.h"

// The rules for { Name = xxx, ... } are from ResStructure::print().
// Copied from R_String::print() & R_String::OutEscape()


void StreamString(RWCString str, ostream &out)
{
    //out << '\"' << OutEscape() << '\"';
    out << '\"' ;

    DRWCString s(str);
    s.replace("\n", "\\n");
    s.replace("\t", "\\t");
    s.replace("\r", "\\r");
    s.replace("\"", "\\\"");

    out << RWCString(s.data());

    out << '\"';
}

// Stream a RW list by printing a list resource header, followed by
// streaming of each list element.
// Rules for streaming a list are from R_List.

void StreamList(RWTValSlist<ResReference>& the_list, ostream &out)
{
    RWTValSlistIterator<ResReference> iter(the_list);
    ResReference       RefElement;
    bool               first = TRUE;

    StreamListHeader(out);

    while ( ++iter )
    {
        if (!first)
            StreamListSeparator(out);
        else
            first = FALSE;
        RefElement = iter.key();
        RefElement.print(out);
    }

    StreamListTrailer(out);
}

// Unstreaming stuff.  Function was written to unstream Transactions list.
// Does not seem to be necessary.  Seem to be able to get list & assign
// using UnStreamList macro.  But, just in case, that runs into
// memory management difficulties, the function is still here.

#ifdef MAYBE
void R_Statement::UnStreamList( RWTValSlist<ResReference>& the_list)
{
cout << "Length of list to be unstreamed is= " << the_list.entries() << "\n";

    RWTValSlistIterator<ResReference> iter(the_list);
    ResReference RefEl=NULL;

    while(iter())
    {
        RefEl = iter.key();

        Resource *r;
        r = RefEl.clone();
        Transactions.append(r);
    }
}
#endif

