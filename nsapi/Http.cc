// *******************************************************************************
// *
// * Module Name: HTTP.cc
// * 
// * Description: 
// *
// * History: Create - TGM 3/24/97
// * 
// * $Id: Http.cc,v 1.1 1998/11/17 23:39:09 toddm Exp $
// * 
// * $Log: Http.cc,v $
// * Revision 1.1  1998/11/17 23:39:09  toddm
// * Initial revision
// *
// * Revision 1.6  1998/01/14 18:23:56  toddm
// * Fix Multi thread support
// *
// * Revision 1.5  1997/10/22 20:16:29  toddm
// * Add Multi-thread support
// *
// * Revision 1.4  1997/09/17 22:46:42  toddm
// * Fix read problem when reading an html file from framework
// *
// * Revision 1.3  1997/09/10 14:43:06  toddm
// * Add Reconnection Support
// *
// * Revision 1.2  1997/08/15 21:27:17  toddm
// * Add Cookie Support
// *
// * Revision 1.1  1997/06/03 18:30:47  toddm
// * Initial revision
// *
// * 
// * Copyright 1997 by Destiny Software Corporation.
// * 
// *******************************************************************************
#include "Http.h"



void HTTP::Init(threadControlBlock *thread)
{
    pThread = thread;

    TheRequest.Init(thread);
    TheResponse.Init(thread);
}

HTTP::HTTP(void)
{
    pThread=NULL;
}

HTTP::HTTP(threadControlBlock *thread)
{
    Init(thread);
}



// *************************************************************
// * ProcessRequest                                            *
// *************************************************************
int HTTP::ProcessRequest(void)
{
    int iStat;

#ifdef DEBUGHTTP
    MyDebugLog(pThread->threadID, "In ProcessRequest\n");
#endif

    if ((iStat = TheRequest.GetRequest()) != REQ_PROCEED)
        return(iStat);
        

    if ((iStat = TheRequest.SendRequestData()) != REQ_PROCEED)
        return(iStat);

    return REQ_PROCEED;
}


// *************************************************************
// * Send the response headers, not the content, to the client *
// * This is a linked list of Strings                          *
// *************************************************************
int HTTP::ProcessResponse(void)
{
    int iStat;

#ifdef DEBUGHTTP
    MyDebugLog(pThread->threadID, "In ProcessResponse\n");
#endif

    if ((iStat = TheResponse.GetResponseData()) != REQ_PROCEED)
        return(iStat);

    if ((iStat = TheResponse.SendResponse()) != REQ_PROCEED)
        return(iStat);

    return REQ_PROCEED;
}

