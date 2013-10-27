// ****************************************************************************
// *
// * $Header: /dest/razor/RAZOR_UNIVERSE/DOMAIN_01/foundation25/Archive/RZ_VCS/granite/granitecore/dutil/src/R_File.h,v 1.1 1998/11/17 23:48:20 toddm Exp $
// * $Id: R_File.h,v 1.1 1998/11/17 23:48:20 toddm Exp $
// *
// *  NAME:  R_File.h
// *
// *  RESOURCE NAME:     File                                        
// *                                                                    
// *  RELATED RESOURCES:                             
// *                                                                    
// *  DESCRIPTION: `File' represents a file on disk. It provides the basic I/O
// *    of read/write/append. Other resources, such as Output, String, and
// *    List, know how to intelligently read/write Files to provide
// *    convenient file system access. Finally, a simple searching
// *    facility is provided for the "bottom line" database file format:
// *    one record per line / tab delimited fields.
// *                                                                    
// * Copyright (c) 1995, 1996, 1997 by Destiny Software Corporation
// *
// ****************************************************************************

// SYSTEM INCLUDES
#include <iostream.h>
#include <fstream.h>
#include <rw/cstring.h>          // RogueWave String Class
#include <rw/tvslist.h>          // RogueWave Slist Class

// LOCAL INCLUDES
#include "res_class.h"
#include "Resource.h"
#include "slog.h"
#include "destiny.h"
//include "runtime.h"

#ifndef _R_FILE_H_
#define _R_FILE_H_

// ******************************************
// *                CONSTANTS               *
// ******************************************
#define R_File_ID 1181314149

// ******************************************************
// *                Class Definitions                   *
// ******************************************************

// *********************************
// * rc_File -- the File RSL class *
// *********************************
class rc_File : public res_class {
    Resource *spawn(RWCString aname);
public:
    rc_File(RWCString aname) : res_class(aname)
    {
//      ResClasses.insert((res_class *) this);
    }
};

// *******************************
// * R_File -- the File Resource *
// *******************************
class R_File : public Resource {

private:    
    // ************************
    // * Private Data Members *
    // *************************
    enum state { closed, open_read, open_write, open_append, open_unknown };

    // ****************************
    // * Private Member Functions *
    // ****************************
    void Init(void)
        { stream_state = closed; }

    void LogIt(int iLevel, RWCString& strMessage);


    // R_File RSL methods
    ResStatus rsl_PrintFiles(const ResList& arglist);
    ResStatus rsl_Open(const ResList& arglist);
    ResStatus rsl_Close(const ResList& arglist);
    ResStatus rsl_Write(const ResList& arglist);
    ResStatus rsl_ReadFile(const ResList& arglist);
    ResStatus rsl_ReadLine(const ResList& arglist);
    ResStatus rsl_SetName(const ResList& arglist);

protected:
    // ******************
    // * Protected Data *
    // ******************
    RWCString filename;
    state stream_state; // see "enum state {...}" above
    fstream the_stream;

    RWCString strMessage;

    // ******************************
    // * Protected Member Functions *
    // ******************************

public:
    // ***********************
    // * Public Data Members *
    // ***********************
    static rc_File rslType;


    // ******************************
    // * Constructors & Destructors *
    // ******************************
    R_File(void) { Init(); name = "File"; }
    R_File(RWCString n) { Init(); name = n; }
    
    // *************
    // * Operators *
    // *************

    // ***************************
    // * Public Member Functions *
    // ***************************
    static R_File *New(RWCString n);
    int     PrintFile(RWCString fname="", ostream &out=cout);
    int     Open();
    int     Open(RWCString strFName, state newstate=open_read);
    void    Close(void);
    int     Write(ResReference &r, int inlist);
    void    Gets(fstream& in, RWCString& strText); 
    int     ReadFile(ResReference &r);
    int     ReadLine(ResReference &r);

    // *********************
    // * Resource virtuals *
    // *********************

    // Info
    unsigned int TypeID() { return R_File_ID; }
    res_class *memberOf(void) { return &rslType; }
    RWCString StrValue(void);
    int LogicalValue();
    
    // Execution
    ResStatus execute(int method, ResList& arglist);
    void Clear();
    
    // output
//    void print(ostream &out=cout) { PrintFile(filename, out); }  // ECI
    void rslprint(ostream &out=cout) { PrintFile(filename, out); }

};

#endif

/*****************************************************************************
 <<Begin Resource Documentation>>


RESOURCE NAME: File

RELATED RESOURCES: Output, String, List

DESCRIPTION: `File' represents a file on disk. It provides the basic I/O
    of read/write/append. Other resources, such as Output, String, and
    List, know how to intelligently read/write Files to provide
    convenient file system access.

PUBLIC MEMBER FUNCTIONS:

    open(String name, String mode)
        Open a file by name. Mode is one of "read", "write", "append".
        At this time, "read" is not supported since there is no
        direct "read" method; a file may only be read in its entirety
        (see the "file" method).

    close

    write(...)
        Write given resources to the file, according to the mode
        as given in a previous open.


 <<End Resource Documentation>>
 ****************************************************************************/
