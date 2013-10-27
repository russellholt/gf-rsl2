// ***************************************************************************
// *
// *  NAME:  R_GCServer.cc
// *
// *  RESOURCE NAME:    GCServer                                        
// *                                                                    
// *  RELATED RESOURCES: 
// *                                                                    
// *  DESCRIPTION:                                                      
// *     Granite Core Server Resource
// *                                                                    
// * $Id: R_GCServer.cc,v 1.1 1998/11/17 23:06:09 toddm Exp $
// *
// * $Log: R_GCServer.cc,v $
// * Revision 1.1  1998/11/17 23:06:09  toddm
// * Initial revision
// *
// * Revision 2.2  1998/05/01 14:03:10  toddm
// * Check Implement TotalSessionAllowed
// *
// * Revision 2.1  1998/03/27 21:19:18  toddm
// * Check out to update version number to 2.x
// *
// * Revision 1.1  1998/03/27 19:02:04  toddm
// * Initial revision
// *
// * Copyright (c) 1998 by Destiny Software Corporation
// *
// *******************************************************************

// *******************
// * System Includes *
// *******************
#include <fstream.h>
#include <sys/types.h>
#include <stdlib.h>

// ******************
// * Local Includes *
// ******************
#include "R_GCServer.h"
#include "R_Integer.h"


// R_GCServer static member
rc_GCServer *R_GCServer::rslType = NULL;

extern "C" res_class *Create_GCServer_RC()
{
	return R_GCServer::rslType = new rc_GCServer("GCServer");
}


// ************************************************************************
// ************************************************************************
// *                       RES_CLASS - rc_GCServer                        *
// ************************************************************************
// ************************************************************************

// ************************************************************************
// *
// * NAME:  spawn - Private function
// *
// * DESCRIPTION:
// *        Create a new resource of this type (R_GCServer).
// *        Called by res_class::New() if there is no object
// *        To pull off the free list.
// *
// * INPUT:
// *        None 
// *
// * RETURNS:
// *        None
// *
// ************************************************************************
Resource* rc_GCServer::spawn(RWCString nm)
{
	return new R_GCServer(nm);	// or other constructor
}

// ******************************************************************
// *                    PRIVATE MEMBER FUNCTIONS                    *
// ******************************************************************

// *********************************************************************
// *                                                                    
// * Function: SetRSLConfig
// *                                                                    
// * Description:   This method is used to set internal server data 
// *                from or take action based on parameters set from 
// *                within the RSL Init() method.
// *                                                  
// * Inputs: None                  
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: Void
// *                                                                    
// *********************************************************************
void R_GCServer::SetRSLConfig(  )
{
    // *********************************************************
    // * Open a file log for this specific process if desired! *
    // *********************************************************
    ResReference resUseFileLog = GetDataMember("UseFileLog");
    if (resUseFileLog.isValid()) 
    {
        if (R_Integer::Int(resUseFileLog()))
            SwitchToFileLog();
    }
    else
    {
        // *************************
        // * Set the log level.... *
        // *************************
    	ResReference resLogLevel = GetDataMember("LogLevel");
        if (resLogLevel.isValid() && resLogLevel.StrValue().length() > 0) 
        {
        	Logf.SetLevel(R_Integer::Int(resLogLevel()));
        }
    	
        // ******************************
        // * Set logging subsystems.... *
        // ******************************
    	ResReference resWhichSystems = GetDataMember("LogWhichSystems");
        if (resWhichSystems.isValid() && resWhichSystems.StrValue().length() > 0) 
        {
        	Logf.SetSubSysMask(resWhichSystems.StrValue());
        }
    }
}


// ************************************************************************
// *                                                                       
// * NAME:    SwitchToFileLog             Public Function                           
// *                                                                       
// * DESCRIPTION:                                                          
// *        Switch log file to the specifed log file name.
// *                                                                       
// * INPUT:                                                                
// *        None
// *                                                                       
// * RETURNS:                                                              
// *        None
// *                                                                       
// ************************************************************************
void R_GCServer::SwitchToFileLog()
{
    RWCString logname;

    ResReference resLogName = GetDataMember("LogFileName");
    if (resLogName.isValid() && resLogName.StrValue().length() > 0) 
    {
        logname = resLogName.StrValue();
    }
    else
    {
        logf->alert(LOGSERVER) << "(R_GCServer) LogFileName not specified." << endline;
        return;
    }

    logname += RWCString(".") + dec(getpid(), 0);

	cout << "Switching to logfile `" << logname << "'\n";

    // ********************************************************
    // * Record the new logfile name (probably to the syslog) *
    // ********************************************************
    logf->notice(LOGSERVER) << "(R_GCServer) Using new logfile: '" 
                            << logname << "'" << endline;

    // *************************
    // * Set the log level.... *
    // *************************
	ResReference resLogLevel = GetDataMember("LogLevel");
    if (resLogLevel.isValid() && resLogLevel.StrValue().length() > 0) 
    {
    	Logf.SetLevel(R_Integer::Int(resLogLevel()));
    }
	
    // ******************************
    // * Set logging subsystems.... *
    // ******************************
	ResReference resWhichSystems = GetDataMember("LogWhichSystems");
    if (resWhichSystems.isValid() && resWhichSystems.StrValue().length() > 0) 
    {
    	Logf.SetSubSysMask(resWhichSystems.StrValue());
    }

    logf->Openlog(logname.data(), _to_stdio_file_);
}


// *************************************************************************
// **                         PUBLIC MEMBER FUNCTIONS                      *
// **                                                                      *
// **         Virtual functions, overriding those in 'rslServer'           *
// *************************************************************************

// ***********************************************************************
// *
// * NAME:    NewConnection         Public Function
// *
// * DESCRIPTION:
// *        This method is used to process requests read from the specified
// *        socket stream.
// *
// * INPUT: in - iosockinet used to read from a socket 
// *                                                                  
// *  RETURNS: 
// *      Nothing
// *                                                                  
// ************************************************************************
void R_GCServer::NewConnection(iosockinet& in)
{
    logf->debug(LOGSERVER) << "(R_GCServer) In NewConnection" << endline;

    // ****************************
    // * Set current lexer state **
    // ****************************
    lexc.SetSource(lexer_context::Stream, "<network>");
    lexc.SetLexerIO(in, in);
    lexc.done = lexer_context::Ready;
    lexc.type = lexer_context::interactive;
    lexc.keystate = lexer_context::Everywhere;

    SetRSLConfig();

    quitWhenZeroSessions = FALSE;

    // ******************************************************
    // * Initialize some variables for the timeout manager. *
    // ******************************************************
    time_t secsRemaining = 0, secsToUse = 0;

    // *****************************************
    // **   Primary Loop for the R_Server.  **
    // *****************************************
    logf->notice(LOGSERVER) << "(R_GCServer) Entering primary child Granite Core Server loop." << endline;
    do
    {
        // ****************************************************************
        // * compute time to select() with timeout manager remaining time *
        // ****************************************************************
        if (timeoutManager)
        {
            secsRemaining = timeoutManager->tm.SecsToTimeout();
            secsToUse = (secsRemaining > 0) ? secsRemaining : 300;
            
            if (shutdownTime >= 0)
            {
                time_t ct = time(0);
                time_t elapsed = shutdownTime - ct;

                if (elapsed > 0 && elapsed < secsToUse)
                    secsToUse = elapsed;
                else
                if (elapsed <= 0)
                    ShutdownNow();
            }
                
            logf->info(LOGSERVER) << "(R_GCServer) Seconds until first timeout event: " << (int) secsRemaining
                                  << ". Waiting for " << (int) secsToUse << " seconds.." << endline;

            // ***********************************************************
            // * wait for input for the remaining time until the        **
            // * next timeout event, at which point is_readready() will **
            // * return zero.                                           **
            // ***********************************************************
            if (in->is_readready(secsToUse, 0))
            {
                if (RoundTripRequest() != 0)
                    return;
                else
                    // *********************************************
                    // * Check to see if the totalSessionsAllowed  *
                    // * has been exceeded.                        *
                    // *********************************************
                    if (RecycleServer())
                        ShutdownNow();                    
            }

            if (timeoutManager->timeoutReady())
            {
                logf->notice(LOGSERVER) << "(R_GCServer) Ready to send timeout events." << endline;
                AuditRequest *toutgoing = timeoutManager->tm.SendTimeouts();

                // **********************************
                // * toutgoing is probably an elist *
                // **********************************
                if (toutgoing)
                {
                    logf->info(LOGSERVER) << "(R_GCServer) Events received from timeout manager: "
                                          << toutgoing << endline;

                    logf->info(LOGSERVER) << "(R_GCServer) outbound tm events for session "
                                          << toutgoing->object_id << endline;

                    // *********************************
                    // * Process timeout event result! *
                    // *********************************
//                    Outbound(toutgoing->arguments); // rslServer::Outbound()                    
                }
                else
                {
                    logf->info(LOGSERVER) << "(R_GCServer) No events returned from timeout manager." 
                                          << endline;
                }
            }
        }
        else
        {
            // ********************************************************
            // * Figure out the seconds to wait for an incoming event *
            // ********************************************************
            secsToUse = 30;                     // use some "large" default value.
            
            // *******************************
            // * Is there a shutdown pending *
            // *******************************
            if (shutdownTime >= 0)
            {
                time_t ct = time(0);
                time_t elapsed = shutdownTime - ct;

                if (elapsed > 0 && elapsed < secsToUse)
                    secsToUse = elapsed;
                else
                if (elapsed <= 0)
                    ShutdownNow();
            }
                
            if (in->is_readready(secsToUse, 0))
            {
                if (RoundTripRequest() != 0)
                    return;
                else
                    // *********************************************
                    // * Check to see if the totalSessionsAllowed  *
                    // * has been exceeded.                        *
                    // *********************************************
                    if (RecycleServer())
                        ShutdownNow();                    
            }
        }

    } while (lexc.done == lexer_context::Ready);
}

// *************************************************************************
// **                         PUBLIC MEMBER FUNCTIONS                      *
// **                                                                      *
// **                       Constructors for this class                    *
// *************************************************************************

// ***********************************************************************
// *
// * NAME:    R_GCServer         Constructor
// *
// * DESCRIPTION:
// *      Constructor for the the R_GCServer class
// *
// * INPUT: 
// *                                                                  
// *  RETURNS: 
// *                                                                  
// ************************************************************************
R_GCServer::R_GCServer(RWCString nm)
	: R_Server(nm)
{

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
ResStatus R_GCServer::execute(int method, ResList& arglist)
{
    logf->debug(LOGSERVER) << "(R_GCServer) In execute" << endline;

	switch(method)
	{
        default:                // RSL inheritance in C++...
            return rslServer::execute(method, arglist);
	}

	return ResStatus(ResStatus::rslFail);
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
// *      Returns a R_GCServer
// *                                                                       
// ************************************************************************
R_GCServer *R_GCServer::New(RWCString n)
{
	if (!R_GCServer::rslType)
		(void) Create_GCServer_RC();

	if (!R_GCServer::rslType)
		return NULL;

	Resource *r= R_GCServer::rslType->New(n);
	return (R_GCServer *) r;
}

