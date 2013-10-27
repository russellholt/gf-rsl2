// *****************************************************************************
// *
// * HttpRequest Class
// *
// * Puposes:   To parse and store both the request data and information
// *            derived from it.
// *
// * History: Create - TGM 3/24/97
// * 
// * $Id: HttpRequest.h,v 1.2 1998/12/02 18:43:12 cking Exp $
// * 
// * $Log: HttpRequest.h,v $
// * Revision 1.2  1998/12/02 18:43:12  cking
// * Performance modifications for encryption support for the GF 2.5 release.
// *
// * Revision 1.1  1998/11/17 23:39:41  toddm
// * Initial revision
// *
// * Revision 2.1  1998/06/29 19:48:03  toddm
// * Move all includes outside of exclusion symbol
// *
// * Revision 1.5  1997/10/22 20:16:40  toddm
// * Add Multi-thread support
// *
// * Revision 1.4  1997/09/17 22:46:52  toddm
// * Fix read problem when reading an html file from framework
// *
// * Revision 1.3  1997/09/10 14:43:17  toddm
// * Add Reconnection Support
// *
// * Revision 1.2  1997/08/15 21:27:28  toddm
// * Add Cookie Support
// *
// * Revision 1.1  1997/06/03 18:31:43  toddm
// * Initial revision
// *
// * 
// * Copyright 1997 by Destiny Software Corporation.
// * 
// *******************************************************************************
#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

// *******************
// * System Inlcudes *
// *******************

// ******************
// * Local Includes *
// ******************
#include <rw/cstring.h>
#include "HttpSession.h"

extern "C" 
{
// **************************************************************************
// * The following three are standard headers for SAFs.  They're used to    *
// * get the data structures and prototypes needed to declare and use SAFs. *
// **************************************************************************
#include "base/pblock.h"
#include "base/session.h"
#include "frame/req.h"

// ************************
// * Other NSAPI includes *
// ************************
#include "netsite.h"
#include "base/util.h"       /* is_mozilla, getline */
#include "frame/protocol.h"  /* protocol_start_response */
#include "base/file.h"       /* system_fopenRO */
#include "base/buffer.h"     /* filebuf */
#include "frame/log.h"       /* log_error */
}

// ***********************
// * External Functions **
// ***********************
void MyDebugLog(char *pstrText);
void MyDebugLog (SYS_THREAD threadID, char *pstrMessage);
SYS_NETFD ConnectToHarness (threadControlBlock *pThread);
int DisconnectFromHarness (threadControlBlock *pThread);
int GetAck (threadControlBlock *pThread);

// **************
// * Constants **
// **************
#define LEN_LENGTH          6
#define LEN_LINEFEED        1
#define LEN_TERMINATOR      1
#define NUM_RETRIES         3

#define INITIAL_READ_WAIT   300
#define READ_WAIT           120
#define ACK_WAIT            120

#define ACK                 "ACK"

// *********************
// * HttpRequest Class *
// *********************
class HttpRequest {

private:
    // ************************
    // * Private Data Members *
    // ************************

    // ********************
    // * NSAPI Structures *
    // ********************
    pblock *pBlock;
    Session *pSessn;
    Request *pReqst;

    // *******************
    // * Request Headers *
    // *******************
    RWCString  strUserAgent,   // name of the web browser being used
                strReferer,     // URL of the document containing the requested URL
                strCookies,     // Cookies sent to the server.
                strHeaderBlock; // The entire header block from the user request.

    // ***********************
    // * Request Information *
    // ***********************
    RWCString  strRequest,     // The first line of the Client request
                strMethod,      // The HTTP method used to access the object
                strURI,         // The URI the client has asked for
                strProtocol,    // Identifies the protocol the client is using
                strQuery;       // CGI data string

    // **********************
    // * Thread Information *
    // **********************
    threadControlBlock *pThread;
    
    // ***************************
    // * Private Member Function *
    // ***************************
    void print(void);

    int ReadPostedQuery(netbuf *buf, int iContentLen);

    int GetRequestLine(void);
    int GetRequestHeaders(void);
    int GetRequestData(void);

public:
    // ****************
    // * Constructors *
    // ****************
    HttpRequest(void);

    void Init(threadControlBlock *thread)
    {
        pThread = thread;
        pBlock = thread->pb;
        pSessn = thread->sn;
        pReqst = thread->rq;
    }
        
    // **************************
    // * Public Member Function *
    // **************************
    int GetRequest(void);
    int SendRequestData(void);
};
#endif



