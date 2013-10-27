// *******************************************************************************
// *
// * Module Name: HttpRequest.cc
// * 
// * Description: 
// *
// * History: Create - TGM 3/24/97
// * 
// * $Id: HttpRequest.cc,v 1.1 1998/11/17 23:39:12 toddm Exp $
// * 
// * $Log: HttpRequest.cc,v $
// * Revision 1.1  1998/11/17 23:39:12  toddm
// * Initial revision
// *
// * Revision 1.5  1997/10/22 20:16:37  toddm
// * Add Multi-thread support
// *
// * Revision 1.4  1997/09/17 22:46:49  toddm
// * Fix read problem when reading an html file from framework
// *
// * Revision 1.3  1997/09/10 14:43:13  toddm
// * Add Reconnection Support
// *
// * Revision 1.2  1997/08/15 21:27:24  toddm
// * Add Cookie Support
// *
// * Revision 1.1  1997/06/03 18:30:50  toddm
// * Initial revision
// *
// * 
// * Copyright 1997 by Destiny Software Corporation.
// * 
// *******************************************************************************
#include "HttpRequest.h"




// *********************************************************************
// *                                                                    
// * Function: HttpRequest         Constructor
// *                                                                    
// * Description:   Constructor for the HttpRequest Class
// *                                                           
// * Inputs: None                  
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: None
// *                                                                    
// *********************************************************************
HttpRequest::HttpRequest(void) 
{
    pBlock = NULL;
    pSessn = NULL;
    pReqst = NULL;
    pThread = NULL;
}


// *********************************************************************
// *                                                                    
// * Function: print                                             
// *                                                                    
// * Description:   Debug function to print the request and header info. 
// *                                                  
// * Inputs: None 
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: Nothing
// *                                                                    
// *********************************************************************
void HttpRequest::print(void)
{
}


// *********************************************************************
// *                                                                    
// * Function: SendRequestData                                             
// *                                                                    
// * Description:   This function sends the strQuery to the Web Harness
// *                for processing.
// *                                                  
// * Inputs: None                  
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: The number of name/values pairs acutually created.
// *                                                                    
// *********************************************************************
int HttpRequest::SendRequestData(void)
{
#ifdef DEBUGREQ
    MyDebugLog(pThread->threadID, "In SendRequestData\n");
#endif

    char *pstrLength = (char *) MALLOC(sizeof(char) * (LEN_LENGTH + 
                                                       LEN_LINEFEED + 
                                                       LEN_TERMINATOR));

    int iTries;
    for (iTries=0; iTries < NUM_RETRIES; iTries++)
    {
        // **********************************************************
        // * Connect to the harness if we are not already connected *
        // **********************************************************
        if ((pThread->fdWebHarness  = ConnectToHarness(pThread)) == SYS_NET_ERRORFD) 
        {
            continue;
        }    

        // ********************************************
        // * Send the header block to the Web Harness *
        // ********************************************
        char *pstrHeaderBlock;

#ifdef DEBUGREQ
    MyDebugLog(pThread->threadID, "Sending Header Length\n");
#endif
        if (strHeaderBlock.isNull())
        {
            util_sprintf(pstrLength, "0\n");
            net_write(pThread->fdWebHarness, pstrLength, strlen(pstrLength));
        }
        else
        {
            util_sprintf(pstrLength, "%d\n", strHeaderBlock.length());
            net_write(pThread->fdWebHarness, pstrLength, strlen(pstrLength));

            // ********************************************
            // * Wait an acknowledgment from the harness **
            // ********************************************
            if (!GetAck(pThread))
            {
                continue;
            }

#ifdef DEBUGREQ
            MyDebugLog(pThread->threadID, "Sending Headers\n");
#endif
            pstrHeaderBlock = (char *) MALLOC(sizeof(char) * (strHeaderBlock.length() + 
                                                              LEN_LINEFEED + 
                                                              LEN_TERMINATOR));
            util_sprintf(pstrHeaderBlock, "%s\n", (char *) strHeaderBlock.data());
            net_write(pThread->fdWebHarness, pstrHeaderBlock, strlen(pstrHeaderBlock));

            FREE(pstrHeaderBlock);
        }

        // ********************************************
        // * Wait an acknowledgment from the harness **
        // ********************************************
        if (!GetAck(pThread))
        {
            continue;
        }

        // ********************************************
        // * Send the query string to the Web Harness *
        // ********************************************
        char *pstrQuery;

#ifdef DEBUGREQ
    MyDebugLog(pThread->threadID, "Sending Query String Length\n");
#endif
        if (strQuery.isNull())
        {
            util_sprintf(pstrLength, "0\n");
            net_write(pThread->fdWebHarness, pstrLength, strlen(pstrLength));
        }
        else
        {
            util_sprintf(pstrLength, "%d\n", strQuery.length());
            net_write(pThread->fdWebHarness, pstrLength, strlen(pstrLength));

            // ********************************************
            // * Wait an acknowledgment from the harness **
            // ********************************************
            if (!GetAck(pThread))
            {
                DisconnectFromHarness(pThread);
                continue;
            }

#ifdef DEBUGREQ
            MyDebugLog(pThread->threadID, "Sending Query String\n");
#endif
            pstrQuery = (char *) MALLOC(sizeof(char) * (strQuery.length() + 
                                                        LEN_LINEFEED + 
                                                        LEN_TERMINATOR));
            util_sprintf(pstrQuery, "%s\n", (char *) strQuery.data());
            net_write(pThread->fdWebHarness, pstrQuery, strlen(pstrQuery));

            FREE(pstrQuery);
        }

        // ********************************************
        // * Wait an acknowledgment from the harness **
        // ********************************************
        if (!GetAck(pThread))
        {
            continue;
        }

        break;
    }


    FREE(pstrLength);

    if (iTries >= NUM_RETRIES)
    {
#ifdef DEBUGREQ
        MyDebugLog(pThread->threadID, "Unable to communicate with harness\n");
#endif

        protocol_status(pSessn, pReqst, PROTOCOL_SERVER_ERROR, "Unable to communicate with harness.");
        log_error(LOG_WARN, "SendRequestData", pSessn, pReqst, "Unable to communicate with harness.");
        return REQ_ABORTED;
    }

    return REQ_PROCEED;
}


// *********************************************************************
// *                                                                    
// * Function: GetRequest                                             
// *                                                                    
// * Description:   Function is used to read the various pieces of the  
// *                request into the respective data members of the     
// *                request class.                                      
// *                                                  
// * Inputs: None                  
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns:   REQ_PROCEED if successful
// *            REQ_ABORTED if fail
// *                                                                    
// *********************************************************************
int HttpRequest::GetRequest(void)
{
    int iStat;

#ifdef DEBUGREQ
    MyDebugLog(pThread->threadID, "In GetRequest\n");
#endif

    if ((iStat = GetRequestLine()) == REQ_PROCEED)
        if ((iStat = GetRequestHeaders()) == REQ_PROCEED)
            iStat = GetRequestData();

    return iStat;
}


// *********************************************************************
// *                                                                    
// * Function: GetRequestLine                                             
// *                                                                    
// * Description:   Function used to read the request line into it's
// *                various pieces.  These are the Method, URI, and
// *                protocol.  The pieces are stored into the respective 
// *                data members of the request class.                                      
// *                                                  
// * Inputs: None                  
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: REQ_PROCEED
// *                                                                    
// *********************************************************************
int HttpRequest::GetRequestLine(void)
{

#ifdef DEBUGREQ
    MyDebugLog(pThread->threadID, "In GetRequestLine\n");
#endif

    // *************************************
    // * Get the first line of the request *
    // *************************************
    strRequest = pblock_findval("clf-request", pReqst->reqpb);

    // *************************************************
    // * Get the HTTP method used to access the object *
    // *************************************************
    strMethod = pblock_findval("method", pReqst->reqpb);

    // ****************************************
    // * Get the URI the client has asked for *
    // ****************************************
    strURI = pblock_findval("uri", pReqst->reqpb);

    // ****************************************
    // * Get the protocol the client is using *
    // ****************************************
    strProtocol = pblock_findval("protocol", pReqst->reqpb);

    return REQ_PROCEED;
}


// *********************************************************************
// *                                                                    
// * Function: GetRequstHeaders                                             
// *                                                                    
// * Description:   Function used to read the request headers.  The 
// *                headers are stored into the respective data members
// *                of the request class.                                      
// *                                                  
// * Inputs: None                  
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: REQ_PROCEED
// *                                                                    
// *********************************************************************
int HttpRequest::GetRequestHeaders(void)
{
    char *pstrUserAgent;
    char *pstrReferer;
    char *pstrCookies;

    int iHeader=FALSE;

#ifdef DEBUGREQ
    MyDebugLog(pThread->threadID, "In GetRequestHeaders\n");
#endif

    strHeaderBlock = "";

    // ********************************************
    // * Get the User-Agent from the header block *
    // ********************************************
    if (request_header("user-agent", &pstrUserAgent, pSessn, pReqst) == REQ_PROCEED)
    {
        if (pstrUserAgent)
        {
            strUserAgent = "USER_AGENT::";
            strUserAgent += pstrUserAgent;

            strHeaderBlock += strUserAgent;

            iHeader = TRUE;
        }
    }

    // *********************************************
    // * Get the Referer from the header plock     *
    // *********************************************
    if (request_header("referer", &pstrReferer, pSessn, pReqst) == REQ_PROCEED)
    {
        if (pstrReferer)
        {
            if (iHeader)
                strReferer = "||REFERER::";
            else
                strReferer = "REFERER::";
            strReferer += pstrReferer;

            strHeaderBlock += strReferer;

            iHeader = TRUE;
        }
    }

    // *********************************************
    // * Get the Cookies from the header plock     *
    // *********************************************
    if (request_header("cookie", &pstrCookies, pSessn, pReqst) == REQ_PROCEED)
    {
        if (pstrCookies)
        {
            if (iHeader)
                strCookies = "||COOKIE::";
            else
                strCookies = "COOKIE::";
            strCookies += pstrCookies;

            strHeaderBlock += strCookies;
        }
    }

#ifdef DEBUGREQ
    MyDebugLog(pThread->threadID, "Header Block:\n");
    MyDebugLog((char *)strHeaderBlock.data());
    if (!(strHeaderBlock.isNull()))
        MyDebugLog("\n");
#endif

    return REQ_PROCEED;
}    


// *********************************************************************
// *                                                                    
// * Function: ReadPostedQuery                                             
// *                                                                    
// * Description:   This function reads in data from the specified netbuf, 
// *                into a string in query string format, up to iContentLen 
// *                characters.  It returns the number of characters actually 
// *                read.
// *                                                  
// * Inputs: None                  
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: The number of characters acutually read
// *                                                                    
// *********************************************************************
int HttpRequest::ReadPostedQuery(netbuf *buf, int iContentLen)
{
    int i=0;                   // index into pstrQuery
    int iChar=1;               // char read in from netbuf
    char *pstrQuery;           // ptr to the actual query string to work on 

#ifdef DEBUGREQ
    MyDebugLog(pThread->threadID, "In ReadPostedQuery\n");
#endif

    pstrQuery = (char *) MALLOC(sizeof(char)*(iContentLen+1));

    // ******************************************************************
    // * Loop through reading a character and writing it to strQuery,   *
    // * until either len characters have been read, there's no more    *
    // * input, or there's an IO error.                                 *
    // ******************************************************************
    while ((iContentLen) && (iChar != IO_EOF))
    {
        iChar = netbuf_getc(buf);
        
        // ******************************
        // * check for error in reading *
        // ******************************
        if(iChar == IO_ERROR)
        {
            break;
        }

        pstrQuery[i++] = iChar;
        iContentLen--;
    }

    pstrQuery[i] = '\0'; 
    strQuery = pstrQuery;

#ifdef DEBUGREQ
    MyDebugLog(pstrQuery);
    if (pstrQuery != NULL)
        MyDebugLog("\n");
#endif

    FREE(pstrQuery);

    return(i);
}


// *********************************************************************
// *                                                                    
// * Function: GetRequestData                                             
// *                                                                    
// * Description:   This function get the query information from the request
// *                If the method is GET, we can get the name=value pairs 
// *                from the query string, but if it's POST, then we need 
// *                to read in all the input from the network and stuff 
// *                it into a string. If it's neither we complain 
// *                vigorously and exit.
// *                                                           
// * Inputs: None                  
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns:   REQ_PROCEED if successful
// *            REQ_ABORTED if fail
// *                                                                    
// *********************************************************************
int HttpRequest::GetRequestData(void)
{
    char *pstrContentType=NULL;   // ptr to the Content-type string in pblock
    char *pstrContentLen=NULL;    // ptr to Content-length string in pblock
    int iContentLen;         // content length
    int iQueryLen;           // number of chars read from net_buf

#ifdef DEBUGREQ
    MyDebugLog(pThread->threadID, "In GetRequestData\n");
#endif

    // ***********************************************************************
    // * If the method is GET, we can get the name=value pairs from the      *
    // * query string, but if it's POST, then we need to read in all the     *
    // * input from the network and stuff it into a string. If it's neither, *
    // * we complain vigorously and exit.                                    *
    // ***********************************************************************
    if (strMethod == "POST")
    {
#ifdef DEBUGREQ
        MyDebugLog(pThread->threadID, "Method is a POST\n");
#endif

        // *******************************************************
        // * if there's no content-length, it's a bogus request  *
        // *******************************************************
        if (request_header("content-length", &pstrContentLen, pSessn, pReqst) == REQ_ABORTED)
        {
            protocol_status(pSessn, pReqst, PROTOCOL_FORBIDDEN, "No content-length.");
            log_error(LOG_WARN, "GetRequestData", pSessn, pReqst, "No content-length.");
            return REQ_ABORTED;
        }
        
        if (pstrContentLen == NULL)
        {
            protocol_status(pSessn, pReqst, PROTOCOL_FORBIDDEN, "No content-length.");
            log_error(LOG_WARN, "GetRequestData", pSessn, pReqst, "No content-length.");
            return REQ_ABORTED;
        }

        iContentLen = atoi(pstrContentLen);

        // ****************************************************************
        // * if the content length is 0 or negative, it's a bogus request *
        // ****************************************************************
        if(iContentLen < 1)
        {
            protocol_status(pSessn, pReqst, PROTOCOL_FORBIDDEN, "Content length <=0.");
            log_error(LOG_WARN, "GetRequestData", pSessn, pReqst, "Content length <=0.");
            return REQ_ABORTED;
        }

        // *********************************************************
        // * if the content-type isn't right, it's a bogus request *
        // *********************************************************
        request_header("content-type", &pstrContentType, pSessn, pReqst);

        if(strcmp(pstrContentType, "application/x-www-form-urlencoded") != 0)
        {
            protocol_status(pSessn, pReqst, PROTOCOL_FORBIDDEN, "Content wrong type.");
            log_error(LOG_WARN, "GetRequestData", pSessn, pReqst, "Content wrong type.");
            return REQ_ABORTED;
        }

        // ***************************************************************
        // * Allocate space to put the query string after we read it in  *
        // ***************************************************************
        iQueryLen = ReadPostedQuery(pSessn->inbuf, iContentLen);

        // ****************************************************************
        // * if the amount of data we read in != content-length, then we  *
        // * will consider the request bogus, although in theory we could *
        // * keep going with the actual length                            *
        // ****************************************************************
        if (iQueryLen != iContentLen)
        {
            protocol_status(pSessn, pReqst, PROTOCOL_FORBIDDEN, "Bad content length.");
            log_error(LOG_WARN, "GetRequestData", pSessn, pReqst, "Bad content length.");
            return REQ_ABORTED;
        }
    } 
    
    else if (strMethod == "GET")
    {
#ifdef DEBUGREQ
        MyDebugLog(pThread->threadID, "Method is a GET\n");
#endif        

        // *****************************************************************
        // * make a copy of the query string to work on, since we're going *
        // * to be chopping it up most mercilessly                         *
        // *****************************************************************
        if(pblock_find("query", pReqst->reqpb))
        {
            strQuery = pblock_findval("query", pReqst->reqpb);
#ifdef DEBUGREQ
            MyDebugLog(pThread->threadID, "Here is the Query: ");
            MyDebugLog((char *)strQuery.data());
            if (!(strQuery.isNull()))
                MyDebugLog("\n");
#endif        
        } 

        // *************************************************
        // * if there's no query string, nothing we can do *
        // *************************************************
        else
        {
            strQuery = "";
        }
    } 

    else
    {
        protocol_status(pSessn, pReqst, PROTOCOL_FORBIDDEN, "Invalid method.");
        log_error(LOG_WARN, "GetRequestData", pSessn, pReqst, "Invalid method.");
        return REQ_ABORTED;
    }

    return REQ_PROCEED;
}