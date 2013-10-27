// ***************************************************************************
// *
// *  NAME:  R_NPServer.cc
// *
// *  RESOURCE NAME:    NPServer                                        
// *                                                                    
// *  RELATED RESOURCES: 
// *                                                                    
// *  DESCRIPTION:                                                      
// *     Granite Core Server Resource
// *                                                                    
// * $Id: R_NPServer.cc,v 1.2 1998/11/23 20:01:03 cking Exp $
// *
// * $Log: R_NPServer.cc,v $
// * Revision 1.2  1998/11/23 20:01:03  cking
// * Additional modifications to initial implementation of encryption support for GF.
// *
// * Revision 1.1  1998/11/17 23:08:37  toddm
// * Initial revision
// *
// * Revision 2.4  1998/05/11 22:30:15  holtrf
// * making non blocking read on the fifo
// *
// * Revision 2.3  1998/05/01 14:04:35  toddm
// * Add NewConnection to NPServer
// *
// *
// ***************************************************************************

// *******************
// * System Includes *
// *******************
#include <fstream.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "sockinet.h"

// ******************
// * Local Includes *
// ******************
#include "R_NPServer.h"
#include "R_Integer.h"

static char rcsid[] = "$Id: R_NPServer.cc,v 1.2 1998/11/23 20:01:03 cking Exp $";

#define _hINTERNAL_STARTSERVER 1683052415   // Internal_StartServer


extern int encryption;

/**
 * R_NPServer static members.
 */
rc_NPServer R_NPServer::rslType("NPServer");

extern "C" res_class *Create_NPServer_RC()
{
    return &(R_NPServer::rslType);
}


// ************************************************************************
// ************************************************************************
// *                       RES_CLASS - rc_NPServer                        *
// ************************************************************************
// ************************************************************************

// ************************************************************************
// *
// * NAME:  spawn - Private function
// *
// * DESCRIPTION:
// *        Create a new resource of this type (R_NPServer).
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
Resource* rc_NPServer::spawn(RWCString nm)
{
    return new R_NPServer(nm);  // or other constructor
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
void R_NPServer::SetRSLConfig(  )
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


    // ***********************************************************
    // * Enable/disable encryption support for NP Server process *
    // ***********************************************************
    encryption = FALSE;
    ResReference refUseCryptoGraphy = GetDataMember("UseCryptoGraphy");
    if (refUseCryptoGraphy.isValid()) 
    {
        if (R_Integer::Int(refUseCryptoGraphy()))
            encryption = TRUE;
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
void R_NPServer::SwitchToFileLog()
{
    RWCString logname;

    ResReference resLogName = GetDataMember("LogFileName");
    if (resLogName.isValid() && resLogName.StrValue().length() > 0) 
    {
        logname = resLogName.StrValue();
    }
    else
    {
        logf->alert(LOGSERVER) << "(R_NPServer) LogFileName not specified." << endline;
        return;
    }

    logname += RWCString(".") + dec(getpid(), 0);

	cout << "Switching to logfile `" << logname << "'\n";

    // ********************************************************
    // * Record the new logfile name (probably to the syslog) *
    // ********************************************************
    logf->notice(LOGSERVER) << "(R_NPServer) Using new logfile: '" 
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
// * NAME:    ListenLoop         Protected Function
// *
// * DESCRIPTION:
// *        Opens a named pipe for reading, reads ECI requests and handles them.
// *        If the pipe closes, check if a shutdown request has been sent.  If
// *        so quit, otherwise try opening the pipe again.
// *
// * INPUT: 
// *        fifo - the name of the pipe
// *                                                                  
// *  RETURNS: 
// *      Nothing
// *                                                                  
// ************************************************************************
void R_NPServer::ListenLoop (RWCString fifo)
{
    int shutdown = 0;

    while (!shutdown)
    {
        // *************************************************
        // * Open stream non-blocking for timeout purposes *
        // *************************************************
		int fifo_fd = open(fifo.data(), O_RDONLY | O_NDELAY);
		
        if (fifo_fd < 0)
        {
            logf->error(LOGSERVER)
				<< "R_NPServer::ListenLoop: Error opening fifo `"
				<< fifo << "'" << endline;
            break;
        }
        else
        {
            cout << "NP Server: Connected to fifo " << fifo
                 << endl << flush;

			sockbuf sb(fifo_fd);
			isockstream ioss(sb);

            NewConnection(ioss);
        }

        cout << "NP Server: Close client socket.\n" << flush;

        ResReference resShutdown = GetDataMember ("Shutdown");

        if (resShutdown.isValid())
            shutdown = R_Integer::Int(resShutdown());
    }
}

void R_NPServer::setupParserContext(istream &in)
{
    // ****************************
    // * Set current lexer state **
    // ****************************
    lexc.SetSource(lexer_context::Stream, "<network>");
    lexc.SetLexerIO(in, cerr);
    lexc.done = lexer_context::Ready;
    lexc.type = lexer_context::interactive;
    lexc.keystate = lexer_context::Everywhere;
}

// ***********************************************************************
// *
// * NAME:    NewConnection         Public Function
// *
// * DESCRIPTION:
// *        This method is used to process requests read from the specified
// *        socket stream.
// *
// * INPUT: in - isockstream used to read from a socket 
// *                                                                  
// *  RETURNS: 
// *      Nothing
// *                                                                  
// ************************************************************************
void R_NPServer::NewConnection(isockstream& in)
{
    logf->debug(LOGSERVER) << "(R_NPServer) In NewConnection" << endline;

	setupParserContext(in);

    SetRSLConfig();

    quitWhenZeroSessions = FALSE;

    // ******************************************************
    // * Initialize some variables for the timeout manager. *
    // ******************************************************
    time_t secsRemaining = 0, secsToUse = 0;

    // *****************************************
    // **   Primary Loop for the R_Server.  **
    // *****************************************
    logf->notice(LOGSERVER) << "(R_NPServer) Entering primary child Granite Core Server loop." << endline;
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
            if ( (lexc.is_request_pending ()) ||
                 (in->is_readready(secsToUse, 0)) )
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
                logf->notice(LOGSERVER) << "(R_NPServer) Ready to send timeout events." << endline;
                AuditRequest *toutgoing = timeoutManager->tm.SendTimeouts();

                // **********************************
                // * toutgoing is probably an elist *
                // **********************************
                if (toutgoing)
                {
                    logf->info(LOGSERVER) << "(R_NPServer) Events received from timeout manager: "
                                          << toutgoing << endline;

                    logf->info(LOGSERVER) << "(R_NPServer) outbound tm events for session "
                                          << toutgoing->object_id << endline;

                    // *********************************
                    // * Process timeout event result! *
                    // *********************************
//                    Outbound(toutgoing->arguments); // rslServer::Outbound()                    
                }
                else
                {
                    logf->info(LOGSERVER) << "(R_NPServer) No events returned from timeout manager." 
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
                
            if ( (lexc.is_request_pending ()) ||
                 (in->is_readready(secsToUse, 0)) )
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
// * NAME:    R_NPServer         Constructor
// *
// * DESCRIPTION:
// *      Constructor for the the R_NPServer class
// *
// * INPUT: 
// *                                                                  
// *  RETURNS: 
// *                                                                  
// ************************************************************************
R_NPServer::R_NPServer(RWCString nm)
    : R_Server(nm, (res_class *) &R_NPServer::rslType)
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
ResStatus R_NPServer::execute(int method, ResList& arglist)
{
    switch(method)
    {
        case _hINTERNAL_STARTSERVER:    // "Internal_StartServer"
            return rsl_Internal_StartServer(arglist);

        default: 
                return rslServer::execute(method, arglist);
    }

    return ResStatus(ResStatus::rslFail);
}


// RSL method "Internal_StartServer"
ResStatus R_NPServer::rsl_Internal_StartServer(const ResList& arglist)
{
    ResReference resFifo = GetDataMember ("Fifo");
    if (resFifo.isValid ())
        ListenLoop(resFifo.StrValue());
    else
        ListenLoop("DEFAULT_FIFO");
    return ResStatus(ResStatus::rslOk, NULL);
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
// *      Returns a R_NPServer
// *                                                                       
// ************************************************************************
R_NPServer *R_NPServer::New(RWCString n /*, some value(s) */)
{
    Resource *r= R_NPServer::rslType.New(n);
    return (R_NPServer *) r;
}
