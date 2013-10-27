// ***************************************************************************
// *
// *  NAME:  R_Server.cc
// *
// *  RESOURCE NAME:    Server                                        
// *                                                                    
// *  RELATED RESOURCES: 
// *                                                                    
// *  DESCRIPTION:                                                      
// *     ECI Server Resource
// *                                                                    
// * $Id: R_Server.cc,v 1.10 1999/01/07 14:47:15 cking Exp $
// *
// * $Log: R_Server.cc,v $
// * Revision 1.10  1999/01/07 14:47:15  cking
// * Additional performance enhancements for encryption support in the GF 2.5 release.
// *
// * Revision 1.9  1998/12/22 20:22:42  cking
// * Additional modification for encryption support in the GF 2.5 release.
// *
// * Revision 1.8  1998/12/22 19:56:52  toddm
// * Reimplement Charles' changes from version 1.6
// *
// * Revision 1.6  1998/12/17 21:45:28  cking
// * Additional modification for encryption support in the GF 2.5 release.
// *
// * Revision 1.5  1998/12/14 17:54:11  cking
// * Additional modifications for encryption support in GF 2.5 release
// *
// * Revision 1.4  1998/12/02 18:46:50  cking
// * Performance modifications for encryption support for the GF 2.5 release.
// *
// * Revision 1.3  1998/11/30 20:36:34  cking
// * Additional modifications for encryption support in GF 2.5 release
// *
// * Revision 1.2  1998/11/23 19:37:39  cking
// * Additional modifications to initial implementation of encryption support for GF.
// *
// * Revision 1.1  1998/11/17 23:53:32  toddm
// * Initial revision
// *
// * Revision 2.3  1998/06/29 19:29:28  toddm
// * Move runtime.h to R_Server.cc
// *
// * Revision 2.2  1998/04/30 18:46:26  toddm
// * Add RecycleServer method
// *
// * Revision 2.1  1998/03/27 18:17:57  toddm
// * Add an execute method and make NewConnection a virtual fuction
// *
// * Copyright (c) 1995, 1996, 1997, 1998 by Destiny Software Corporation
// *
// ***************************************************************************

// *******************
// * System Includes *
// *******************
#include <iostream.h>
#include <fstream.h>
#include <stdlib.h>
#include <stdio.h>			// Standard I/O
#include <stddef.h>			// Standard Definitions
#include <stdlib.h>			// Standard Library
#include <stream.h>
#include <iostream.h>
#include <strstream.h>

// ******************
// * Local Includes *
// ******************
#include "R_Server.h"
#include "lexer_context.h"
#include "slog.h"
#include "R_Integer.h"
#include "b.h"
#include "R_TimeoutManager.h"
#include "destiny.h"
#include "runtime.h"
#include "killevents.h"


extern ofstream rslerr;

extern int parse_it(lexer_context &lexc);
extern slog *logf;

int encryption = FALSE;

extern "C" res_class *Create_Server_RC()
{
    return &(R_Server::rslType);
}

// R_Server static member
rc_Server R_Server::rslType("Server");


// ************************************************************************
// ************************************************************************
// *                       RES_CLASS - rc_Server                       *
// ************************************************************************
// ************************************************************************

// ************************************************************************
// *
// * NAME:  spawn - Private function
// *
// * DESCRIPTION:
// *        Create a new resource of this type (R_Server).
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
Resource* rc_Server::spawn(RWCString nm)
{
    return new R_Server(nm);    // or other constructor
}


// *************************************************************************
// **                      VIRTUAL MEMBER FUNCTION                         *
// **                                                                      *
// **             Cryptography SubSystem of Server specific                *
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


// *************************************************************************
// **                      PROTECTED MEMBER FUNCTIONS                      *
// **                                                                      *
// **                           Server specific                            *
// *************************************************************************

// ***********************************************************************
// *
// * NAME:    RecycleServer         Protected Function
// *
// * DESCRIPTION:
// *        This method is used to check if the total number of sessions
// *        processed by this server has execeeded the session limit(TotalSessionAllowed).
// *        If so then when the total number of current sessions drops to 
// *        zero, return True.
// *
// * INPUT: None
// *
// * OUTPUT: None
// *                                                                  
// *  RETURNS:  True if the recycle criteria has been met.
// *            False otherwise.
// *                                                                  
// ************************************************************************
int R_Server::RecycleServer(void)
{
    // *******************************************************************
    // * Get the total number of sessions allowed. If the total sessions *
    // * exceeds the number of sessions allowed then we are going to     *
    // * shutdown this process when the number of active session         *
    // * reaches zero                                                    *
    // *******************************************************************
    if (!quitWhenZeroSessions)
    {
    	ResReference resAppsAllowed = GetDataMember("TotalSessionsAllowed");
    	if (resAppsAllowed.isValid() && resAppsAllowed.StrValue().length() > 0)
    	{			
            if (R_Integer::Int(resAppsAllowed()) > 0)
            {
    			if (runtimeStuff.TotalUserSessions() >= R_Integer::Int(resAppsAllowed()))
    			{
    				logf->notice(LOGSERVER) << "(R_WebServer) TotalSessionsAllowed threshold of "
                        					<< (runtimeStuff.TotalUserSessions())
    					                    << " has been reached. Server will shutdown when the last session quits."
                        					<< endline;
    				quitWhenZeroSessions = TRUE;
    			}
            }
    	}
    }

    if (quitWhenZeroSessions && runtimeStuff.CurrentUserSessions() == 0)
    {
        logf->notice(LOGSERVER) << "Zero sessions reached: Shutdown in progress."
                                << endline;
        return(TRUE);
    }
    
    return(FALSE);
}

// *************************************************************************
// **                         PUBLIC MEMBER FUNCTIONS                      *
// **                                                                      *
// **                       Constructors for this class                    *
// *************************************************************************

// ***********************************************************************
// *
// * NAME:    R_Server         Constructor
// *
// * DESCRIPTION:
// *      Constructor for the the R_Server class  (NOT FOR SUBCLASS)
// *
// * INPUT: 
// *                                                                  
// *  RETURNS: 
// *                                                                  
// ************************************************************************
R_Server::R_Server(RWCString nm)
    : rslServer(nm, (res_class *) &R_Server::rslType)
{
    incomingArgs = NULL;
    dhdeSn = NULL;		// diffie-hellman & digital hellman session descriptor
    cryptoSubsystem = NULL;	// cryptography support subsystem
}

// ***********************************************************************
// *
// * NAME:    R_Server         Constructor
// *
// * DESCRIPTION:
// *      Constructor for the the R_Server subclass
// *
// * INPUT: 
// *                                                                  
// *  RETURNS: 
// *                                                                  
// ************************************************************************
R_Server::R_Server(RWCString n, res_class *super)
    : rslServer(n, super)
{
    incomingArgs = NULL;
    dhdeSn = NULL;		// diffie-hellman & digital hellman session descriptor
    cryptoSubsystem = NULL;	// cryptography support subsystem
}

// ***********************************************************************
// *
// * NAME:    ListenLoop         Public Function
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
void R_Server::ListenLoop(int portnum,int toFork)
{
    int nerrors=0, bindresult=0;

    logf->debug(LOGSERVER) << "(R_ECIServer) In ListenLoop" << endline;

    // ******************
    // * Fork variables *
    // ******************
    Fork::suicide_signal (SIGTERM);
    int killchild = 1;  // kill child when parent terminates.
    int reason = 1;

    // ************************************************************
    // * If there was a port number specified than use that port  *
    // * else connect to the first available port.                *
    // ************************************************************
    sockinetbuf sin (sockbuf::sock_stream);

    if (portnum > 0)
    {
        bindresult=sin.bind(INADDR_ANY, portnum);
    }
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

    cout << "localhost = " << sin.localhost() << endl
        << "localport = " << sin.localport() << endl << flush;

    // ***************
    // * listen loop *
    // ***************
    sin.listen();
    for(;;)
    {
        Logf.notice(LOGSERVER) << "RSL Server: Waiting for connection..." << endline;
        cout << "RSL Server: Waiting for connection...\n" << flush;

        iosockinet s(sin.accept());

        logf->notice(LOGSERVER) << "RSL Server: Parent - New connection accepted." 
                                << endline;
        if (toFork)
        {
            Fork *spawning = new Fork(killchild, reason);
    
            if (spawning && spawning->process_id() == -1)
            {
                perror("fork failed");
                exit(-5);
            }
            
            if (spawning->is_child())
            {
                NewConnection(s);
                delete spawning;
                return;
            }

            delete spawning;
        }
        else
            NewConnection(s);

        s->close();
    }
}

// ***********************************************************************
// *
// * NAME:    NewConnection         Public Function
// *
// * DESCRIPTION:
// *        This method is used to process the read of the iostream
// *
// * INPUT: in - iostream used to read from a file during the 
// *            reconfiguration process
// *                                                                  
// *  RETURNS: 
// *      Nothing
// *                                                                  
// ************************************************************************
void R_Server::NewConnection(iostream& in)
{
    lexc.SetSource(lexer_context::Stream, "<reconfiguration>");
    lexc.SetLexerIO(in, cerr);
    lexc.done = lexer_context::Ready;
    lexc.type = lexer_context::interactive;
    lexc.keystate = lexer_context::Everywhere;

    do
    {
        // If request invalid, we're at the end, quit.
        if (RoundTripRequest() != 0)
            return;
    } while (lexc.done == lexer_context::Ready);
}


void R_Server::closeCryptographySession (void)
{
    if (cryptoSubsystem != NULL)
    {
        // Close cryptography session, and free cryptography objects.
        if (dhdeSn != NULL)
        {
            cryptoSubsystem->closeSession (dhdeSn);
            delete (dhdeSn);
        }
        delete (cryptoSubsystem);

        dhdeSn = NULL;
        cryptoSubsystem = NULL;
    }               
}


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
void R_Server::NewConnection(iosockinet& in)
{
    logf->debug(LOGSERVER) << "(R_Server) In NewConnection" << endline;

    cerr << "RSL Server: Accepted connection, pid " << (getpid()) << endl
         << "Switching lexer streams with client socket.\n" << flush;

    // ****************************
    // * Set current lexer state **
    // ****************************
    lexc.SetSource(lexer_context::Stream, "<network>");
    lexc.SetLexerIO(in, in);
    lexc.done = lexer_context::Ready;
    lexc.type = lexer_context::interactive;
    lexc.keystate = lexer_context::Everywhere;

//  // print a first prompt (if applicable)
//  if (lexc.type == lexer_context::prompt)
//      lexc.Out() << "RSL> " << flush;

    // ******************************************************
    // * Initialize some variables for the timeout manager. *
    // ******************************************************
    time_t secsRemaining = 0, secsToUse = 0;

    dhdeSn = NULL;
    cryptoSubsystem = NULL;
    ResReference refUseCryptoGraphy = GetDataMember("UseCryptoGraphy");
    if (encryption) 
    {
        // create cryptography objects required for encryption support.
        cryptoSubsystem = new _CryptoSubSystem;
        dhdeSn = new CryptoDHDEsession;

        // initialize cryptography sub-system for encryption support.            
        if ( (dhdeSn == NULL ) ||
             (cryptoSubsystem == NULL) ||
             (cryptoSubsystem->initialize () != CRYPTO_ERR_OK) )
        {
            if (cryptoSubsystem)
                delete (cryptoSubsystem);
            if (dhdeSn)
                delete (dhdeSn);
            
            logf->error(LOGSERVER) << "(R_Server::NewConnection: ) Fatal error: unable to initialize cryptography support system." << endline;
            return;
        }
    
        // Set cryptography session attributes.
        dhdeSn->setNodeType (CRYPTO_NODE_USER_PARTY);			// set node type       
        dhdeSn->setPeerNodeType (CRYPTO_PEER_NODE_CENTRAL_AUTHORITY);	// set peer node type       
        dhdeSn->setPeerNodeConnection (&in);				// set socket connection to remote peer
        dhdeSn->setCentralAuthorityNodeConnection (&in);			// set socket connection to CA        

        // Open cryptography session.
        CryptoError crc;
        if ((crc = cryptoSubsystem->openSession (dhdeSn)) != CRYPTO_ERR_OK)
        {
            logf->error (LOGSERVER) << "(R_Server) Fatal error: unable to open cryptography session; error = " << crc << endline;
            return;
        }

        logf->debug(LOGSERVER) << "(R_Server) Granite core encryption support enabled." << endline;
    }

    // *****************************************
    // **   Primary Loop for the R_Server.  **
    // *****************************************
    do
    {
        // ****************************************************************
        // * compute time to select() with timeout manager remaining time *
        // ****************************************************************
        if (timeoutManager)
        {
            secsRemaining = timeoutManager->tm.SecsToTimeout();
            secsToUse = (secsRemaining > 0) ? secsRemaining : 300;
            
            logf->info(LOGSERVER) << "(R_Server) Seconds until first timeout event: " << (int) secsRemaining
                                  << ". Waiting for " << (int) secsToUse << " seconds.." << endline;

            // ***********************************************************
            // * wait for input for the remaining time until the        **
            // * next timeout event, at which point is_readready() will **
            // * return zero.                                           **
            // ***********************************************************
//          printf ("(R_Server) Before is_readready.\n");
            logf->debug(LOGSERVER) << "(R_Server) Before is_readready." << endline;
            if (in->is_readready(secsToUse, 0))
            {
                logf->debug(LOGSERVER) << "(R_Server) Before Round Trip Request." << endline;
//              printf ("(R_Server) Before Round Trip Request.\n");
                if (RoundTripRequest() != 0)
                {
//                  printf ("(R_Server) After Round Trip Request (EOF).\n");
                    logf->debug(LOGSERVER) << "(R_Server) After Round Trip Request." << endline;

                    // Close cryptography session after processing request with client.
                    closeCryptographySession ();

                    return;
                }
//              printf ("(R_Server) After Round Trip Request.\n");
                logf->debug(LOGSERVER) << "(R_Server) After Round Trip Request." << endline;

            }
//          printf ("(R_Server) After is_readready.\n");
            logf->debug(LOGSERVER) << "(R_Server) After is_readready." << endline;

            while (timeoutManager->timeoutReady())
            {
                cerr << "(R_Server) Ready to send timeout events.\n";
                event *toutgoing = timeoutManager->tm.SendTimeouts();
            }
        }
        else
        {
//          printf ("(R_Server) Before Round Trip Request.\n");
            logf->debug(LOGSERVER) << "(R_Server) Before Round Trip Request." << endline;
            if (RoundTripRequest() != 0)
            {
//              printf ("(R_Server) After Round Trip Request (EOF).\n");
                logf->debug(LOGSERVER) << "(R_Server) After Round Trip Request (EOF)." << endline;

                // Close cryptography session after processing request with client.
                closeCryptographySession ();
                
                return;
            }
//          printf ("(R_Server) After Round Trip Request.\n");
            logf->debug(LOGSERVER) << "(R_Server) After Round Trip Request." << endline;
        }                       

        
    } while (lexc.done == lexer_context::Ready);

    // Close cryptography session.
    closeCryptographySession ();
}

// GetIncomingRequest
// Will use the parser to get one ECI request, to be stored in
// R_Server::incomingEvent.
void R_Server::GetIncomingRequest()
{
#ifdef RSLERR
    rslerr << "R_Server::GetIncomingRequest()\n";
#endif

    if ( (dhdeSn != NULL) && (cryptoSubsystem != NULL) )
    {
        logf->debug(LOGSERVER) << "R_Server::GetIncomingRequest(): Set lexer context for cryptography processing" << endline;

        CryptoError crc;
        int eciPlainTextLength; 
        char *eciPlainText = NULL;
        istream& prevIn = lexc.In ();		// Preserve current lexer context input stream setting
        
        crc = cryptoSubsystem->decryptData (dhdeSn, (iosockinet *) &prevIn, &eciPlainText, &eciPlainTextLength);
        if (crc != CRYPTO_ERR_OK)
        {
           logf->debug(LOGSERVER) << "Fatal error: unable to decrypt ECI request; error = " << crc << endline;
           return;
        }

        // Set the lexer context input stream to input string stream
        // that contains the decrypted stream data.
        istrstream lexerInputStream (eciPlainText, eciPlainTextLength);
        lexc.SetLexerIO (lexerInputStream, lexc.Out());

        int nerrors = parse_it(lexc);

        // Reset the lexer context input stream to the previous setting
        lexc.SetLexerIO (prevIn, lexc.Out());
        if (eciPlainText != NULL)
            free (eciPlainText);	// deallocate buffer allocated for plain text data

#ifdef RSLERR
        rslerr << "Returned from cryptography parse_it(): " << nerrors << endl;
        rslerr << "lexc.done == " << lexc.done << endl;
#endif
    }
    else
    {
        int nerrors = parse_it(lexc);

#ifdef RSLERR
        rslerr << "Returned from parse_it(): " << nerrors << endl;
        rslerr << "lexc.done == " << lexc.done << endl;
#endif
    }
}

// ValidRequest
// Not used yet -- will be when NewConnection and ListenLoop are
// modified to do select, as well as RoundTripRequest/GetIncomingRequest.
int R_Server::ValidRequest()
{
    return (lexc.done == lexer_context::GotECIReq && lexc.dynamic_request);
}

//  ExecuteIncomingEvents()
event *R_Server::ExecuteIncomingEvents(event *e)
{
#ifdef RSLERR
    rslerr << "R_Server::ExecuteIncomingEvents()\n";
#endif

    if (lexc.done == lexer_context::GotECIReq)
    {
        if (e && e->isA(event::requestKind))    // should be equal to lexc.dynamic_request
        {
            Request *areq = (AuditRequest *) e;
    
#ifdef RSLERR
            rslerr << "\nGOT ECI REQUEST:\n";
            areq->print(rslerr);
#endif
    
            lexc.done = lexer_context::Ready;

#ifdef RSLERR
            rslerr << "R_Server:processing ECI request..\n" << flush;
#endif
            // process request
            return areq->execute(NULL);
        }
#ifdef RSLERR
        else
            rslerr << "\nIncoming Event is not a Request (or AuditRequest).\n";
#endif
            
    }

    return NULL;
}

// TranslateOutLoop()
// construct & send the ECI response
// this uses the orignal incoming request as the structure
// of the response (object, session, method, audit) and
// substitutes the outgoing event(s) (a list of arguments)
// as the arg list.. if there is no outgoing event, however,
// then the original incoming arguments are used in the response.
void R_Server::TranslateOutLoop(event *outgoing)
{
    Request *areq = (AuditRequest *) (lexc.dynamic_request);

    // format outgoing data as `arguments' in the ECI response
    // or simply reflect the incoming arguments.

    elistArg *outArgs = NULL;
// Commented out TGM - 12/21/98
//    bool outArgsAllocated;

    if (outgoing && outgoing->isA(event::elistArgKind))
    {
        incomingArgs = areq->arguments; // save the incoming arguments

        //areq->SetArgs((elistArg *) outgoing);
        outArgs = (elistArg *) outgoing;
// Commented out TGM - 12/21/98
//        outArgsAllocated = FALSE;
    }
    else    // any other outgoing event. (what about elist?)
    {
        outArgs = (elistArg *) Remember (new elistArg);
        outArgs->add(outgoing);
// Commented out TGM - 12/21/98
//        outArgsAllocated = TRUE;
    }

    areq->SetArgs(outArgs);

    logf->debug(LOGSERVER) << "R_Server::TranslateOutLoop(): Before processing ECI response." << endline;

    if ( (dhdeSn != NULL) && (cryptoSubsystem != NULL) )
    {
        // Encrypt ECI response for client, via cryptography session.
        CryptoError crc;
        ostream& socket = lexc.Out ();
        ostrstream eciResponse;		// No buffer specification, causes ostrstream to manage any memory
                                        // requirements necessary for output operations.

        // Construct ECI response
        areq->print (eciResponse);
    
        logf->debug(LOGSERVER) << "R_Server::TranslateOutLoop(): Before encrypt ECI response (len = " << eciResponse.pcount() << ")" << endline;

        crc = cryptoSubsystem->encryptData (dhdeSn, (iosockinet *)&socket, (char *)eciResponse.str(), eciResponse.pcount());
        if (crc != CRYPTO_ERR_OK)
            logf->debug(LOGSERVER) << "Fatal error: unable to encrypt ECI response; error = " << crc << endline;
        logf->debug(LOGSERVER) << "R_Server::TranslateOutLoop(): After encrypt ECI response" << endline;
    }
    else
    {
        areq->print(lexc.Out());
//      areq->print(cout);
//      areq->print(rslerr);
    }

    logf->debug(LOGSERVER) << "R_Server::TranslateOutLoop(): After processing ECI response" << endline;


// Commented out TGM - 12/21/98
//    if (outArgsAllocated)
//        Remember (outArgs);

    lexc.dynamic_request = NULL;
    lexc.done = lexer_context::Ready;   // RFH 24Nov1997

}

void R_Server::FinalizeRequest()
{
    lexc.Out() << endl << flush;
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
ResStatus R_Server::execute(int method, ResList& arglist)
{
    logf->debug(LOGSERVER) << "(R_Server) In execute" << endline;

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
// *      Returns a R_Server
// *                                                                       
// ************************************************************************
R_Server *R_Server::New(RWCString n)
{
    Resource *r= R_Server::rslType.New(n);
    return (R_Server *) r;
}

