// *******************************************************************************
// *
// * Module Name: HttpRequest.cc
// * 
// * Description: 
// *
// * History: Create - TGM 3/24/97
// * 
// * $Id: HttpResponse.cc,v 1.2 1998/12/21 19:33:59 cking Exp $
// * 
// * $Log: HttpResponse.cc,v $
// * Revision 1.2  1998/12/21 19:33:59  cking
// * Patch implements a configurable receive timeout value.
// *
// * Revision 1.1  1998/11/17 23:39:15  toddm
// * Initial revision
// *
// * Revision 1.5  1997/10/22 20:16:44  toddm
// * Add Multi-thread support
// *
// * Revision 1.4  1997/09/17 22:46:56  toddm
// * Fix read problem when reading an html file from framework
// *
// * Revision 1.3  1997/09/10 14:43:21  toddm
// * Add Reconnection Support
// *
// * Revision 1.2  1997/08/15 21:27:31  toddm
// * Add Cookie Support
// *
// * Revision 1.1  1997/06/03 18:30:54  toddm
// * Initial revision
// *
// * 
// * Copyright 1997 by Destiny Software Corporation.
// * 
// *******************************************************************************
#include "HttpResponse.h"



// *********************************************************************
// *                                                                    
// * Function: HttpResponse         Constructor
// *                                                                    
// * Description:   Constructor for the HttpResponse Class
// *                                                           
// * Inputs: None                  
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: None
// *                                                                    
// *********************************************************************
HttpResponse::HttpResponse(void)
{
    pThread = NULL;
    pBlock = NULL;
    pSessn = NULL;
    pReqst = NULL;

    status_code = 0;
    bytes_sent = 0;
}


// *********************************************************************
// *                                                                    
// * Function: print                                             
// *                                                                    
// * Description:   Debug function to print the response and header info. 
// *                                                  
// * Inputs: None 
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: Nothing
// *                                                                    
// *********************************************************************
void HttpResponse::print(void)
{
}


// *********************************************************************
// *                                                                    
// * Function: GetResponseData                                             
// *                                                                    
// * Description:   This function get the response from the Web Harness
// *                                                  
// * Inputs: None                  
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: The number of name/values pairs a
// *                                                                    
// *********************************************************************
int HttpResponse::GetResponseData(void)
{
    int iBytesRead;

#ifdef DEBUGRES
    MyDebugLog(pThread->threadID, "In GetResponseData\n");
#endif

    // **********************************************************
    // * Connect to the harness if we are not already connected *
    // **********************************************************
    if ((pThread->fdWebHarness = ConnectToHarness(pThread)) < 0) 
    {
#ifdef DEBUGREQ
        MyDebugLog(pThread->threadID, "Unable to connect to harness\n");
#endif

        protocol_status(pSessn, pReqst, PROTOCOL_SERVER_ERROR, "Unable to open Harness connection.");
        log_error(LOG_WARN, "GetResponseData", pSessn, pReqst, "Unable to open Harness connection.");
        return REQ_ABORTED;
    }    

    char *pstrLength = (char *) MALLOC(sizeof(char) * (LEN_LENGTH + 
                                                       LEN_LINEFEED + 
                                                       LEN_TERMINATOR));
    char *pstrChar = (char *) MALLOC(sizeof(char));
    *pstrLength = '\0';
    *pstrChar = '\0';

    // ********************************************
    // * First read a length from the socket so  **
    // * that we can determine how long the HTML **
    // * response will be                        **
    // ********************************************
#ifdef DEBUGRES
    MyDebugLog(pThread->threadID, "Going to read response length\n");
#endif

    int iTotalBytesToRead = 1;
    int iIndex = 0;

    while (*pstrChar != '\n')
    {
    
#ifdef DEBUGRES
        MyDebugLog(pThread->threadID, "Reading response length\n");
#endif

        if ((iBytesRead = net_read(pThread->fdWebHarness, pstrChar, iTotalBytesToRead, iReceiveTimeOut * 2)) < 0)
        {
            DisconnectFromHarness(pThread);
            FREE(pstrLength);
            FREE(pstrChar);

            protocol_status(pSessn, pReqst, PROTOCOL_SERVER_ERROR, "Unable to read response length.");
            log_error(LOG_WARN, "GetResponseData", pSessn, pReqst, "Unable to read response length.");
            return REQ_ABORTED;
        }

        pstrLength[iIndex++] = *pstrChar;
    }

    pstrLength[iIndex] = '\0';

#ifdef DEBUGRES
    MyDebugLog(pThread->threadID, "Got response length: ");
    MyDebugLog(pstrLength);
#endif

    // **************************
    // * Check for valid length *
    // **************************
    iTotalBytesToRead = atoi(pstrLength);    

    // ******************************************************  
    // * if the length is -1 then we have a read error.     *
    // ******************************************************  
    if (iTotalBytesToRead < 0)
    {
        DisconnectFromHarness(pThread);
        FREE(pstrLength);
        FREE(pstrChar);

        protocol_status(pSessn, pReqst, PROTOCOL_SERVER_ERROR, "Read an invalid length.");
        log_error(LOG_WARN, "GetResponseData", pSessn, pReqst, "Read an invalid length.");
        return REQ_ABORTED;
    }

    // ***********************************************
    // * Using the length that was sent to us       **
    // * from the harness, read the HTML response   **
    // * from the socket until we get the specified **
    // * number of bytes.                           **
    // ***********************************************
    char *pstrHtmlResponse = (char *) MALLOC(sizeof(char) * (iTotalBytesToRead + 
                                                             LEN_TERMINATOR));
    *pstrHtmlResponse = '\0';

    // ******************************************************
    // * Read the specified number of bytes from the socket *
    // ******************************************************
#ifdef DEBUGRES
    MyDebugLog(pThread->threadID, "Reading response!\n");
#endif

    int i;
    iBytesRead=0;
    for(i=0; i<iTotalBytesToRead; i+=iBytesRead)
    {
        if ((iBytesRead = net_read(pThread->fdWebHarness, pstrHtmlResponse, iTotalBytesToRead, iReceiveTimeOut)) < 0)
        {
            DisconnectFromHarness(pThread);
            FREE(pstrLength);
            FREE(pstrChar);
            FREE(pstrHtmlResponse);

            protocol_status(pSessn, pReqst, PROTOCOL_SERVER_ERROR, "Unable to read HTML response.");
            log_error(LOG_WARN, "GetResponseData", pSessn, pReqst, "Unable to read HTML response.");
            return REQ_ABORTED;
        }

        pstrHtmlResponse[iBytesRead] = '\0';
        strResponseLine += pstrHtmlResponse;

#ifdef DEBUGRES
        MyDebugLog(pstrHtmlResponse);
        MyDebugLog("\n");
#endif

    }    

    FREE(pstrLength);
    FREE(pstrChar);
    FREE(pstrHtmlResponse);

    return REQ_PROCEED;
}


// *********************************************************************
// *                                                                    
// * Function: SendResponse
// *                                                                    
// * Description:   This function sends the reponse back to the client.
// *                                                           
// * Inputs: None                  
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns:   REQ_PROCEED if successful
// *            REQ_EXIT if fail
// *                                                                    
// *********************************************************************
int HttpResponse::SendResponse(void)
{
    int iStat;

#ifdef DEBUGRES
    MyDebugLog(pThread->threadID, "In SendResponse\n");
#endif

    if ((iStat = SendResponseHeaders()) == REQ_PROCEED)
        iStat = SendResponseData();

    return iStat;
}    


// *********************************************************************
// *                                                                    
// * Function: SendResponseHeaders                                             
// *                                                                    
// * Description:   This function creates and sends the response headers
// *                to the client.
// *                                                           
// * Inputs: None                  
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns:   REQ_PROCEED if successful
// *                                                                    
// *********************************************************************
int HttpResponse::SendResponseHeaders(void)
{

#ifdef DEBUGRES
    MyDebugLog(pThread->threadID, "In SendResponseHeaders\n");
#endif

    // ********************************************************************
    // * This is actually not necessary in a normal server configuration, *
    // * since the dummy file with it's .dummy extension will default to  *
    // * text/plain, but the default MIME type could have been changed,   *
    // * or this program could be modified to output a different type.    *
    // ********************************************************************
    RWCString  strContentType = pblock_findval("content-type", pReqst->srvhdrs);

    param_free(pblock_remove("content-type", pReqst->srvhdrs));

    if (strContentType == "magnus-internal/destiny-ofx")
       pblock_nvinsert("content-type", "application/x-ofx", pReqst->srvhdrs);
    else if (strContentType == "magnus-internal/destiny-qif")
       pblock_nvinsert("content-type", "application/qif", pReqst->srvhdrs);
    else
       pblock_nvinsert("content-type", "text/html", pReqst->srvhdrs);

    // **********************************************************************
    // * These two lines send the headers to the client, and set everything *
    // * up so successive net_write()s will work properly.                  *
    // **********************************************************************
    protocol_status(pSessn, pReqst, PROTOCOL_OK, NULL);
    protocol_start_response(pSessn, pReqst);

    return REQ_PROCEED;
}


// *********************************************************************
// *                                                                    
// * Function: SendResponseData
// *                                                                    
// * Description:   This function sends the reponse data back to the client.
// *                                                           
// * Inputs: None                  
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns:   REQ_PROCEED if successful
// *            REQ_EXIT if fail
// *                                                                    
// *********************************************************************
int HttpResponse::SendResponseData(void)
{

#ifdef DEBUGRES
    MyDebugLog(pThread->threadID, "In SendResponseData\n");
#endif

    // ************************************************************************
    // * netwrite(sn->csd, <string to send>, <length of string>) is the other *
    // * half of sending data to the client. When you're done sending, just   *
    // * end the function; you don't need to do anything to shut down the     *
    // * connection.                                                          *
    // *                                                                      *
    // * If the client disconnects before we're done, then net_write() will   *
    // * return IO_ERROR, which we will interpret as a sign to give up and    *
    // * go away. Before doing so, it is imperative that we clean up any      *
    // * allocated memory.                                                    *
    // ************************************************************************

    if(net_write(pSessn->csd, (char *)strResponseLine.data(), strResponseLine.length()) == IO_ERROR)
    {
        return REQ_EXIT;
    }

    return REQ_PROCEED;
}    
