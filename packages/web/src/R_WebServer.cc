// ***************************************************************************
// *
// *  NAME:  R_WebServer.cc
// *
// *  RESOURCE NAME:    WebServer                                        
// *                                                                    
// *  RELATED RESOURCES: 
// *                                                                    
// *  DESCRIPTION:                                                      
// *     Web Server Resource
// *                                                                    
// * $Id: R_WebServer.cc,v 1.6 1998/12/22 16:41:53 cking Exp $
// *
// * $Log: R_WebServer.cc,v $
// * Revision 1.6  1998/12/22 16:41:53  cking
// * Patch implements a configurable receive time-out for the GF Web Channel.
// *
// * Revision 1.5  1998/12/22 15:12:55  cking
// * Patch implements a configurable receive time-out for the GF Web Channel.
// *
// * Revision 1.4  1998/12/02 18:44:16  cking
// * Performance modifications for encryption support for the GF 2.5 release.
// *
// * Revision 1.3  1998/11/30 20:52:39  cking
// * Additional modifications for encryption support in GF 2.5 release
// *
// * Revision 1.2  1998/11/23 19:41:21  cking
// * Additional modifications to initial implementation of encryption support for GF.
// *
// * Revision 1.1  1998/11/17 23:30:06  toddm
// * Initial revision
// *
// * Revision 2.18  1998/11/12 21:30:12  toddm
// * Add new subsystems
// *
// * Revision 2.17  1998/11/09 20:58:18  toddm
// * Fix R_LIST processing
// *
// * Revision 2.16  1998/06/29 19:46:58  toddm
// * Add gethostname prototype
// *
// * Revision 2.15  1998/05/29 22:19:41  holtrf
// * oops, extern 'C' !
// *
// * Revision 2.14  1998/05/29 22:07:05  holtrf
// * fixed bus error for sun.
// *
// * Revision 2.13  1998/05/13 23:33:45  toddm
// * Benchmark test logging
// *
// * Revision 2.12  1998/04/27 20:07:38  prehmet
// * Fix SIGCLD handling.
// *
// * Revision 2.11  1998/04/23 20:38:18  toddm
// * Fix shutdown process
// *
// * Revision 2.10  1998/04/16 20:20:43  toddm
// * Add Request Counting
// *
// * Revision 2.9  1998/04/15 16:06:36  toddm
// * Modify rsl_Quit
// *
// * Revision 2.8  1998/04/10 16:03:36  toddm
// * Merge 2.1 changes
// *
// * Revision 2.6  1998/04/10 13:44:27  prehmet
// * Handles port range.
// *
// * Revision 2.5  1998/04/09 20:02:15  prehmet
// * Port asgnmt done in child, not parent. Random port used if not specified.
// *
// * Revision 2.4  1998/04/07 19:23:23  prehmet
// * Adding admin functionality.
// *
// * Revision 2.3  1998/04/06 19:25:48  prehmet
// * Replaced pGraniteCore delete with Disconnect.
// *
// * Revision 2.2  1998/04/06 17:28:31  prehmet
// * Integration with Registrar and Broker.
// *
// * Revision 2.1  1998/02/13 21:33:57  toddm
// * Start work on splitting the Web Channel from the Granite Core
// *
// * Revision 1.11  1998/01/15 18:48:34  toddm
// * Add Dynamic reconfiguration stuff
// *
// * Revision 1.10  1997/11/05 16:53:45  toddm
// * HTM extension support / Fix multiple cookie problem
// *
// * Revision 1.9  1997/09/25 23:49:50  toddm
// * Initial iFileLen
// *
// * Revision 1.8  1997/09/24 23:59:38  toddm
// * Fix Memory Leaks
// *
// * Revision 1.7  1997/09/19 18:33:17  toddm
// * Fix session counting
// *
// * Revision 1.6  1997/09/17 22:44:34  toddm
// * Fix Garbage Character problem when using Internet Explorer
// *
// *
// * Copyright (c) 1995, 1996, 1997, 1998 by Destiny Software Corporation
// *
// ***************************************************************************

// *******************
// * System Includes *
// *******************
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>
#include <fstream.h>
#include <stdlib.h>
#include <wait.h>

extern "C" int gethostname(char *name, int namelen);

// ******************
// * Local Includes *
// ******************
#include "R_WebServer.h"
#include "R_Integer.h"
#include "NetAddress.h"
#include "runtime.h"
#include "CryptoSubSystem.h"

// ******************************************
// *                CONSTANTS               *
// ******************************************

#define MAXHOSTNAMELEN 256        // e.g., "horizon.destinyusa.com"

void sighandlerUSR1(int);         // interrupt child
void sighandlerUSR1parent(int);   // interrupt parent
void sighandlerCLD(int);          // notify parent of killed child
void sighandlerHUP(int);
int read_dynamic_config = 0, shutdown_now=0;

// List of child process ids, used only by parent.
RWTValSlist<int> childPids;

extern int coreReceiveTimeOut;		// 1 minute receive time-out with core process(s)
int webServerReceiveTimeOut = 120;	// 2 minutes receive time-out with web server


rc_WebServer *R_WebServer::rslType = NULL;

extern "C" res_class *Create_WebServer_RC()
{
    R_WebServer::rslType = new rc_WebServer("WebServer");
    return R_WebServer::rslType;
}


// ************************************************************************
// ************************************************************************
// *                       RES_CLASS - rc_WebServer                       *
// ************************************************************************
// ************************************************************************

// ************************************************************************
// *
// * NAME:  spawn - Private function
// *
// * DESCRIPTION:
// *        Create a new resource of this type (R_WebServer).
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
Resource* rc_WebServer::spawn(RWCString nm)
{
    return new R_WebServer(nm); // or other constructor
}

// *************************************************************************
// **                      VIRTUAL MEMBER FUNCTION                         *
// **                                                                      *
// **           Cryptography SubSystem of Web Server specific              *
// *************************************************************************

void _CryptoSubSystem::logMessage (CryptoLogMsgType msgType, char *pstrMessage)
{
 
    switch (msgType)
    {
        case INFO:
            logf->notice(LOGENCRYPTION) <<  pstrMessage << endline;
            break;

        case ERR:
            logf->error(LOGENCRYPTION) <<  pstrMessage << endline;
            break;

        case DBUG:
        default:
            logf->debug(LOGENCRYPTION) <<  pstrMessage << endline;
            break;
    }

}    /* end logMessage */


// ************************************************************************
// ************************************************************************
// *                        RESOURCE - R_WebServer                        *
// ************************************************************************
// ************************************************************************

// ******************************************************************
// *                    PRIVATE MEMBER FUNCTIONS                    *
// ******************************************************************

// *********************************************************************
// *                                                                    
// * Function: UnescapeStr                                             
// *                                                                    
// * Description:   Function is used to convert all occurrences of '%' 
// *                followed by two hex digits with the ascii equivalent, 
// *                ie "%2f" -> '/'     
// *                                                  
// * Inputs: DRWCString strData -   String containing the original escaped 
// *                                string
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns:   DRWCString - The Unescaped string.
// *                                                                    
// *********************************************************************
DRWCString R_WebServer::UnescapeStr(DRWCString strData)
{
    int iEscPos, iStartIdx=0;
    RWCString strEscStr;
    RWCString strLSD, strMSD;

    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In UnescapeStr" << endline;

    RWCRegexp esc_hex("%[0-9a-fA-F][0-9a-fA-F]");   // percent followed by two hex digits
    RWCString HexDig = "0123456789abcdef";

    while ((iEscPos = strData.index(esc_hex, iStartIdx)) >= 0)
    {
        strEscStr = strData(iEscPos, 3);    // %xx
        strMSD = toLower(strEscStr[1]);     // most significant digit
        strLSD = toLower(strEscStr[2]);     // least significant digit
        if ((strMSD.length() > 0) && 
            (strLSD.length() > 0))          // Do the conversion
        {
            strData(iEscPos,3) = (char) (HexDig.index(strMSD) * 16 + 
                                         HexDig.index(strLSD));
        }

        iStartIdx = iEscPos;
    }

    return strData;
}


// *********************************************************************
// *                                                                    
// * Function: ParseCookies
// *                                                                    
// * Description:   This function parses the cookies sent to the server
// *                and splits them into name/values pairs.  These pairs are
// *                added to the a Single linked list of name/values
// *                pairs in the request class
// *                                                           
// *                The format is name=value pairs separated by ,  
// *                                                  
// * Inputs: sCookies - String containing a list of cookies 
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: The number of name/values pairs acutually created.
// *                                                                    
// *********************************************************************
int R_WebServer::ParseCookies(RWCString strCookies)
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In ParseCookies" << endline;

    // *******************************
    // * Is there a query string     *
    // *******************************
    if (strCookies.length() == 0) 
        return(0);

    int iNumCookies=0;
    StringPair pair;

    RWCTokenizer next(strCookies);
    DRWCString token;                // Will receive each token
    DRWCString strTmp;

    // ******************************************************
    // * Get each name/value pair from the cookie string    *
    // ******************************************************
    while (!(token=next(";")).isNull())
    {
        if (token.contains("="))
        {
            strTmp = UnescapeStr(token.before("="));
            strTmp = strTmp.strip(RWCString::both);
            pair.SetLeft(strTmp);

            strTmp = UnescapeStr(token.after("="));
            strTmp = strTmp.strip(RWCString::both);
            pair.SetRight(strTmp);

            logf->debug(LOGWEBCHANNEL) << "(R_WebServer) Name: " << pair.left() 
                                    << " / Value: " << pair.right() << endline;        
            
            IncomingEvents.append(pair);
            iNumCookies++;
        }
    }

    return(iNumCookies);
}


// *********************************************************************
// *                                                                    
// * Function: ParseRequestHeaders
// *                                                                    
// * Description:   This function parses the strHeader and splits the
// *                header into name/values pairs.  These pairs are
// *                added to the a Single linked list of name/values
// *                pairs.
// *                                                           
// *                The format is name=value pairs separated by &  
// *                with spaces turned into '+'.  '%' indicates that the 
// *                next 2 chars are a hex value. 
// *                                                  
// * Inputs: None                  
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: The number of name/values pairs acutually created.
// *                                                                    
// *********************************************************************
int R_WebServer::ParseRequestHeaders(void)
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In ParseRequestHeaders" << endline;

    // *******************************
    // * Is there a query string     *
    // *******************************
    if (strHeaders.length() == 0) 
        return(0);

    int iNumHeaders=0;
    StringPair pair;

    RWCTokenizer next(strHeaders);
    DRWCString token;                // Will receive each token

    // ******************************************************
    // * Get each name/value pair from the query string     *
    // ******************************************************
    while (!(token=next("||")).isNull())
    {
        token.replace("+", " ");

        if (token.contains("::"))
        {
            pair.SetLeft(UnescapeStr(token.before("::")));
            pair.SetRight(UnescapeStr(token.after("::")));

            logf->debug(LOGWEBCHANNEL) << "(R_WebServer) Name: " << pair.left() 
                                    << " / Value: " << pair.right() << endline;        
            

            if (pair.left() == "COOKIE") 
            {
                ParseCookies(pair.right());
            }
            else
            {
                IncomingEvents.append(pair);
            }

            iNumHeaders++;
        }
    }

    return(iNumHeaders);
}


// *********************************************************************
// *                                                                    
// * Function: ParseRequestData                                             
// *                                                                    
// * Description:   This function parses the strQuery and splits the
// *                query into name/values pairs.  These pairs are
// *                added to the a Single linked list of name/values
// *                pairs in the request class
// *                                                           
// *                The format is name=value pairs separated by &  
// *                with spaces turned into '+'.  '%' indicates that the 
// *                next 2 chars are a hex value. 
// *                                                  
// * Inputs: None                  
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: The number of name/values pairs acutually created.
// *                                                                    
// *********************************************************************
int R_WebServer::ParseRequestData(void)
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In ParseRequestData" << endline;

    // *******************************
    // * Is there a query string     *
    // *******************************
    if (strQuery.length() == 0) 
        return(0);

    int iNumPairs=0;
    StringPair pair;

    RWCTokenizer next(strQuery);
    DRWCString token;                // Will receive each token

    // ******************************************************
    // * Get each name/value pair from the query string     *
    // ******************************************************
    while (!(token=next("&")).isNull())
    {
        token.replace("+", " ");

        if (token.contains("="))
        {
            pair.SetLeft(UnescapeStr(token.before("=")));
            pair.SetRight(UnescapeStr(token.after("=")));
        }
        else
        {
            pair.SetLeft(UnescapeStr(token));
        }

        logf->debug(LOGWEBCHANNEL) << "(R_WebServer) Name: " << pair.left() 
                                << " / Value: " << pair.right() << endline;        
        
        IncomingEvents.append(pair);
        iNumPairs++;
    }

    return(iNumPairs);
}


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
void R_WebServer::SetRSLConfig(  )
{
    // *********************************************************
    // * Open a file log for this specific process if desired! *
    // *********************************************************
    ResReference refUseFileLog = GetDataMember("UseFileLog");
    if (refUseFileLog.isValid()) 
    {
        if (R_Integer::Int(refUseFileLog()))
            SwitchToFileLog();
    }
    else
    {
        // *************************
        // * Set the log level.... *
        // *************************
        ResReference refLogLevel = GetDataMember("LogLevel");
        if (refLogLevel.isValid() && refLogLevel.StrValue().length() > 0) 
        {
            Logf.SetLevel(R_Integer::Int(refLogLevel()));
        }
        
        // ******************************
        // * Set logging subsystems.... *
        // ******************************
        ResReference refWhichSystems = GetDataMember("LogWhichSystems");
        if (refWhichSystems.isValid() && refWhichSystems.StrValue().length() > 0) 
        {
            Logf.SetSubSysMask(refWhichSystems.StrValue());
        }
    }

    // *****************************************************
    // * Get the name of the RSL class to create initially *
    // *****************************************************
    ResReference refStartupClassName = GetDataMember("StartupClassName");
    if (!refStartupClassName.isValid() || refStartupClassName.StrValue().length() == 0) 
    {
        logf->fatal(LOGWEBCHANNEL) << "(R_WebServer) StartupClassName not specified!" << endline;
        exit(-1);
    }
    startupClassName = refStartupClassName.StrValue();

    // ***************************************************************
    // * Get the total number of requests allowed. If the total      *
    // * requests exceeds the number of requests allowed then we are *
    // * going to shutdown this process.                             *
    // ***************************************************************
    iTotalRequestsAllowed = 0;
	ResReference refRequestsAllowed = GetDataMember("TotalRequestsAllowed");
	if (refRequestsAllowed.isValid() && refRequestsAllowed.StrValue().length() > 0)
	{		
	    iTotalRequestsAllowed = R_Integer::Int(refRequestsAllowed());
	}

    // ***********************************
    // * Get the Document Root directory *
    // ***********************************
    ResReference refDocRoot = GetDataMember("DocumentRoot");
    if (!refDocRoot.isValid() || refDocRoot.StrValue().length() == 0) 
        strDocumentRoot = "";
    else
        strDocumentRoot = refDocRoot.StrValue() + "/";

    // ******************************
    // * Get the Document Extension *
    // ******************************
    ResReference refDocExt = GetDataMember("DocumentExt");
    if (!refDocExt.isValid() || refDocExt.StrValue().length() == 0) 
        strDocumentExt = "html";
    else
        strDocumentExt = refDocExt.StrValue();
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
// *        Listen on the specifed port.  When a connection is made, depending
// *        on the toFork variable, either fork and call new connection or
// *        just call new connection.
// *
// * INPUT: 
// *        portnum - Integer containing the port number to listen on.
// *        toFork - Integer to fork = 1 , not to for = 0
// *                                                                  
// *  RETURNS: 
// *      Nothing
// *                                                                  
// ************************************************************************
void R_WebServer::ListenLoop(int portnum,int toFork)
{
    int bindresult=0;

    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In ListenLoop" << endline;

    // ******************
    // * Fork variables *
    // ******************
    Fork::suicide_signal (SIGTERM);
    int killchild = 1;  // kill child when parent terminates.
    int reason = 1;

    
    // **************************************
    // * Set up USR1 parent signal handler. *
    // **************************************
    (*signal)(SIGUSR1, sighandlerUSR1parent);

    sockinetbuf sin (sockbuf::sock_stream);

    // ************************************************************
    // * If there was a port number specified than use that port  *
    // * else connect to the first available port.                *
    // ************************************************************
    if (portnum > 0)
        bindresult=sin.bind(INADDR_ANY, portnum);
    else
        bindresult=sin.bind();
        
    // *****************************************
    // * Could we bind to the address.         *
    // *****************************************
    if (bindresult != 0)
    {
        cerr << "Unable to bind to address.\n";
        return;
    }
        
    // ***************************
    // * Set some socket options *
    // ***************************
    sin.reuseaddr(1);
    sin.keepalive(1);

    logf->notice(LOGWEBCHANNEL) << "(R_WebServer) WebServer started, parent PID " 
                            << (int) (getpid()) << endline;

    logf->notice(LOGWEBCHANNEL) << "(R_WebServer) WebServer listening on " << sin.localhost() 
                            << ", Port: " << sin.localport() << endline;

    cout << "localhost = " << sin.localhost() << endl
         << "localport = " << sin.localport() << endl << flush;

    // ***************
    // * listen loop *
    // ***************
    sin.listen();
    for(;;)
    {
        logf->notice(LOGWEBCHANNEL) << "(R_WebServer) Waiting for connection..." << endline;
        iosockinet cio(sin.accept());

        // *****************************************
        // * Did we get a HUP signal to shutdown?? *
        // *****************************************
        if (shutdown_now)
        {
            logf->notice(LOGWEBCHANNEL) << "(R_WebServer) Parent - Got HUP signal. Shutting down now." 
                                    << endline;
            cio->close();

            exit(1);
        }

        logf->notice(LOGWEBCHANNEL) << "(R_WebServer) Parent - New connection accepted." 
                                << endline;

        if (toFork)
        {
            Fork *spawning = new Fork(killchild, reason);
    
            if (spawning && spawning->process_id() == -1)
            {
                logf->fatal(LOGWEBCHANNEL) << "(R_WebServer) Fork failed!" << endline;
                perror("fork failed");
                exit(-5);
            }
            
            if (spawning->is_child())
                NewConnection(cio);

            // Store child PID.
            if (spawning->is_parent())
            {
                // Set up CLD signal handler.  For some reason, this code
                // needs to be here, in parent AFTER fork, rather than
                // above where the other signal handler is set up.
                (*signal)(SIGCLD, sighandlerCLD);

                childPids.insert (spawning->process_id ());
            }
        }
        else
            NewConnection(cio);

        cio->close();
    }
}

// ***********************************************************************
// *
// * NAME:    NewConnection         Protected Function
// *
// * DESCRIPTION:
// *        CommManager is only used for select(), not for managing
// *        multiple protocols and input streams, etc.
// *
// * INPUT: 
// *                                                                  
// *  RETURNS: 
// *      Nothing
// *                                                                  
// ************************************************************************
void R_WebServer::NewConnection(iosockinet& sio)
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In NewConnection" << endline;

    SetRSLConfig();

    // ********************************************************
    // * Create ECI client object for connecting to the core. *
    // ********************************************************
    pGraniteCore = new ECI_Client();
    if (!pGraniteCore)
    {
        Logf.error(LOGWEBCHANNEL) << "(R_WebServer) can't create ECI_Client object"  << endline;
        return;
    }

    // *******************************************************
    // * Initialize receive time-out values for the channel. *
    // *******************************************************
    int receiveTimeOut;
    ResReference refCoreReceiveTimeOut = GetDataMember("CoreReceiveTimeOut");
    if (refCoreReceiveTimeOut.isValid())
    { 
        receiveTimeOut = R_Integer::Int(refCoreReceiveTimeOut());
        if (receiveTimeOut > 0)
            coreReceiveTimeOut = receiveTimeOut;
    }
        
    ResReference refWebServerReceiveTimeOut = GetDataMember ("WebServerReceiveTimeOut");
    if (refWebServerReceiveTimeOut.isValid()) 
    {
        receiveTimeOut = R_Integer::Int(refWebServerReceiveTimeOut());
        if (receiveTimeOut > 0)
            webServerReceiveTimeOut = receiveTimeOut;
    }

    logf->debug(LOGWEBCHANNEL) << "Core process receive time-out setting = " << coreReceiveTimeOut << endline;
    logf->debug(LOGWEBCHANNEL) << "Web Server receive time-out setting = " << webServerReceiveTimeOut << endline;

    // ****************************************************
    // * Initialize cryptography support for the channel. *
    // ****************************************************
    CryptoSubSystem * cryptoSubsystem = NULL;		// cryptography support subsystem

    ResReference refUseCryptoGraphy = GetDataMember("UseCryptoGraphy");
    if (refUseCryptoGraphy.isValid()) 
    {
        if (R_Integer::Int(refUseCryptoGraphy()))
        {
            cryptoSubsystem = new _CryptoSubSystem;

            // Initialize cryptography sub-system for encryption support.            
            if ( (cryptoSubsystem == NULL) || 
                 (cryptoSubsystem->initialize () != CRYPTO_ERR_OK) )
            {
                Logf.error(LOGWEBCHANNEL) << "Fatal error: unable to initialize cryptography sub-system, for encryption support." << endline;
                return;
            }

            logf->debug(LOGWEBCHANNEL) << "Granite core encryption support enabled." << endline;
        }
    }

    // Set cryptography object of ECI Client object, for encryption
    // support.
    pGraniteCore->cryptoSubsystem = cryptoSubsystem;


    // ***************************************************
    // * Set up for listening on a port.   Use specified *
    // * port or any port if none specified.             *
    // ***************************************************
    int startPort = 0, endPort = 0;
    ResReference resGCorePort = GetDataMember("GraniteCorePortStart");
    if (resGCorePort.isValid())
    {
        startPort = R_Integer::Int(resGCorePort());
        resGCorePort = GetDataMember("GraniteCorePortEnd");
        if (resGCorePort.isValid())
            endPort = R_Integer::Int(resGCorePort());
    }

#ifdef NOBROKER
	if (!pGraniteCore->Connect("horizon", startPort))
	{
	    return;
	}
#else
    iGCorePort = pGraniteCore->Listen(startPort, endPort);
    if (!iGCorePort)
    {
        Logf.error(LOGWEBCHANNEL) << "(R_WebServer) can't listen to core"  << endline;
        return;
    }

    // *************************************************
    // * Create a broker connection object.            *
    // *************************************************

    // Fetch the local host and port that the core will 
    // connect back to.  Use port that was picked in bind above.
    char host[MAXHOSTNAMELEN+1];
    gethostname(host, MAXHOSTNAMELEN);

    // Set cryptography object of network address object, for encryption
    // support.
    NetAddress chnlAddr (host, iGCorePort);
    chnlAddr.cryptoSubsystem = cryptoSubsystem;

    // Fetch address of registrar.
    ResReference resRegAddr = GetDataMember("RegAddr");
    if (!resRegAddr.isValid())
    {
        Logf.error(LOGWEBCHANNEL) << "Missing registrar address, unable to connect to broker." << endline;
        return;
    }

    // Set cryptography object of network address object, for encryption
    // support.
    NetAddress regAddr (resRegAddr.StrValue());
    regAddr.cryptoSubsystem = cryptoSubsystem;

    // Create broker connection object.
    brkConn = new BrokerConnection (chnlAddr, regAddr);
    if (!brkConn)
    {
        Logf.error(LOGWEBCHANNEL) << "(R_WebServer) can't create broker connection object"  << endline;
        return;
    }
#endif

    // ******************************************
    // * Initialize the client streaming socket *
    // ******************************************
    pioClient = &sio;

    // ******************************************************
    // * Initialize some variables for the timeout manager. *
    // ******************************************************
    time_t secsRemaining = 0, secsToUse = 0;

    iTotalRequests = 0;

    // ****************************************************************
    // * Install the signal handler used for dynamic reconfiguration **
    // ****************************************************************
    read_dynamic_config = 0;
    (*signal)(SIGUSR1, sighandlerUSR1);
    
    // ***********************************************
    // **   Primary Loop for the child Web Server.  **
    // ***********************************************
    logf->notice(LOGWEBCHANNEL) << "(R_WebServer) Entering primary child Web server loop." << endline;

    for(;;)
    {
        // ****************************************************************
        // * compute time to select() with timeout manager remaining time *
        // ****************************************************************
        if (timeoutManager)
        {
            secsRemaining = timeoutManager->tm.SecsToTimeout();

            // ********************************************************
            // * Figure out the seconds remaining to the next timeout *
            // ********************************************************
            if (timeoutManager->tm.isEmpty())
                secsToUse = 300;    // use some "large" default value.
            else
                secsToUse = (secsRemaining < 0) ? 0 : secsRemaining;
            
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
                
            logf->info(LOGWEBCHANNEL) << "(R_WebServer) Seconds until first timeout event: " << (int) secsRemaining
                                  << ". Waiting for " << (int) secsToUse << " seconds.." << endline;

            // ******************************
            // * Wait for an incoming event *
            // ******************************
            if(sio->is_readready(secsToUse))
            {
                RoundTripRequest();
                iTotalRequests++;
                if ((iTotalRequestsAllowed > 0) && (iTotalRequests >= iTotalRequestsAllowed))
                {
                    ShutdownNow();
                }
            }

            // ************************************************
            // * Dynamically reconfigure the server if needed *
            // ************************************************
            if (read_dynamic_config)
                ReadDynamicConfig();
            
            // ***************************************
            // * Has a timeout occured in the system *
            // ***************************************
            if (timeoutManager->timeoutReady())
            {
                logf->notice(LOGWEBCHANNEL) << "(R_WebServer) Ready to send timeout events." << endline;
                AuditRequest *toutgoing = timeoutManager->tm.SendTimeouts();

                // **********************************
                // * toutgoing is probably an elist *
                // **********************************
                if (toutgoing)
                {
                    logf->info(LOGWEBCHANNEL) << "(R_WebServer) Events received from timeout manager: "
                                          << toutgoing << endline;

                    // ***********************************************
                    // * Set current session id from timeout manager *
                    // ***********************************************
                    strCurrentSession = toutgoing->object_id;

                    logf->info(LOGWEBCHANNEL) << "(R_WebServer) outbound tm events for session "
                                          << strCurrentSession << endline;
                }
                else
                {
                    logf->info(LOGWEBCHANNEL) << "(R_WebServer) No events returned from timeout manager." 
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
                
            // ******************************
            // * Wait for an incoming event *
            // ******************************
            if(sio->is_readready(secsToUse))
            {
                RoundTripRequest();
                iTotalRequests++;
                if ((iTotalRequestsAllowed > 0) && (iTotalRequests >= iTotalRequestsAllowed))
                {
                    ShutdownNow();
                }
            }

            // ************************************************
            // * Dynamically reconfigure the server if needed *
            // ************************************************
            if (read_dynamic_config)
                ReadDynamicConfig();
        }
    }
    
    // Destroy the broker connection.
    if (brkConn)
        delete brkConn;

}

// ***********************************************************************
// *
// * NAME:    InitRequest         Protected Function
// *
// * DESCRIPTION:
// *        Initialize the Request
// *
// * INPUT: 
// *        None
// *                                                                  
// *  RETURNS: 
// *        Nothing
// *                                                                  
// ************************************************************************
void R_WebServer::InitRequest()
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In InitRequest" << endline;

    // *************************************
    // * Clear the list of incoming events *
    // *************************************
    IncomingEvents.clear();

    // *************************
    // * Clear the HTML string *
    // *************************
    OutHtmlFile.clear();
    iFileLen = 0;
}


// ***********************************************************************
// *
// * NAME:    GetIncomingRequest         Protected Function
// *
// * DESCRIPTION:
// *            Read the query string from the socket.
// *
// * INPUT: 
// *                                                                  
// *  RETURNS: 
// *      Nothing
// *                                                                  
// ************************************************************************
void R_WebServer::GetIncomingRequest()
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In GetIncomingRequest" << endline;
    
    GetHttpHeaders();
    GetHttpRequest();

    // *********************
    // * Clear the socket **
    // *********************
    ioClient().clear();
}


// ***********************************************************************
// *
// * NAME:    GetHttpHeaders         Protected Function
// *
// * DESCRIPTION:
// *            Read the Http headers from the socket.
// *
// * INPUT: 
// *                                                                  
// *  RETURNS: 
// *      Nothing
// *                                                                  
// ************************************************************************
void R_WebServer::GetHttpHeaders()
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In GetHttpHeaders" << endline;

    // ******************************************
    // * Read the query string from the socket. *
    // ******************************************
    RWCString strLine;
    int iLength=-1;

    DRWCString strLength;

    int iTimeOut = ioClient()->recvtimeout (webServerReceiveTimeOut);

    // *********************************************
    // * The first thing read should be the length *
    // *********************************************
    if(!strLength.readLine(ioClient()).good())
    {
        logf->fatal(LOGWEBCHANNEL) << "(R_WebServer) Error reading message length from client socket... exiting" << endline;

        ioClient().clear();
        exit(-1);
    }

    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) Header Block Length " << strLength << endline;

    iLength = atoi(strLength.data());    

    // ******************************************************
    // * if the length is -1 then we have a read error.     *
    // ******************************************************
    if (iLength < 0) 
    {
        logf->fatal(LOGWEBCHANNEL) << "(R_WebServer) Error invalid message length received from client socket... exiting" << endline;

        ioClient().clear();
        exit(-1);
    }

    // *************************************************
    // * Send a ACK back across the socket to indicate *
    // * that we got the data successfully             *
    // *************************************************
    ioClient() << "ACK" << flush;
    
    // *****************************************************
    // * Read the specifed number of bytes from the stream *
    // *****************************************************
    strHeaders = "";
    if (iLength > 0) 
    {
        int i;
        for(i=0; i<iLength; i+=strLine.length())
        {
            ioClient()->recvtimeout (webServerReceiveTimeOut);

            if (!strLine.readLine(ioClient(), 0).good())
                break;

            strHeaders += strLine;
        }    

        logf->debug(LOGWEBCHANNEL) << "(R_WebServer) Header Block " << strHeaders << endline;

        // ****************************************************************
        // * if we were unable to read enough bytes we have a read error. *
        // ****************************************************************
        if (i < iLength)
        {
            logf->fatal(LOGWEBCHANNEL) << "(R_WebServer) Error reading request from client socket... exiting" << endline;

            ioClient().clear();
            exit(-1);
        }

        // *************************************************
        // * Send a ACK back across the socket to indicate *
        // * that we got the data successfully             *
        // *************************************************
        ioClient() << "ACK" << flush;
    }
    
    // *************************************************************
    // * Parse the request headers into a list of name/value pairs *
    // *************************************************************
    ParseRequestHeaders( );
}


// ***********************************************************************
// *
// * NAME:    GetHttpRequest         Protected Function
// *
// * DESCRIPTION:
// *            Read the query string from the socket.
// *
// * INPUT: 
// *                                                                  
// *  RETURNS: 
// *      Nothing
// *                                                                  
// ************************************************************************
void R_WebServer::GetHttpRequest()
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In GetHttpRequest" << endline;

    // ******************************************
    // * Read the query string from the socket. *
    // ******************************************
    RWCString strLine;
    int iLength=-1;

    DRWCString strLength;

    int iTimeOut = ioClient()->recvtimeout (webServerReceiveTimeOut);

    // *********************************************
    // * The first thing read should be the length *
    // *********************************************
    if(!strLength.readLine(ioClient()).good())
    {
        logf->fatal(LOGWEBCHANNEL) << "(R_WebServer) Error reading message length from client socket... exiting" << endline;

        ioClient().clear();
        exit(-1);
    }

    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) Reguest Data Length " << strLength << endline;

    iLength = atoi(strLength.data());    

    // ******************************************************
    // * if the length is -1 then we have a read error.     *
    // ******************************************************
    if (iLength < 0) 
    {
        logf->fatal(LOGWEBCHANNEL) << "(R_WebServer) Error invalid message length received from client socket... exiting" << endline;

        ioClient().clear();
        exit(-1);
    }

    // *************************************************
    // * Send a ACK back across the socket to indicate *
    // * that we got the data successfully             *
    // *************************************************
    ioClient() << "ACK" << flush;

    // *****************************************************
    // * Read the specifed number of bytes from the stream *
    // *****************************************************
    strQuery = "";
    if (iLength > 0) 
    {
        int i;
        for(i=0; i<iLength; i+=strLine.length())
        {
            ioClient()->recvtimeout (webServerReceiveTimeOut);

            if (!strLine.readLine(ioClient(), 0).good())
                break;

            strQuery += strLine;
        }    

        logf->debug(LOGWEBCHANNEL) << "(R_WebServer) Request Data " << strQuery << endline;

        // ****************************************************************
        // * if we were unable to read enough bytes we have a read error. *
        // ****************************************************************
        if (i < iLength)
        {
            logf->fatal(LOGWEBCHANNEL) << "(R_WebServer) Error reading request from client socket... exiting" << endline;

            ioClient().clear();
            exit(-1);
        }

        // *************************************************
        // * Send a ACK back across the socket to indicate *
        // * that we got the data successfully             *
        // *************************************************
        ioClient() << "ACK" << flush;
    }
    
    // **********************************************************
    // * Parse the request data into a list of name/value pairs *
    // **********************************************************
    ParseRequestData( );
}


// ***********************************************************************
// *
// * NAME:    ValidRequest         Protected Function
// *
// * DESCRIPTION:
// *        Check if this is a valid Request
// *
// * INPUT: 
// *        None
// *                                                                  
// *  RETURNS: 
// *        Nothing
// *                                                                  
// ************************************************************************
int R_WebServer::ValidRequest()
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In ValidRequest" << endline;

    return (1);
}
    
// ***********************************************************************
// *
// * NAME:    FinalizeRequest         Protected Function
// *
// * DESCRIPTION:
// *        Finalize the Request
// *
// * INPUT: 
// *        None
// *                                                                  
// *  RETURNS: 
// *        Nothing
// *                                                                  
// ************************************************************************
void R_WebServer::FinalizeRequest()
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In FinalizeRequest" << endline;

    // *************************************************
    // * Send html page back to client or in this case *
    // * back to the NSAPI that sent us the request.   *
    // *************************************************
    ioClient() << iFileLen << "\n" << flush;
    ioClient() << OutHtmlFile << flush;

    // ***********************
    // * Free the ECI_Client *
    // ***********************
#ifndef NOBROKER
    if (pGraniteCore)
        pGraniteCore->Disconnect();
#endif

#ifdef PURIFYTEST
    if (purify_is_running())
    {
        purify_printf("Request complete.\n");
        purify_new_leaks();
    }
#endif
}


// ************************************************************************
// *                                                                       
// * NAME:    ReadDynamicConfig             Protected Function                           
// *                                                                       
// * DESCRIPTION:                                                          
// *        Use an ECI server (R_Server) to process eci commands to                 
// *        do dynamic stuff like change server config parameters.                  
// *        Could also be used to do anything ECI can do, like add/update RSL code. 
// *                                                                       
// * INPUT:                                                                
// *        None
// *                                                                       
// * RETURNS:                                                              
// *        None
// *                                                                       
// ************************************************************************
void R_WebServer::ReadDynamicConfig()
{
    // *****************************************************************************
    // * Allocate a static ECI server object, to be used only within this context, *
    // * so we don't keep re-allocating it during the life of                      *
    // * the server. Note that a declaration like this does not run the            *
    // * RSL Init() method, as it would if it were R_Server::New("test_server");   *
    // *****************************************************************************
    static R_Server rs("test-server");

    Logf.alert(LOGWEBCHANNEL) << "(R_WebServer) Dynamic reconfiguration in progress." << endline;

    read_dynamic_config = 0;
    
    // *******************************************************
    // * The server will process ECI commands from a file... *
    // * first get the name of the file to use.              *
    // *******************************************************
    ResReference resReconfigFile = GetDataMember("ReconfigFile");
    RWCString filename;

    if (resReconfigFile.isValid())
        filename = resReconfigFile.StrValue();
    
    if (filename.length() == 0) // RSL data member might not be set.
    {
        // provide a default name
        logf->notice(LOGWEBCHANNEL) << "(R_WebServer) Couldn't find server configuration variable `ReconfigFile'."
                                << "using default name `reconfig.rsl'." << endline;
        
        filename = "reconfig.rsl";
    }
    
    fstream reconfigf(filename.data(), ios::in);

    if (reconfigf)
    {
        rs.NewConnection(reconfigf);
        reconfigf.close();

        // *******************************************
        // * Reinitilize the configuration variables *
        // *******************************************

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

        // ***************************************************************
        // * Get the total number of requests allowed. If the total      *
        // * requests exceeds the number of requests allowed then we are *
        // * going to shutdown this process.                             *
        // ***************************************************************
        iTotalRequestsAllowed = 0;
    	ResReference refRequestsAllowed = GetDataMember("TotalRequestsAllowed");
    	if (refRequestsAllowed.isValid() && refRequestsAllowed.StrValue().length() > 0)
    	{		
    	    iTotalRequestsAllowed = R_Integer::Int(refRequestsAllowed());
    	}
    }
}


// ************************************************************************
// *                                                                       
// * NAME:    ShutdownNow             Protected Function                           
// *                                                                       
// * DESCRIPTION:                                                          
// *        Shuts the server down immediately by closing the socket and 
// *        exiting.
// *                                                                       
// * INPUT:                                                                
// *        None
// *                                                                       
// * RETURNS:                                                              
// *        None
// *                                                                       
// ************************************************************************
void R_WebServer::ShutdownNow()
{
    Logf.alert(LOGWEBCHANNEL) << "(R_WebServer) Shutdown now." << endline;

    ioClient()->close();
    exit(0);
}


// *************************************************************************
// **                         PUBLIC MEMBER FUNCTIONS                      *
// **                                                                      *
// **                       Constructors for this class                    *
// *************************************************************************

// ***********************************************************************
// *
// * NAME:    R_WebServer         Constructor
// *
// * DESCRIPTION:
// *      Constructor for the the R_WebServer class
// *
// * INPUT: 
// *                                                                  
// *  RETURNS: 
// *                                                                  
// ************************************************************************
R_WebServer::R_WebServer(RWCString nm) :
	rslServer(nm, (res_class *) R_WebServer::rslType)
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
ResStatus R_WebServer::execute(int method, ResList& arglist)
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In execute" << endline;

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
// *      Returns a R_WebServer
// *                                                                       
// ************************************************************************
R_WebServer *R_WebServer::New(RWCString n)
{
    Resource *r= R_WebServer::rslType->New(n);
    return (R_WebServer *) r;
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
void R_WebServer::SwitchToFileLog()
{
    DRWCString logname;

    ResReference resLogName = GetDataMember("LogFileName");
    if (resLogName.isValid() && resLogName.StrValue().length() > 0) 
    {
        logname = resLogName.StrValue();
    }
    else
    {
        logf->alert(LOGWEBCHANNEL) << "(R_WebServer) LogFileName not specified." << endline;
        return;
    }

    logname += DRWCString(".") + dec(getpid(), 0);

    cout << "Switching to logfile `" << logname << "'\n";

    // ********************************************************
    // * Record the new logfile name (probably to the syslog) *
    // ********************************************************
    logf->notice(LOGWEBCHANNEL) << "(R_WebServer) Using new logfile: '" 
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


// ***************************************************************
// *  Signal USR1 handler                                        *
// ***************************************************************
void sighandlerUSR1(int i)
{
    // *********************
    // * Reinstate Handler *
    // *********************
    (*signal)(SIGUSR1, sighandlerUSR1);

    // ********************************************
    // * Set the flag, checked in NewConnection() *
    // ********************************************
    read_dynamic_config = 1;
}

// ***************************************************************
// *  Signal USR1 parent handler                                 *
// ***************************************************************
void sighandlerUSR1parent(int x)
{
    for (int i = 0; i < childPids.entries (); i++)
    {
        // cout << "USR1ing " << childPids[i] << endl;
        Fork *fork = new Fork ();
    if (fork)
      if (fork->is_child ())
      {
          char pid [10];
          sprintf (pid, "%d", childPids[i]);
          execlp ("kill", "kill", "-USR1", pid, 0);
          exit (-1);
      }
    }

    // *********************
    // * Reinstate Handler *
    // *********************
    (*signal)(SIGUSR1, sighandlerUSR1parent);
}

// ***************************************************************
// *  Signal CLD parent handler                                  *
// ***************************************************************
void sighandlerCLD(int i)
{
    // Remove from child pid list.
    int status;
    childPids.remove(wait(&status));

    // Restore signal handler.
    (*signal)(SIGCLD, sighandlerCLD);
}

// ***************************************************************
// *  Signal HUP handler                                         *
// ***************************************************************
void sighandlerHUP(int i)
{
    shutdown_now = 1;
}
