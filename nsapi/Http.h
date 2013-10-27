// *****************************************************************************
// *
// * HTTP Protocol Class
// *
// * History: Create - TGM 3/24/97
// * 
// * $Id: Http.h,v 1.1 1998/11/17 23:39:38 toddm Exp $
// * 
// * $Log: Http.h,v $
// * Revision 1.1  1998/11/17 23:39:38  toddm
// * Initial revision
// *
// * Revision 2.1  1998/06/29 19:47:54  toddm
// * Move all includes outside of exclusion symbol
// *
// * Revision 1.5  1997/10/22 20:16:33  toddm
// * Add Multi-thread support
// *
// * Revision 1.4  1997/09/17 22:46:45  toddm
// * Fix read problem when reading an html file from framework
// *
// * Revision 1.3  1997/09/10 14:43:09  toddm
// * Add Reconnection Support
// *
// * Revision 1.2  1997/08/15 21:27:21  toddm
// * Add Cookie Support
// *
// * Revision 1.1  1997/06/03 18:31:39  toddm
// * Initial revision
// *
// * 
// * Copyright 1997 by Destiny Software Corporation.
// * 
// *******************************************************************************
// *******************
// * System Includes *
// *******************

// ******************
// * Local Includes *
// ******************
#include "HttpSession.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

extern "C" 
{
// **************************************************************************
// * The following three are standard headers for SAFs.  They're used to    *
// * get the data structures and prototypes needed to declare and use SAFs. *
// **************************************************************************
#include "base/pblock.h"
#include "base/session.h"
#include "frame/req.h"
#include "base/crit.h"
}

#ifndef _HTTP_H_
#define _HTTP_H_

void MyDebugLog(char *pstrText);
void MyDebugLog (int threadID, char *pstrMessage);

class HttpRequest;
class HttpResponse;

// **************
// * HTTP Class *
// **************
class HTTP {

private:
    // ************************
    // * Private Data Members *
    // ************************

    // **********************
    // * thread Information *
    // **********************
    threadControlBlock *pThread;
    
    // ********************
    // * NSAPI Structures *
    // ********************
    pblock *pBlock;
    Session *pSessn;
    Request *pReqst;

    // ****************
    // * Data Members *
    // ****************
    HttpRequest     TheRequest;
    HttpResponse    TheResponse;

    // ****************************
    // * Private Member Functions *
    // ****************************
    void  Init(threadControlBlock *thread);

public:

    // ****************
    // * Constructors *
    // ****************
    HTTP(void);

    HTTP(threadControlBlock *thread);

    // **************************
    // * Public Member Fuctions *
    // **************************

    // Main methods
    int ProcessRequest(void);

    int ProcessResponse(void);
};
#endif



