// ****************************************************************************
// *
// * $Header: /dest/razor/RAZOR_UNIVERSE/DOMAIN_01/foundation25/Archive/RZ_VCS/granite/granitecore/dutil/src/R_DateTime.h,v 1.1 1998/11/17 23:48:17 toddm Exp $
// *
// * NAME:              R_DateTime.h
// *
// * RESOURCE NAME:     DateTime                                        
// *                                                                    
// * RELATED RESOURCES:   
// *                                                                    
// * DESCRIPTION:                                                      
// *                                                                    
// *     Date & Time resource based on RogueWave RWTime object.
// *     See documentation at bottom of this .h file.
// *
// * Copyright (c) 1995-1998 by Destiny Software Corporation
// *
// ****************************************************************************
// SYSTEM INCLUDES
#include <rw/cstring.h>         // Roguewave String class 
#include <rw/rwtime.h>          // Roguewave Time class
#include <rw/rwdate.h>          // Roguewave Date class

// LOCAL INCLUDES
#include "destiny.h"
#include "Resource.h"           // Resource class
#include "res_class.h"          // res_class class

#include "R_String.h"           // String resource
#include "R_Integer.h"          // Integer resource
#include "R_Boolean.h"          // Boolean resource

#ifndef _R_DATE_TIME_H_
#define _R_DATE_TIME_H_

// ******************************************
// *                CONSTANTS               *
// ******************************************

#define R_DateTime_ID 268966144

// Define a baseline for dates.  RogueWave initializes dates to current date.
// Creating a R_DateTime object and initializing it to a known NO_DATE
// value makes it easy to check if it's been initialized with real data.

#define NO_DATE	rwJul1901


// ******************************************************
// *                Class Definitions                   *
// ******************************************************

// DateTime res_class 
class rc_DateTime : public res_class {

    Resource *spawn(RWCString aname);
public:
    rc_DateTime(RWCString aname) : res_class(aname)
    {     }
};

// DateTime Resource
class R_DateTime : public ResObj {

private:
    // ************************
    // * Private Data Members *
    // *************************
    RWTime      the_time;                  // RogueWave time object

    unsigned int mon, day, year;
    unsigned int hour, min, sec;

    // ****************************
    // * Private Member Functions *
    // ****************************
    void    Init(void);                     // Initialize data members
    void    init_the_time(void);            // Initialize tm structure members

    int     extract_number (int *iString, RWCString &input_string, 
                            int min_char, int max_char);

    int     convert_string (const char *s); // If string is numeric,ret value
    void    log_time(void);                 // For debugging purposes only

    R_Boolean *isNull(void);

    R_Boolean  *Compare(Resource *r, RWCString how);
    R_String   *GetSetYear(Resource *f);      // Returned string is YYYY form
    R_String   *GetSetMonth(Resource *f);     // Returned string is MM form
    R_String   *GetSetDay(Resource *f);       // Returned string is DD form
    R_String   *TimeNow(void);                // Returns formatted time


protected:
    // ******************
    // * Protected Data *
    // ******************
    RWCString  input_format;                // To parse incoming date-time string
    RWCString  output_format;               // To convert internal representation
    RWCString  formatted_date_time;         // Formatted using output_format

    // ******************************
    // * Protected Member Functions *
    // ******************************

public:
    // ***********************
    // * Public Data Members *
    // ***********************
    static rc_DateTime *rslType;

    // ******************************
    // * Constructors & Destructors *
    // ******************************
    
    R_DateTime()          
        : ResObj("foo", (res_class *) rslType)    { Init(); }

    R_DateTime(RWCString nm)
        : ResObj(nm, (res_class *) rslType)       { Init(); }

    // Constructor to be called by classes inheriting from R_DateTime
    R_DateTime(RWCString nm, res_class *rc)
        : ResObj(nm, (res_class *) rslType)       { Init(); }

    // *************
    // * Operators *
    // *************
    R_DateTime &operator= (struct tm *new_time);
    R_DateTime &operator= (R_DateTime &that);
    int operator== (R_DateTime &that);

    // ***************************
    // * Public Member Functions *
    // ***************************
    static R_DateTime *New(RWCString n);

    RWTime     get_time(void)       { return the_time; }
    R_DateTime &get_this(void)      { return *this; }

    void       set_time (RWTime t)  { the_time = t; }
    void       set_date (RWDate d)  { the_time = RWTime(d, hour, min, sec); }

    void    assign(RWCString date_time, RWCString format_spec = "");
    void    assign(const char *date_time, RWCString format_spec = "")
    {
        RWCString  d = date_time;
        assign(d, format_spec);
    }

    void Assign(ResList &arglist);

    void DoAssign(Resource *rstrNewDate, Resource *rstrFormatSpec=(Resource *) NULL)
    {
        if (rstrNewDate == (Resource *) NULL)
            return;

        if (rstrFormatSpec == (Resource *) NULL)
            assign (((R_String *) rstrNewDate)->StrValue());
        else
            assign (((R_String *) rstrNewDate)->StrValue(),
                    ((R_String *) rstrFormatSpec)->StrValue());
    }

    RWCString  format(RWCString format_spec = "");

    R_String *Format(Resource *rstrFormatSpec = NULL)
    {
        if (rstrFormatSpec != (Resource *) NULL)
            output_format = rstrFormatSpec->StrValue();
        format();


        R_String *rstrFormatDate = (R_String *) R_String::rslType.New("strFormatDate");
        *rstrFormatDate = formatted_date_time;
        return(rstrFormatDate);
    }

    int     get_current_year(void) { return RWDate().year(); }

    int     get_year(void)   
            { return the_time.isValid() ? RWDate(the_time).year() : year; }

    int     get_month(void) 
            { return the_time.isValid() ? RWDate(the_time).month() : mon; }

    int     get_day(void)    
            { return the_time.isValid() ? RWDate(the_time).day() : day; }

    int     get_hour(void)   
            { return the_time.isValid() ? the_time.hour() : hour; }

    int     get_minute(void) 
            { return the_time.isValid() ? the_time.minute() : min; }

    int     get_second(void) 
            { return the_time.isValid() ? the_time.second() : sec; }


    // set_year MUST be called with year including century marker
    void    set_year(int y)  
            { year = y; 
              the_time = RWTime(RWDate(day, mon, year), hour, min, sec); }

    void        Print(ostream &out=cout) { cout << format() << ": "; }
    void        PrintStream(ostream &out=cout);

    // *********************
    // * Virtual functions  overriding Resource
    // *********************

    unsigned int TypeID()                   { return R_DateTime_ID; }
    res_class   *memberOf(void)             { return rslType; }     
    RWCString   StrValue(void)              { return formatted_date_time;}
    int         LogicalValue(void)          { return the_time.isValid(); }
    int         IsEqual(Resource *r) 
                { return (Compare(r, "=="))->LogicalValue(); }

    ResStatus   execute(int method, ResList& arglist);    
    void        SetFromInline(RWTPtrSlist<Resource>& inliner);
    void        Assign(Resource *r);
    void        Clear(void)                 { Init(); }

    void        print(ostream &out=cout)    { PrintStream(out); }
    void        rslprint(ostream &out=cout) { Print(out); }
};

#endif      /* _R_DATE_TIME_H_ */


/*****************************************************************************
 <<Begin Resource Documentation>>


 RESOURCE NAME:     DateTime

 RELATED RESOURCES: None

 DESCRIPTION:

    DateTime resource.   

    Internally, the date-time is maintained using RogueWave RWTime object.
    (See RWTime documentation for its limitations.)

    NOTES:
    1. Dates beginning from 1901 are accepted.

    2. The maximum date-time value is limited by RWTime which itself is
       limited by the system definition of 'time_t'.  On UNIX systems, 
       'time_t' is a long.  The maximum date-time value is then limited
       by the system definition of a'long'.  If a 'long' is 4-bytes,  the
       maximum date-value is 2/5/2037.
       RWTIME DOES NOT PREVENT ASSIGNMENT OF DATES THAT EXCEED 'long'.
       Example - the date 3/15/2037 is accepted as valid, but it is
       translated to 2/6/1901. 3/15/2040 becomes 2/7/1904.

    3. The century marker is added when the year is supplied as a 
       2-digit value (without the century).  The century marker added by RWTime
       is the century of the current year.  (So, a year of 78 becomes 1978
       now, but will be 2078 after the turn of the century.)  This rule 
       becomes a problem when the year supplied is 00.  The century prefix
       for this is 19 now.  The year becomes 1900; but RWTime only accepts
       dates from 1901.  The DateTime resource takes care of year 00 by
       prefacing it with '20' to force it to be 2000.  In general,
       dates should be specified using a 4 digit year to ensure accuracy.

 PUBLIC DATA MEMBERS:

    String Day( );
    String Day( String strDay );
    String Day( Integer iDay );

        The new_value for day can be specified as an integer
        or string.  Valid day range 1 - 31.

        A 2-digit day string is returned.
        Note - if the new_value for day is invalid, NULL is
        returned AND the day value is unchanged.

        Ex:  DateTime d;
             d.Day("5");                // Specify day as a string
             d.Day(5);                  // Specify day as an int
             out.print(d.Day());        // Prints current day value

    String Month( );
    String Month( String strMonth );
    String Month( Integer iMonth );

        The new_value for month can be specified as an integer
        or string.  Valid month range 1 - 12.

        A 2-digit month string is returned.
        Note - if the new_value for month is invalid, NULL is
        returned AND the month value is unchanged.

    String Year( );
    String Year( String strYear );
    String Year( Integer iYear );

        The new_value for year can be specified as an integer
        or string, and it can be specified as a 2-digit 
        year (without the century marker) or 4-digit year.
        See Notes above on 2 to 4 digit year mapping.

        A 4-digit year string is returned.
        Note - if the new_value for year is invalid, NULL is
        returned AND the year value is unchanged.

        Example:  d.Year(78);
                  d.Year(1978);
                  d.Year("1978");
                  d.Year("78");
                  all result in the same assignment.

    String Input_Format( );
    String Input_Format( String strFormat );

        strFormat specifies the rules for parsing an incoming DateTime
        string.  Default input_format is "%m/%d/%y".  The format
        tokens supported are:

        m   Month
        d   Day of the month
        y   2-digit year within century
        Y   4-digit year with century
        M   Minutes
        H   Hour, military style
        S   Seconds
        
        The default input_format is "%m/%d/%y".
        By default, the separator between date fields can be '/',
        '.' or '-'.  An additional separator may be specified in
        input_format, but the default set is always applicable.

        The current input_format string is returned.

    String Output_Format( );
    String Output_Format( String strFormat );
        
        strFormat specifies the rules for formatting DateTime as a string.
        Default output_format is "%m/%d/%y".
        See UNIX strftime() for supported formatting tokens.

        The current output_format string is returned.

 PUBLIC MEMBER FUNCTIONS:

    ()
        Usage of the DateTime resource name returns TRUE if the
        datetime is valid, and FALSE otherwise.  

        Example:    DateTime d;
                    d = "04/05/96";
                    if (d)
                        // processing for valid date
                    else
                        // processing for invalid date

    Boolean IsNull( );
        Returns 0 (false) if DateTime has not been specified;
        otherwise returns 1 (true).

    String Format(  );
    String Format( String strFormat );

        Format() relies on UNIX library function strftime() to 
        perform the actual conversion from the internal representation 
        of date-time to the formatted string.  See strftime() for
        format specification (strFormat) details.  strFormat parameter
        is optional.  If not supplied, the last supplied strFormat
        Or, the default format specification %m%d%y is used.

        Returns the formatted date-time string.

    String Time()
        Sets the DateTime to the current time.  

        Returns the formatted date-time string.


    =( DateTime rDate );
    =( String strDate );
    Assign( DateTime rDate );
    Assign( DateTime rDate, String strInputFormat );
    Assign( String strDate );
    Assign( String strDate, String strInputFormat );

        Assigns the new date-time (rDate, strDate) to DateTime, using the
        previously set input_format specification to parse the
        new date-time, or the new input format (strInputFormat). 

        Examples:   DateTime d;
                    d.assign("12/30/90"); 
                    d = "12/30/90";  
                    d.assign("12/13/1997"); 
                    d.assign("12.13.1998 08:30:00", "%m.%d.%Y %H:%M:%S");

    Comparison Operators: <, >, ==, !=, <=, >=
        
        Compares 2 DateTime resources.  If either resource does
        not contain a valid time, FALSE is returned.  Otherwise,
        comparison is performed and a boolean result is returned.
    
    Print(void )
        This is a resource debugging function that displays the
        contents of the resource in a formatted manner.
        Returns nothing.

 SEE ALSO:
        strftime(), time.h

 <<End Resource Documentation>>
 ****************************************************************************/
 






