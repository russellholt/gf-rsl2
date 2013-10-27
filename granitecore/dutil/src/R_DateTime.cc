// ***************************************************************************
// *
// * $Header: /dest/razor/RAZOR_UNIVERSE/DOMAIN_01/foundation25/Archive/RZ_VCS/granite/granitecore/dutil/src/R_DateTime.cc,v 1.1 1998/11/17 23:46:57 toddm Exp $
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
// * Copyright (c) 1995-1998 by Destiny Software Corporation
// * 
// ***************************************************************************

// SYSTEM INCLUDES
#include <stdio.h>          // Standard I/O
#include <stddef.h>         // Standard Definitions
#include <stdlib.h>         // Standard Library
#include <time.h>
#include <ctype.h>          // For isdigit() prototype

// LOCAL INCLUDES
#include "R_DateTime.h"
#include "ResStream.h"      // Resource Streaming macros & functions

static char rcsid[] = "$Id: R_DateTime.cc,v 1.1 1998/11/17 23:46:57 toddm Exp $";

// ******************************************
// *                CONSTANTS               *
// ******************************************

// R_DateTime method name hash definitions
#define _hDAY           4481401         // Day
#define _hMONTH         628059764       // Month
#define _hYEAR          1499816306   	// Year
#define _hISNULL        622808693  	// IsNull
#define _hOpLT          60 		// <
#define _hOpGT          62   		// >
#define _hOpEQ2         15677   	// ==
#define _hOpNE          8509		// !=
#define _hOpLE          15421		// <=
#define _hOpGE          15933		// >=
#define _hTIME          1416195429	// Time
#define _hOpEQ          61		// =
#define _hASSIGN        639464297	// Assign
#define _hFORMAT        656110189	// Format
#define _hINPUT_FORMAT  1331451758      // Input_Format
#define _hOUTPUT_FORMAT 561202775       // Output_Format
#define _hPRINT         611477870   	// Print


// R_DateTime static member
rc_DateTime *R_DateTime::rslType = NULL;

extern "C" res_class *Create_DateTime_RC()
{
	return R_DateTime::rslType = new rc_DateTime("DateTime");
}

// ************************************************************************
// ************************************************************************
// *                       RES_CLASS - rc_DateTime                        *
// ************************************************************************
// ************************************************************************

// ************************************************************************
// *
// * NAME:  spawn - Private function
// *
// * DESCRIPTION:
// *        Create a new resource of this type (R_DateTime).
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
Resource *rc_DateTime::spawn(RWCString nm)
{
    return( new R_DateTime(nm) );
    
}    

// ************************************************************************
// ************************************************************************
// *                        RESOURCE - R_DateTime                         *
// ************************************************************************
// ************************************************************************

// ******************************************************************
// *                    PRIVATE MEMBER FUNCTIONS                    *
// ******************************************************************

// ************************************************************************
// *
// * NAME:  Init, init_the_time - Private function
// *
// * DESCRIPTION:
// *        Initialize class data members: year is set to current year & all
// *        other date/time members are zeroed.
// *        Defaults date-time input/output format strings to mm/dd/yy.
// *
// * INPUT:
// *        None 
// *
// * RETURNS:
// *        None
// *
// ************************************************************************
void R_DateTime::init_the_time(void)
{
    hour = min = sec = mon = day = 0;
    year = RWDate().year();
}

void R_DateTime::Init(void)
{
    the_time = NO_DATE;
    input_format  = "%m/%d/%y";
    output_format = "%m/%d/%y";
    formatted_date_time = "";
    init_the_time();
}


// ************************************************************************
// *
// * NAME:  convert_string      Private function
// *
// * DESCRIPTION:
// *        Verifies string contains numeric characters, and then uses
// *        atoi to convert to integer
// *
// * INPUT:
// *        s   String to convert to integer
// *
// * RETURNS:
// *        -1 or integer value of converted string
// *
// ************************************************************************
int R_DateTime::convert_string (const char *s)
{
    const char *t = s;

    for (; *s != EOS; s++)
        if ( ! isdigit(*s) )
            return -1;

    return atoi(t);
}


// ************************************************************************
// *
// * NAME:  extract_number      Private function
// *
// * DESCRIPTION:
// *        Extract numeric field from input date-time string
// *
// * INPUT:
// *        *iString        Current index into input_string
// *        input_string    input-string contains date-time to parse
// *        min_char        Minimun number of characters to extract
// *        max_char        Maximum number of characters to extract 
// *
// * RETURNS:
// *        -1 or integer value of extracted numeric field 
// *        *iString is updated if numeric field extracted ok
// *
// ************************************************************************
int R_DateTime::extract_number (int *iString, RWCString &input_string, 
                                int min_char, int max_char)
{
    RWCString   num_string;
    int     i, lInputString;  
    int     num_value = -1;  

    for (i = 0, lInputString = input_string.length(); 
            (i < max_char) && ((*iString+i) < lInputString); i++)
    {
        if ( isdigit (input_string[*iString+i]) )
            num_string += input_string[*iString+i];
        else
        {
            break;
        }
    }

    if (num_string.length() >= min_char)
    {
        num_value = atoi ( num_string.data() );
        *iString += i;
    }

    return num_value;
}


// ***********************************************************************
// *
// * NAME:    GetSetYear      Private Function
// *
// * DESCRIPTION:
// *      The function can be used to retrieve the current setting of
// *      year, or set the year to a new value.  The function operates
// *      as 'Get' if there is no input parameter.  The function
// *      operates as a 'Set' if there is an input parameter.
// *
// *      The year to be set can be specified as an
// *      integer or string, and it can be specified as a 
// *      2-digit year (w/o century marker) or 4-digit year.
// *      A 2-digit year is expanded to 4-digit year by RWTime which
// *      seems to use the century marker of the current year.
// *      Years 1901+ are valid; years < 1901 are not accepted.
// *
// * INPUT:
// *      f   If f is supplied, it specifies the year to set within
// *          this object.  The year can be specified as an
// * RETURNS:
// *      For 'Get', it returns the current value of year within
// *          this object as a 4-character string.
// *      For 'Set', it returns the year value after set has been
// *          performed.  Note - if the input year is invalid,
// *          NULL is returned AND the year value is unchanged.
// *
// ***********************************************************************
R_String *R_DateTime::GetSetYear(Resource *f)
{
    char      syear[5];
    R_String  *rpstrYear = (R_String *) NULL;
    int       num_value;

    if (f)                                          // Set operation
    {
        if (f->ClassName() == "Integer")            // Year input as an integer 
            num_value = ((R_Integer *) f)->intval();
        else if (f->ClassName() == "String")        // Year input as a string
            num_value = convert_string(((R_String *) f)->StrValue().data() );

        if ((num_value > 0) && (num_value <= 99) )  // 2-digit year input
            year = num_value;
        else if (num_value == 0)                    // RWTime does not handle
            year = 2000;                            // a 0 year value!
        else if (num_value > 1900)                  // 4-digit year input
            year = num_value;
        else
            num_value = -1;
     }

    if (num_value != -1)
    {
        the_time = RWTime(RWDate(day, mon, year), hour, min, sec);

        rpstrYear = (R_String *) R_String::rslType.New("strYear");

        // If the_time is invalid, RWTime returns huge numbers for year.
        // the_time may be invalid because only year is being set here.

        if ( (the_time.isValid()) && (the_time != NO_DATE) )
            sprintf(syear, "%d", RWDate(the_time).year());
        else
            sprintf(syear, "%d", year);
        rpstrYear->Set(syear);
    }

    return rpstrYear;
}


// ************************************************************************
// *
// * NAME:    GetSetMonth     Private Function
// *
// * DESCRIPTION:
// *      The function can be used to retrieve the current setting of
// *      month, or set the month to a new value.  The function operates
// *      as 'Get' if there is no input parameter.  The function
// *      operates as a 'Set' if there is an input parameter.
// *
// * INPUT:
// *      f   If f is supplied, it specifies the month to set within
// *          this object.  The month can be specified as an
// *          integer or string.  Valid month range 1 - 12.
// *
// * RETURNS:
// *      For 'Get', it returns the current value of month within
// *          this object as a 2-character string.
// *      For 'Set', it returns the month value after set has been
// *          performed.  Note - if the input month is invalid,
// *          NULL is returned AND the month value is unchanged.
// *
// ************************************************************************
R_String *R_DateTime::GetSetMonth(Resource *f)
{
    char      smon[3];
    R_String  *rpstrMonth = (R_String *) NULL;
    int       num_value;

    if (f)                                      // Set operation
    {
        if (f->ClassName() == "Integer")        // Month input as an integer    
            num_value = ((R_Integer *) f)->intval();
        else if (f->ClassName() == "String")    // Month input as a string
            num_value = convert_string (((R_String *) f)->StrValue().data() );

        if ((num_value >= 1) && (num_value <= 12)) 
            mon = num_value;
        else
            num_value = -1;
    }

    if (num_value != -1)
    {
        the_time = RWTime(RWDate(day, mon, year), hour, min, sec);

        rpstrMonth = (R_String *) R_String::rslType.New("strMonth");

        // If the_time is invalid, RWTime returns huge numbers for month.
        // the_time may be invalid because only month is being set here.

        if ( (the_time.isValid()) && (the_time != NO_DATE) )
            sprintf(smon, "%d", RWDate(the_time).month());
        else
            sprintf(smon, "%d", mon);
        rpstrMonth->Set(smon);
    }

    return rpstrMonth;
}


// ************************************************************************
// *
// * NAME:    GetSetDay       Private Function
// *
// * DESCRIPTION:
// *      The function can be used to retrieve the current setting of
// *      day, or set the day to a new value.  The function operates
// *      as 'Get' if there is no input parameter.  The function
// *      operates as a 'Set' if there is an input parameter.
// *
// * INPUT:
// *      f   If f is supplied, it specifies the day to set within
// *          this object.  The day can be specified as an
// *          integer or string.  Valid day range 1 - 31.
// *
// * RETURNS:
// *      For 'Get', it returns the current value of day within
// *          this object as a 2-character string.
// *      For 'Set', it returns the day value after set has been
// *          performed.  Note - if the input day is invalid,
// *          NULL is returned AND the day value is unchanged.
// *
// ************************************************************************
R_String *R_DateTime::GetSetDay(Resource *f)
{
    char      sday[3];
    R_String  *rpstrDay = (R_String *) NULL;
    int       num_value;

    if (f)                                      // Set operation
    {
        if (f->ClassName() == "Integer")        // Day input as an integer  
            num_value = ((R_Integer *) f)->intval();
        else if (f->ClassName() == "String")    // Day input as a string
            num_value = convert_string(((R_String *) f)->StrValue().data() );

        if ( (num_value >= 1) && (num_value <= 31) ) 
            day = num_value;
        else
            num_value = -1;
    }

    if (num_value != -1)
    {
        the_time = RWTime(RWDate(day, mon, year), hour, min, sec);

        rpstrDay = (R_String *) R_String::rslType.New("strDay");

        // If the_time is invalid, RWTime returns huge numbers for day.
        // the_time may be invalid because only day is being set here.

        if ( (the_time.isValid()) && (the_time != NO_DATE) )
            sprintf(sday, "%d", RWDate(the_time).dayOfMonth());
        else
            sprintf(sday, "%d", day);
        rpstrDay->Set(sday);
    }

    return rpstrDay;
}


// ***********************************************************************
// *
// * NAME:    Compare         Private function
// *
// * DESCRIPTION:
// *      Compares the R_DateTime Resource (r) to 'this'.  
// *
// * INPUT:
// *      r   R_DateTime Resource to compare
// *      how Specified type of comparison.  Supported types are:
// *          >, <, ==, <=, >=. 
// *
// * RETURNS:
// *      NULL pointer if comparison cannot be performed (either 'this'
// *          or 'r' do not contain a valid DateTime).
// *      TRUE/FALSE result from comparison
// *
// ***********************************************************************
R_Boolean *R_DateTime::Compare(Resource *r, RWCString how)
{
    RWTime      compare_time;
    int         compare;
    R_Boolean   *rpbCompares;

    rpbCompares = (R_Boolean *) R_Boolean::rslType.New("bCompares");
    *rpbCompares = FALSE;

    if (r  &&  r->HierarchyContains(TypeID()) )
    {                                   // r is a DateTime resource
        if ( the_time.isValid() )
        {                               // this resource has a time to compare
            compare_time = ((R_DateTime *) r)->get_time() ;
            if ( compare_time.isValid() )
            {                           // r has a time to compare too
                if (how == "<") 
                {
                    if ( the_time < compare_time )
                        *rpbCompares = TRUE;
                }

                else if (how == ">")
                {
                    if (the_time > compare_time)
                        *rpbCompares = TRUE;
                }
                else if (how == "==")
                {
                    if (the_time == compare_time)
                        *rpbCompares = TRUE;
                }
                else if (how == "!=")
                {
                    if (the_time != compare_time)
                        *rpbCompares = TRUE;
                }
                else if (how == "<=")
                {
                    if (the_time <= compare_time)
                        *rpbCompares = TRUE;
                }
                else if (how == ">=")
                {
                    if (the_time >= compare_time)
                        *rpbCompares = TRUE;
                }

                else if (!rpbCompares)
                    cout << ClassName() << ": comparison method \"" << how << "\" unknown.\n";

            }
            else
                cout << ClassName() << ": Comparison date-time is invalid.  Can't compare.\n";
        }
        else
            cout << ClassName() << ": this date-time is invalid.  Can't compare.\n";
    }
    else
        cout << ClassName() << ": Need a DateTime resource to compare!\n";

    return(rpbCompares);
}


// ***********************************************************************
// *
// * NAME:    TimeNow         Private Function
// *
// * DESCRIPTION:
// *      The function can be used to set the DateTime resource to
// *      the current time.  The RogueWave object constructors initialize
// *      the objects to the current time.
// *
// * INPUT:   
// *      None
// *
// * RETURNS:
// *      Formatted current time.
// *
// ************************************************************************
R_String *R_DateTime::TimeNow()
{
    RWDate  d;
    RWTime  t;

    sec = t.second();
    min = t.minute();
    hour = t.hour();
    mon = d.month();
    day = d.dayOfMonth();
    year = d.year();

    the_time = t;

    R_String *rpstrTimeNow = (R_String *) R_String::rslType.New("strTimeNow");
    *rpstrTimeNow = format();
    return(rpstrTimeNow);
}

// ***********************************************************************
// *
// * NAME:    isNull                          Private Function
// *
// * DESCRIPTION:
// *      The function can be used to determine if the DateTime has been
// *      set, or is null.
// *
// * INPUT:   
// *      None
// *
// * RETURNS:
// *      True / False
// *
// ************************************************************************
R_Boolean *R_DateTime::isNull(void) 
{ 
	R_DateTime temp_rdt;	// constructor sets it "null" by default
	
	return R_Boolean::New("tempbIsNull", (temp_rdt == (*this)));
}

// ******************************************************************
// *                    PUBLIC OPERATOR                             *
// ******************************************************************

R_DateTime & R_DateTime::operator= (R_DateTime &that)
{
    sec   = that.sec;
    min   = that.min;
    hour  = that.hour;
    day  =  that.day;
    mon   = that.mon;
    year  = that.year;

    the_time = that.the_time;

    input_format  = that.input_format;
    output_format = that.output_format;
    formatted_date_time = that.formatted_date_time;

    return *this;
}

int R_DateTime::operator== (R_DateTime &that)
{
    return ( the_time == that.get_time() );
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
ResStatus R_DateTime::execute(int method, ResList& arglist)
{
    switch(method)
    {
        case _hDAY: 				// "Day"
            return (ResStatus(ResStatus::rslOk, 
                                GetSetDay(arglist.get(0))));
            break;
        case _hMONTH:   			// "Month"
            return (ResStatus(ResStatus::rslOk, 
                                GetSetMonth(arglist.get(0))));
            break;
        case _hYEAR:    			// "Year"
            return (ResStatus(ResStatus::rslOk, 
                                GetSetYear(arglist.get(0))));
            break;
        case _hISNULL:  			// "IsNull"
            return (ResStatus(ResStatus::rslOk, 
                                isNull()));
            break;
        case _hOpLT:    			// "<"
            return (ResStatus(ResStatus::rslOk, 
                                Compare(arglist.get(0), "<")));
            break;
        case _hOpGT:    			// ">"
            return (ResStatus(ResStatus::rslOk, 
                                Compare(arglist.get(0), ">")));
            break;
        case _hOpEQ2:   			// "=="
            return (ResStatus(ResStatus::rslOk, 
                                Compare(arglist.get(0), "==")));
            break;
        case _hOpNE:    			// "!="
            return (ResStatus(ResStatus::rslOk, 
                                Compare(arglist.get(0), "!=")));
            break;
        case _hOpLE:    			// "<="
            return (ResStatus(ResStatus::rslOk, 
                                Compare(arglist.get(0), "<=")));
            break;
        case _hOpGE:    			// ">="
            return (ResStatus(ResStatus::rslOk, 
                                Compare(arglist.get(0), ">=")));
            break;
        case _hTIME:    			// "Time"
            return (ResStatus(ResStatus::rslOk, 
                                TimeNow( )));
            break;
        case _hOpEQ:    			// "="
        case _hASSIGN:  			// "Assign"
            Assign(arglist);
            return (ResStatus(ResStatus::rslOk));
            break;
        case _hFORMAT:  			// "Format"
            return (ResStatus(ResStatus::rslOk, 
                                Format(arglist.get(0))));
            break;
        case _hINPUT_FORMAT:    	// "Input_Format"
            return (ResStatus(ResStatus::rslOk, 
                                GetSet(input_format, arglist.get(0))));
            break;
        case _hOUTPUT_FORMAT:   	// "Output_Format"
            return (ResStatus(ResStatus::rslOk, 
                                GetSet(output_format, arglist.get(0))));
            break;
        case _hPRINT:   			// "Print"
            Print( );
            return (ResStatus(ResStatus::rslOk));
            break;
        default:
            break;
    }

    ResStatus stat;
    stat.status = ResStatus::rslFail;
    return stat;
}

// ***********************************************************************
// *
// * NAME:    SetFromInline       Public Function
// *
// * DESCRIPTION:
// *      Sets the DateTime resource data members using an
// *      input data stream. This function is the opposite of
// *      PrintStream(). Any changes in PrintStream() 
// *      MAY ALSO REQUIRE a change in this functions also.
// *
// *      Resource data members are identified by name in the
// *      input data stream.  The hash code for the name (which
// *      matches a RSL method name) is used for faster matching 
// *      instead of the string name.
// *
// * INPUT: 
// *      List of streamed resources.
// *                                                                  
// *  RETURNS: 
// *      None
// *                                                                  
// ************************************************************************

void R_DateTime::SetFromInline(RWTPtrSlist<Resource>& inliner)
{
    RWTPtrSlistIterator<Resource> iter(inliner);
    Resource *r=NULL;

    while ( iter() )
    {
        r = iter.key();

        switch ( Resource::theIDHash(r->Name().data()) )
        {
            case _hFORMAT:   { UnStreamString(formatted_date_time, r); break; }
            default: 
/*
                cout << "R_DateTime::SetFromInline() Error.  Name = "
                     << r->Name() << ", HashCode = "
                     << Resource::theIDHash(r->Name().data())
                     << "unrecognized.  Ignored.\n";
*/
                break;
        }
    }
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
// *      Create a new R_DateTime                
// *                                                                       
// * INPUT:                                                                
// *      RWCString containing the object name.
// *                                                                       
// * RETURNS:                                                              
// *      Returns a R_DateTime
// *                                                                       
// ************************************************************************
R_DateTime *R_DateTime::New(RWCString n) 
{
    if (!R_DateTime::rslType)
        (void) Create_DateTime_RC();

    if (!R_DateTime::rslType)
        return NULL;

    Resource *r= R_DateTime::rslType->New(n);
    //((R_DateTime *) r)->Set( /* values (add this function if needed) */ );
    return (R_DateTime *) r;
}

// ****************************************************************************
// *
// * NAME:    log_time     - Public function
// *
// * DESCRIPTION:
// *      Prints the time structure.  Used for debugging purposes only.
// *
// * INPUT:
// *      None 
// *
// * RETURNS:
// *      None
// *
// ****************************************************************************
void R_DateTime::log_time(void)     // For debugging purposes only
{
    cout << "the_time: YYYY/MM/DD HH:MM:SS " 
         << the_time.asString("%Y/%m/%d %H:%M:%S") << "\n";
    
    cout << "the_time misc: wday= " << RWDate(the_time).weekDay()
         << ", yday= " << RWDate(the_time).julian()
         << ", isdst= " << the_time.isDST() << "\n";
}

// ****************************************************************************
// *
// * NAME:    Assign     - Public function
// *
// * DESCRIPTION:
// *        Assign a new date_time.  The date_time is input as a string.
// *        The date_time string is parsed using the 
// *        previously set input_format specification. 
// *        Or, if a new input_format is provided, use it.
// *
// *        Example:    DateTime d;
// *                    d.assign("12/30/90"); 
// *                    d = "12/30/90";
// *                    d.assign("12/13/78 08:30", "%m/%d/%y %H:%M");
// *
// * INPUT:
// *      ResList &arglist - arglist contains (1) the new date to be assigned.
// *            (2) Optional.  The input_format to be used to parse new date.
// *
// * RETURNS:
// *      None
// *
// ****************************************************************************
void R_DateTime::Assign(ResList &arglist)
{
    Resource *new_date, *format =(Resource *) NULL;
    RWCString strDate;

    if (!arglist.isEmpty())
    {
        // ****************************************
        // * 1st arg = new date to assign         *
        // ****************************************
        new_date = arglist.get(0);
        if (new_date)
            strDate = new_date->StrValue();                                    

        // **************************************
        // * 2nd arg = format to parse new date *
        // **************************************
        format = arglist.get(1);
        if (strDate.length() > 0)
            DoAssign(new_date, format);
    }
}
                                                                               
// *****************************************************************
// *                                                                
// *  NAME:    Assign          Public Function                
// *                                                                
// *  DESCRIPTION:                                                  
// *        Assign a like resource to this.
// *
// *  INPUT:                                                        
// *        Resource r - Resource to be assigned.
// *                                                                
// *  RETURNS:                                                      
// *        Returns this if Successful
// *        else NULL
// *                                                                
// *****************************************************************
void R_DateTime::Assign(Resource *r)
{
    if (r == (Resource *) NULL)
        return;

    if ( ! r->HierarchyContains(TypeID()) )
        return;

    // Use the = operator function to do the assignment.
    operator=(*(R_DateTime *)r);
}


// ****************************************************************************
// *
// * NAME:    assign     - Public function
// *
// * DESCRIPTION:
// *        Assign a new date_time.  The date_time is input as a string.
// *        The date_time string is parsed using the 
// *        previously set input_format specification.
// *        Or, if a new input_format is provided, use it instead.
// *
// *        Example:    DateTime d;
// *                    d.assign("12/30/90"); 
// *                    d = "12/30/90";
// *                    d.assign("12/13/78 08:30", "%m/%d/%y %H:%M");
// *
// * INPUT:
// *      RWCString date_time - new date-time
// *      RWCString format_spec - new input_format to use to parse date_time
// *      
// *
// * RETURNS:
// *      None
// *
// ****************************************************************************
                                                                              
void R_DateTime:: assign(RWCString date_time, RWCString format_spec)
{
    int     lInputFormat;       // Length of input_format string
    int     lInputString;       // Length of date string to be assigned
    int     iFormat;            // Index into input_format string
    int     iString;            // Index into input_string date
    int     status;
    int     num_value = 0;

//    init_the_time();            // Reset the_time data member

    if (format_spec != "")
        input_format = format_spec;

#ifdef DEBUG
    cout << "R_DateTime::assign() with input_string '"
         << date_time.data() << "', input_format '"
         << input_format.data() << "\n";
#endif

    if ( (lInputFormat = input_format.length()) <= 0) 
        return;                 // Don't know how to parse date_time

    if ( (lInputString = date_time.length()) <= 0)
        return;                 // No date_time to parse

    for (iFormat = 0, iString = 0, status = SUCCEED;                           
             (iFormat < lInputFormat) && 
             (iString < lInputString) &&
             (status == SUCCEED); 
            iFormat++)
    {

        // ************************************************************
        // * Check next char in format string.  If '%', then format   *
        // * specifier.  Else, treat it as a separator.               *
        // * Ex: input_format = %m/%d/%y, then the '/' is a separator *
        // * recognized in the input_string & skipped.                *
        // ************************************************************
        if (input_format[iFormat] != '%')
        {
            //cout << "Separator found '" << date_time[iString] << "' ";
            if (date_time[iString] != input_format[iFormat])
            {           
                // Did not match separator specified in input_format.
                // Check if it one of the standard separators.
                if ( !((date_time[iString] == '.') ||
                       (date_time[iString] == '-') ||
                       (date_time[iString] == '/')))
                    status = FAIL;
            }
            iString++;
            continue;
        }

        switch ( input_format[++iFormat] )
        {
#ifdef TO_BE_ADDED_LATER
            case 'a':           // Abbreviated weekday name
                if((num_value = extract_char_string(iString, ShortDaysList)) >= 0)
                _wday = num_value;
                else
                   status = FAIL;
                break;

            case 'A':           // Full weekday name
                if((num_value = extract_char_string(iString, FullDaysList)) >= 0)
                    _wday = num_value;
                else
                   status = FAIL;
                break;
            
            case 'b':           // Abbreviated month name
                if((num_value = extract_char_string(iString, ShortMonthList)) >= 0)
                    _mon = num_value;
                else
                   status = FAIL;
                break;

            case 'B':           // Full month name
                if((num_value = extract_char_string(iString, FullMonthList)) >= 0)
                    _mon = num_value;
                else
                   status = FAIL;
                break;
#endif

            case 'm':           // Month, as a number
                if ((num_value = extract_number(&iString, date_time, 1, 2)) >= 0)
                    mon = num_value;
                else
                    status = FAIL;
//cout << "Month extracted " << num_value << " assigned " << mon;
                break;
    
            case 'd':           // Day of the month
                if ((num_value = extract_number(&iString, date_time, 1, 2)) >= 0)
                    day = num_value;
                else
                    status = FAIL;
//cout << "Day extracted " << num_value << " assigned " << day;
                break;

            case 'y':           // (2-digit) Year within century, or 4 digits
                if ((num_value = extract_number(&iString, date_time, 2, 4)) >= 0)
                {
                    year = num_value;
                    if (year == 0)      // RWTime does not handle 0 year value.
                        year = 2000;    // (Try RWTime(RWDate(13, 12, 0))
                }
                else
                    status = FAIL;
//cout << "2digit year  extracted " << num_value << " assigned " << year;
                break;
    
            case 'Y':           // (4-digit) Year with century
                if ((num_value = extract_number(&iString, date_time, 4, 4)) >= 0)
                    year = num_value;
                else
                    status = FAIL;
//cout << "4digit year  extracted " << num_value << " assigned " << year;
                break;
    
            case 'M':           // Minute
                if ((num_value = extract_number(&iString, date_time, 2, 2)) >= 0)
                    min = num_value; 
                else
                    status = FAIL;
//cout << "Min extracted " << num_value << " assigned " << min;
                break;
    
            case 'H':           // Hour, military style
                if ((num_value = extract_number(&iString, date_time, 2, 2)) >= 0)
                    hour = num_value;
                else
                    status = FAIL;
//cout << "Hour extracted " << num_value << " assigned " << hour;
                break;
    
            case 'S':           // Seconds
                if ((num_value = extract_number(&iString, date_time, 2, 2)) >= 0)
                    sec = num_value;
                else
                    status = FAIL;
//cout << "Sec extracted " << num_value << " assigned " << sec << "\n";
                break;
    
            default:
                status = FAIL;
                break;
        }
    }

    if (status != FAIL)
        the_time = RWTime(RWDate(day, mon, year), hour, min, sec);
//        the_time = RWTime(RWDate(day, mon, year));

#ifdef DEBUG
    log_time();             
#endif
}


// ****************************************************************************
// *
// * NAME:    format     - Public function
// *
// * DESCRIPTION:
// *        Format() relies on RWTime.asString() (which relies on UNIX library 
// *        function strftime()) to perform the actual conversion from the 
// *        internal representation of date-time to the formatted string.  
// *        See strftime() for FormatSpecification details.  
// *        FormatSpecification parameter is optional.  
// *        If not supplied, the last supplied FormatSpecification is used.  
// *        Or, the default FormatSpecification %m%d%y is used.
// *
// * INPUT:
// *      format_spec - format specification for formatting date_time output
// *
// * RETURNS:
// *        Returns the formatted date-time string.
// *
// ****************************************************************************
RWCString R_DateTime::format(RWCString format_spec)
{
    char    s[50];

    formatted_date_time = "";

    if (format_spec != "")
        output_format = format_spec;

    if ( (the_time.isValid()) && (output_format.length() > 0) )
    {
        formatted_date_time = the_time.asString(output_format.data());
    }

//cout << "format(): " << s << "\n";
    return formatted_date_time;
}

// *****************************************************************
// *                                                                
// *  NAME:    PrintStream          Public Function                
// *                                                                
// *  DESCRIPTION:                                                  
// *        Outputs DateTime info as a formatted stream of data.
// *        The ResStructure::print() (in Resource.cc) defines how
// *        resource data members should be output as a stream of data.
// *        See the ResStructure::print() for the syntax rules.
// *        (PrintStream function added to work with split harness)
// *
// *        This function is the opposite of SetFromInline().
// *        Any changes in PrintStream()  MAY ALSO REQUIRE
// *        a change in SetFromInline().
// *                                                                
// *  INPUT:                                                        
// *        output stream
// *                                                                
// *  RETURNS:                                                      
// *        None
// *                                                                
// *****************************************************************

void R_DateTime::PrintStream(ostream &out)
{
    StreamHeader(out, R_DateTime_ID);

    // NOTE: THE NAME OF THE RESOURCES STREAMED IS DIRECTLY
    //       TIED TO THE RSL METHOD NAME (see the hash method names 
    //       at the top of this file).

    out << "Format: ";  StreamString(formatted_date_time, out);

    // At this time the only DateTime data that needs to be streamed is
    // the formatted string.  If others are needed, they can be
    // added as shown below.
    //out << ", OutputFormat: ";     StreamString(output_format, out);
    //out << ", InputFormat: ";      StreamString(input_format, out);
    //out << ", Year: " << year;

    StreamTrailer(out, R_DateTime_ID);
}
