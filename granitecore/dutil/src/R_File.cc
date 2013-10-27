// ***************************************************************************
// *
// * $Header: /dest/razor/RAZOR_UNIVERSE/DOMAIN_01/foundation25/Archive/RZ_VCS/granite/granitecore/dutil/src/R_File.cc,v 1.1 1998/11/17 23:47:00 toddm Exp $
// * $Id: R_File.cc,v 1.1 1998/11/17 23:47:00 toddm Exp $
// *
// *  NAME:              R_DateTime.cc
// *
// *  RESOURCE NAME:     DateTime                                        
// *                                                                    
// *  RELATED RESOURCES: 
// *                                                                    
// *  DESCRIPTION:                                                      
// *                                                                    
// *     Date & Time Resource based on RogueWave RWTime object
// *                                                                    
// * Copyright (c) 1995, 1996, 1997 by Destiny Software Corporation
// *
// * Russell Holt, created July 24 1995
// *
// ***************************************************************************

// SYSTEM INCLUDES

// LOCAL INCLUDES
#include "R_File.h"
#include "R_String.h"
#include "R_Boolean.h"
#include "R_List.h"

// ******************************************
// *                CONSTANTS               *
// ******************************************
#define _hSEND 1399156324       // Send
#define _hPRINT 611477870       // Print
#define _hOPEN 1332766062       // Open
#define _hCLOSE 644640627       // Close
#define _hWRITE 846358900       // Write
#define _hOpLS 15420            // <<
#define _hREADLINE 504106753    // ReadLine
#define _hREADFILE 336334081    // ReadFile
#define _hOpRS 15934            // >>
#define _hSETNAME 839389518     // SetName

// R_File static member
rc_File R_File::rslType("File");


extern "C" res_class *Create_File_RC()
{
	return &(R_File::rslType);
}

// ************************************************************************
// ************************************************************************
// *                       RES_CLASS - rc_File                            *
// ************************************************************************
// ************************************************************************

// ************************************************************************
// *
// * NAME:  spawn - Private function
// *
// * DESCRIPTION:
// *        Create a new resource of this type (R_File).
// *        Called by res_calss::New() if there is no object
// *        To pull off the free list.
// *
// * INPUT:
// *        None 
// *
// * RETURNS:
// *        None
// *
// ************************************************************************
Resource* rc_File::spawn(RWCString nm)
{
	return new R_File(nm);	// or other constructor
}

// ************************************************************************
// ************************************************************************
// *                        RESOURCE - R_File                             *
// ************************************************************************
// ************************************************************************

// ******************************************************************
// *                    PRIVATE MEMBER FUNCTIONS                    *
// ******************************************************************

// ************************************************************************
// *
// * NAME:  LogIt - Private function
// *
// * DESCRIPTION:
// *        Print a message to the system log file.
// *
// * INPUT:
// *         iLevel - Number specifing the loging level
// *         strMsg - Message to log.
// *
// * RETURNS:
// *        None
// *
// ************************************************************************
void R_File::LogIt(int iLevel, RWCString& strMsg)
{
    DRWCString strText = strMsg;

#ifdef DEBUG
    DRWCString strClassNm, strNm;

    strClassNm = ClassName();
    strNm = Name();
    cout << strNm << ":" << strClassNm << "::" << strText << ".\n"; 
#endif

    if (iLevel == _EMERGENCY)
        logf->emergency(LOGSERVER) << "(R_File) - " << strText  << endline;
    else if (iLevel == _FATAL)
        logf->fatal(LOGSERVER) << "(R_File) - " << strText  << endline;
    else if (iLevel == _ERROR)
        logf->error(LOGSERVER) << "(R_File) - " << strText  << endline;
    else if (iLevel == _ALERT)
        logf->alert(LOGSERVER) << "(R_File) - " << strText  << endline;
    else if (iLevel == _NOTICE)
        logf->notice(LOGSERVER) << "(R_File) - " << strText  << endline;
    else if (iLevel == _INFO)
        logf->info(LOGSERVER) << "(R_File) - " << strText  << endline;
    else if (iLevel == _DEBUG)
        logf->debug(LOGSERVER) << "(R_File) - " << strText  << endline;
}


// ************************************************************************
// *
// * NAME:  PrintFiles - Private function
// *
// * DESCRIPTION:
// *        print a list of files named in args to stdout
// *        if there are no args, then print the file named by R_File::filename
// *        Calls R_File::PrintFile to print the files.
// *        - maybe this is trying to do too many things in one class, and this
// *        method probably won't be used very often anyway, if ever.
// *        Even though it is legal and would work, it doesn't really make much
// *        sense to say (in rsl):
// *
// *            File a;
// *            a.open("myfile", read);
// *            a.printfiles("file1", "file2", "file3");
// *
// *        although it does make sense to say:
// *
// *            File.printfiles("file1", "file2", "file3");
// *
// * INPUT:
// *       args    Optional. List of files to print
// *         
// *
// * RETURNS:
// *        None
// *
// ************************************************************************
ResStatus R_File::rsl_PrintFiles(const ResList& arglist)
{
    int iReturnStatus;

    if (!arglist.isEmpty())
    {
        ResReference r;
        for(int i=0; i<arglist.entries(); i++)
        {
            r = arglist[i];
            if (!PrintFile(r.StrValue()))  // R_File::PrintFile()
            {
                iReturnStatus = 0;          
            }
        }
    }
    else
    {
        iReturnStatus = PrintFile();
    }

    R_Boolean *rStatus = R_Boolean::New("Status", iReturnStatus);
    return(ResStatus(ResStatus::rslOk, rStatus));
}


// ************************************************************************
// *
// * NAME:  Open - Private function
// *
// * DESCRIPTION:
// *        Opens a file with a mode
// *        expects a filename and a mode string, one of "read", "write",
// *        "append".  If no arguments are given, it tries to use 
// *        R_File::filename with open_append as the default mode.
// *
// * INPUT:
// *       args    Optional. 
// *        arg1    - String containing the filename of file to open
// *        arg2    - String containing the mode to open the file with
// *         
// * RETURNS:
// *        Nothing
// *
// ************************************************************************
ResStatus R_File::rsl_Open(const ResList& arglist)
{
    int iReturnStatus;
    RWCString strFName, strFMode;

    // **************************************************
    // * If no arg, open current filename for read only *
    // **************************************************
    if (!arglist.isEmpty())
    {
        // ********************
        // * Get the filename *
        // ********************
        strFName = arglist[0].StrValue();

        // ************************************************************
        // * If there are two argument, Then the 2nd is the file mode *
        // ************************************************************
        if (arglist.entries() > 1)
        {
            state st;
            strFMode = arglist[1].StrValue();
       
            // ********************
            // * Analyze the Mode *
            // ********************
            strFMode.toLower();        

            if (strFMode == "read")
            {
                st = open_read;
            }
            else if (strFMode == "write")
            {
                st = open_write;
            }
            else if (strFMode == "append") 
            {
                st = open_append;
            }
            else
            {
                st = open_unknown;
            }

            iReturnStatus = Open(strFName, st);
        }
        else
        {
            iReturnStatus = Open(strFName);
        }
    }
    else
    {
        iReturnStatus = Open();
    }
    
    // *****************
    // * Return Status *
    // *****************
    R_Boolean *rStatus = R_Boolean::New("Status", iReturnStatus);
    return(ResStatus(ResStatus::rslOk, rStatus));
}


// ************************************************************************
// *
// * NAME:  Close - Private function
// *
// * DESCRIPTION:
// *        Closes the file specifed by filename
// *
// * INPUT:
// *        None
// *         
// * RETURNS:
// *        Nothing
// *
// ************************************************************************
ResStatus R_File::rsl_Close(const ResList& arglist)
{
    Close();
	return ResStatus(ResStatus::rslOk, NULL);
}

// ************************************************************************
// *
// * NAME:  Write - Private function
// *
// * DESCRIPTION:
// *        Append the Value of each argument to the file
// *
// * INPUT:
// *       args
// *        Any number of resources to write to the specifed file.
// *        
// * RETURNS:
// *        Nothing
// *
// ************************************************************************
ResStatus R_File::rsl_Write(const ResList& arglist)
{
    int iReturnStatus=1;

    // ******************************************
    // * Is the file opened for write or append *
    // ******************************************
    if ((stream_state != open_write) && (stream_state != open_append))
    {
        strMessage = "Write - stream not open for write or append\n";
        LogIt( _ERROR, strMessage);
        iReturnStatus = 0;
    }
    else
    {
        ResReference r;

        // ***********************************
        // * Write all the resouces listed   *
        // * in the args to the file         *
        // ***********************************
        for(int i=0; i<arglist.entries(); i++)
        {
            r = arglist[i];
            if (!Write(r, 0))
                iReturnStatus = 0;
        }
    }

    // *****************
    // * Return Status *
    // *****************
    R_Boolean *rStatus = R_Boolean::New("Status", iReturnStatus);
    return(ResStatus(ResStatus::rslOk, rStatus));
}


// ************************************************************************
// *
// * NAME:  rsl_ReadFile - Private function
// *
// * DESCRIPTION:
// *        Read a file into the specfied resourcee
// *
// * INPUT:
// *         
// * RETURNS:
// *        Nothing
// *
// ************************************************************************
ResStatus R_File::rsl_ReadFile(const ResList& arglist)
{
    int iReturnStatus;

    ResReference r = arglist[0];

    iReturnStatus = ReadFile(r);

    // *****************
    // * Return Status *
    // *****************
    R_Boolean *rStatus = R_Boolean::New("Status", iReturnStatus);
    return(ResStatus(ResStatus::rslOk, rStatus));
}


// ************************************************************************
// *
// * NAME:  rsl_ReadLine - Private function
// *
// * DESCRIPTION:
// *        Read the next line from the file into a string resource
// *
// * INPUT:
// *         
// * RETURNS:
// *        Nothing
// *
// ************************************************************************
ResStatus R_File::rsl_ReadLine(const ResList& arglist)
{
    int iReturnStatus;

    ResReference r = arglist[0];

    iReturnStatus = ReadLine(r);

    // *****************
    // * Return Status *
    // *****************
    R_Boolean *rStatus = R_Boolean::New("Status", iReturnStatus);
    return(ResStatus(ResStatus::rslOk, rStatus));
}


// ************************************************************************
// *
// * NAME:  rsl_SetName - Private function
// *
// * DESCRIPTION:
// *        Change the filename of the file.
// *
// * INPUT:
// *        
// *         
// * RETURNS:
// *        Nothing
// *
// ************************************************************************
ResStatus R_File::rsl_SetName(const ResList& arglist)
{
    int iReturnStatus=1;
    ResReference r = arglist[0];

    if (stream_state == closed) 
    {
        filename = r.StrValue();
    }
    else
    {
        strMessage = "rsl_SetName - file already open.\n";
        LogIt( _ERROR, strMessage);
        iReturnStatus = 0;
    }

    // *****************
    // * Return Status *
    // *****************
    R_Boolean *rStatus = R_Boolean::New("Status", iReturnStatus);
    return(ResStatus(ResStatus::rslOk, rStatus));
}


// *************************************************************************
// **                         PUBLIC MEMBER FUNCTIONS                      *
// **                                                                      *
// **         Virtual functions, overriding those in 'resource'            *
// *************************************************************************

// ***********************************************************************
// *
// * NAME:    execute         Public Function
// *
// * DESCRIPTION:
// *      RSL interface to this class. 
// *
// * INPUT: 
// *      method -  hash value for a method within the       
// *                resource                                  
// *      arglist - argument list.                           
// *                                                                  
// *  RETURNS: 
// *      ResStatus
// *                                                                  
// ************************************************************************
ResStatus R_File::execute(int method, ResList& arglist)
{
	switch(method)
	{
		case _hSEND:        // "Send"
		case _hPRINT:       // "Print"
            return  rsl_PrintFiles(arglist);

		case _hOPEN:        // "Open"
            return  rsl_Open(arglist);

		case _hCLOSE:       // "Close"
            return rsl_Close(arglist);

		case _hWRITE:       // "Write"
        case _hOpLS:
            return rsl_Write(arglist);

		case _hREADFILE:    // "ReadFile"
        case _hOpRS:
            return rsl_ReadFile(arglist);

		case _hREADLINE:    // "ReadLine"
            return rsl_ReadLine(arglist);

		case _hSETNAME:     // "Setname"
			return rsl_SetName(arglist);

		default: ;
	}

	return ResStatus(ResStatus::rslFail);
}


// ************************************************************************
// *                                                                       
// * NAME:    StrValue             Public Function                           
// *                                                                       
// * DESCRIPTION:                                                          
// *        Return the file contents as the string value
// *                                                                       
// * INPUT:                                                                
// *                                                                       
// * RETURNS:                                                              
// *      Returns an RWCString
// *                                                                       
// ************************************************************************
RWCString R_File::StrValue(void)
{
    RWCString strText;
    RWCString strFile;

    if (filename == "")
    {
        strMessage = "StrValue - No file name given.\n";
        LogIt( _ERROR, strMessage);
        return("");
    }

    fstream ifile(filename.data(), ios::in);
    if (!ifile)
    {
        strMessage = "StrValue - unable to open file " + filename + ".\n";
        LogIt( _ERROR, strMessage);
    }

    // ***********************************************
    // * Read each line of the file and write it out *
    // ***********************************************
    while(ifile.good())
    {
        Gets(ifile, strText);
        strFile += strText + "\n";
    }

    return(strFile);
}

// ************************************************************************
// *                                                                       
// * NAME:    LogicalValue             Public Function                           
// *                                                                       
// * DESCRIPTION:                                                          
// *        Evaluate the "trueness" of this Resource (1 for true, 0 for false)
// *        Used in logical comparisons.
// *                                                                       
// * INPUT:                                                                
// *                                                                       
// * RETURNS:                                                              
// *      Returns an int
// *                                                                       
// ************************************************************************
int R_File::LogicalValue()
{
    if (stream_state == closed)
        return(0);
    else
        return(1);
}

// ************************************************************************
// *                                                                       
// * NAME:    IsEqual             Public Function                           
// *                                                                       
// * DESCRIPTION:                                                          
// *        Test for equality with another Resource.
// *                                                                       
// * INPUT:                                                                
// *                                                                       
// * RETURNS:                                                              
// *      Returns an int
// *                                                                       
// ************************************************************************
/*
int R_File::IsEqual(Resource *r)
{
}
*/

// ************************************************************************
// *                                                                       
// * NAME:    SetFromInline             Public Function                           
// *                                                                       
// * DESCRIPTION:                                                          
// *        Given a list of Resources, match with a data member of the same
// *        name and assign. eg, in RSL, "myclass { a:1, b:2, /* etc */ }"
// *        an object of type `myclass' is created and SetFromInline() is
// *        called for the list of resources enclosed in { }.
// *        (ResStructure provides a default version)
// *                                                                       
// * INPUT: inliner - List of Resources
// *                                                                       
// * RETURNS:                                                              
// *      Nothing
// *                                                                       
// ************************************************************************
/*
void R_File::SetFromInline(RWTPtrSlist<Resource>& inliner)
{
}
*/

// ************************************************************************
// *                                                                       
// * NAME:    Assign             Public Function                           
// *                                                                       
// * DESCRIPTION:                                                          
// *        set this resource equal to r.
// *        (ResStructure provides a default version)
// *                                                                       
// * INPUT: r - Resource to assign
// *                                                                       
// * RETURNS:                                                              
// *      Nothing
// *                                                                       
// ************************************************************************
/*
void R_File::Assign(Resource *r)
{
}
*/

// ************************************************************************
// *                                                                       
// * NAME:    Clear             Public Function                           
// *                                                                       
// * DESCRIPTION:                                                          
// *        Memory management - called to restore an object
// *        to a "just created" state, for free-list management.
// *        (ResStructure provides a default version)
// *                                                                       
// * INPUT: None
// *                                                                       
// * RETURNS:                                                              
// *      Nothing
// *                                                                       
// ************************************************************************
void R_File::Clear()
{
    Close();
}


// *************************************************************************
// **                       PUBLIC MEMBER FUNCTIONS                        *
// **                                                                      *
// **               Class specific public member functions                 *
// *************************************************************************

// ************************************************************************
// *                                                                       
// * NAME:    New             Public Function                           
// *                                                                       
// * DESCRIPTION:                                                          
// *        static convenience function
// *        See R_String::New(), R_Integer::New(), etc.
// *        If no functionality beyond res_class::New() is required
// *        (ie, no special values to be set conveniently), then this
// *        function simply eliminates the need to cast the result
// *        of res_class::New().
// *                                                                       
// * INPUT:                                                                
// *      RWCString containing the object name.
// *                                                                       
// * RETURNS:                                                              
// *      Returns a R_File
// *                                                                       
// ************************************************************************
R_File *R_File::New(RWCString n)
{
	Resource *r= R_File::rslType.New(n);
	return (R_File *) r;
}


// ************************************************************************
// *
// * NAME:  PrintFile - Public function
// *
// * DESCRIPTION:
// *        use ifstream to read a named file. Print it to stdout.
// *        This is easy but not efficient. Rewrite to use system calls
// *        read and write to be fast.
// *        - if fname == "" then use R_File::filename
// *
// * INPUT:
// *       fname    Optional RWCString specifing the file to print
// *         
// *
// * RETURNS:
// *        None
// *
// ************************************************************************
int R_File::PrintFile(RWCString fname, ostream &MyOut)
{
    RWCString strPrint;

    if (fname == "")
    {
        strMessage = "PrintFile - No file name given.\n";
        LogIt( _ERROR, strMessage);
        return(0);
    }

    fstream ifile(fname.data(), ios::in);
    if (!ifile)
    {
        strMessage = "PrintFile - unable to open file " + fname + "\n";
        LogIt( _ERROR, strMessage);
        return(0);
    }

    // ***********************************************
    // * Read each line of the file and write it out *
    // ***********************************************
    while(ifile.good())
    {
        Gets(ifile, strPrint);
        strPrint += "\n";
        MyOut << strPrint;
    }

    return(1);
}


// ************************************************************************
// *
// * NAME:  Open - Public function
// *
// * DESCRIPTION:
// *        Opens a stream for the file named by R_File::filename
// *        or verifies that it is already opened in the requested state.
// *        If the stream is already open but in a different state than
// *        requested, it will not change state or reopen, just fail.
// *
// * INPUT:
// *        None
// *
// * RETURNS:
// *        1 - Successfull
// *        0 - Otherwise
// *
// ************************************************************************
int R_File::Open(void)
{
    // ******************************
    // * Is the file already opened *
    // ******************************
    if (stream_state != closed)
    {
        strMessage = "Open - file already open.\n";
        LogIt( _ERROR, strMessage);
        return(0);
    }

    if (filename == "") 
    {
        strMessage = "Open - No file name given.\n";
        LogIt( _ERROR, strMessage);
        return(0);
    }

    // *********************************
    // * mode ios::in - open for input *
    // *********************************
    the_stream.open(filename.data(), ios::in);
    if (the_stream.good())  //  if (the_stream)
    {
        stream_state = open_read;
        return(1);
    }

    strMessage = "Open - Unable to open file " + filename + ".\n";
    LogIt( _ERROR, strMessage);
    return(0);
}

// ************************************************************************
// *
// * NAME:  Open - Public function
// *
// * DESCRIPTION:
// *        Opens a stream for the file named by R_File::filename
// *        or verifies that it is already opened in the requested state.
// *        If the stream is already open but in a different state than
// *        requested, it will not change state or reopen, just fail.
// *
// * INPUT:
// *        strFName - String containing the name of the file to open
// *        newstate - The state to open the file with open_read, open_write, 
// *                    open_append
// *         
// *
// * RETURNS:
// *        1 - Successfull
// *        0 - Otherwise
// *
// ************************************************************************
int R_File::Open(RWCString strFName, state newstate)
{
    // ******************************
    // * Is the file already opened *
    // ******************************
    if (stream_state != closed)
    {
        strMessage = "Open - file already open.\n";
        LogIt( _ERROR, strMessage);
        return(0);
    }

    if (strFName == "") 
    {
        strMessage = "Open - No file name given.\n";
        LogIt( _ERROR, strMessage);
        return(0);
    }

    // *****************
    // * Open for read *
    // *****************
    if (newstate == open_read)
    {
        // mode ios::in - open for input
        the_stream.open(strFName.data(), ios::in);
        if (the_stream.good())  //  if (the_stream)
        {
            stream_state = newstate;
            filename = strFName;
            return(1);
        }
    }

    // *******************
    // * Open for append *
    // *******************
    else if (newstate == open_append)
    {
        // mode ios::app - append
        the_stream.open(strFName.data(), ios::app);
        if (the_stream.good())  // if (the_stream)
        {
            stream_state = newstate;
            filename = strFName;
            return(1);
        }
    }

    // ******************
    // * Open for write *
    // ******************
    else if (newstate == open_write)
    {
        // mode ios::out - open for output
        the_stream.open(strFName.data(), ios::out);
        if (the_stream.good())
        {
            stream_state = newstate;
            filename = strFName;
            return(1);
        }
    }

    // *********************
    // * Unknown file mode *
    // *********************
    else
    {
        strMessage = "Open - Requires a file mode, one of \"read\", \"write\", or \"append\"\n";
        LogIt( _ERROR, strMessage);
        return(0);
        
    }

    strMessage = "Open - Unable to open file " + strFName + ".\n";
    LogIt( _ERROR, strMessage);
    return(0);
}


// ************************************************************************
// *
// * NAME:  Close - Public function
// *
// * DESCRIPTION:
// *        Close the stream, set its state to 'closed', and blank the 
// *        file name.
// *
// * INPUT:
// *        None
// *
// * RETURNS:
// *        Nothing
// *
// ************************************************************************
void R_File::Close(void)
{
    if (stream_state != closed)
    {
        the_stream.close();
    }

    stream_state = closed;
    filename = "";
}


// ************************************************************************
// *
// * NAME:  Write - Public function
// *
// * DESCRIPTION:
// *        Writes the specifed string to the file.
// *
// * INPUT:
// *
// * RETURNS:
// *        1 - Successfull
// *        0 - Otherwise
// *
// ************************************************************************
int R_File::Write(ResReference &r, int inlist) 
{
    
    // ******************************************
    // * Is the file opened for write or append *
    // ******************************************
    if ((stream_state != open_write) && (stream_state != open_append))
    {
        strMessage = "Write - File not open for write or append.\n";
        LogIt( _ERROR, strMessage);
        return(0);
    }

    if (r.ClassName() == "List")
    {
        R_List *rl;
        rl = (R_List *) r();

        // ************************************************************
        // * Loop through the list and recursively write each element *
        // ************************************************************
        RWTValSlist<ResReference>& theList = rl->GetList();
        RWTValSlistIterator<ResReference> iter(theList);

        ResReference rElement;

        while (iter())
        {
            rElement = iter.key();

            Write(rElement, 1);
        }
    }
    else
    {
        RWCString strText = r.StrValue();

        if (inlist)
        {
            if ((strText.length() == 0) || (strText[strText.length()-1] != '\n'))
                strText += '\n';
        }

        // ******************************
        // * Write the text to the file *
        // ******************************
        the_stream.write(strText.data(), strText.length());
    }

    return(1);
}


// ************************************************************************
// *
// * NAME:  Gets - Public function
// *
// * DESCRIPTION:
// *        Reads a line from the specifed fstream into the specified
// *        string.
// *
// * INPUT: 
// *        in - fstream specifing the file stream to read from
// *    
// * OUTPUT:
// *        strText - Read a line from the file in to this string.
// *
// * RETURNS:
// *        Nothing
// *
// ************************************************************************
void R_File::Gets(fstream& in, RWCString& strText) 
{
	strText.readLine(in);
}


// ************************************************************************
// *
// * NAME:  ReadLine - Public function
// *
// * DESCRIPTION:
// *        Read a line from the file and return it in the specified string
// *
// * INPUT: 
// *        Nothing
// *    
// * OUTPUT:
// *        R_String - String return the line read from the file.
// *
// * RETURNS:
// *        1 - Successfull
// *        0 - Otherwise
// *
// ************************************************************************
int R_File::ReadLine(ResReference &r)
{
    RWCString strLine;

    // *******************************
    // * Is the file opened for read *
    // *******************************
    if (stream_state != open_read)
    {
        strMessage = "Read - File not open for read.\n";
        LogIt( _ERROR, strMessage);
        return(0);
    }

    // **************************
    // * If no error or not EOF *
    // **************************
    if (the_stream.good())
    {
        Gets(the_stream, strLine);

        R_String *rs;
        rs = (R_String *) r();
        
        rs->Set(strLine);
        return(1);
    }
        
    return(0);
}

// ************************************************************************
// *
// * NAME:  ReadFile - Public function
// *
// * DESCRIPTION:
// *        Take the contents of myfile and put it into
// *        whatever resource "expression" evaluates to.
// *        This means we must do different things based on the type
// *        of the "expression" (the resource).
// *
// * INPUT: 
// *        Nothing
// *    
// * OUTPUT:
// *        Resource - Resource returning the contents of the file.
// *
// * RETURNS:
// *        1 - Successfull
// *        0 - Otherwise
// *
// ************************************************************************
int R_File::ReadFile(ResReference &r)
{
    RWCString strLine, strFile;

    // *******************************
    // * Is the file opened for read *
    // *******************************
    if (stream_state != open_read)
    {
        strMessage = "Read - File not open for read.\n";
        LogIt( _ERROR, strMessage);
        return(0);
    }

    // **************************************************
    // * First check arg type.  expecting only one arg. *
    // **************************************************
    if (r.ClassName() == "List")    // List
    {
        R_List *rl;
        rl = (R_List *) r();

        RWTValSlist<ResReference>& theList = rl->GetList();

        while(the_stream.good())
        {
            Gets(the_stream, strLine);
            ResReference ref(R_String::New("", strLine));
            theList.append(ref);
        }
    }
    else
    {
        while(the_stream.good())
        {
            Gets(the_stream, strLine);
            strFile += strLine + "\n";
        }

        R_String *rs;
        rs = (R_String *) r();
        
        rs->Set(strFile);

    }

    return(1);
}



