// *******************************************************************************
// *
// * Module Name: CryptoLog.cc
// * 
// * Description: 
// *
// * History: Create - CKING 11/13/98
// * 
// * 
// * Copyright 1998 by Destiny Software Corporation.
// * 
// *******************************************************************************
#include "CryptoLog.h"


char logBuffer[2000];



void logMsg (CryptoLogMsgType msgType, char *pstrMessage)
{

    switch (msgType)
    {
        case INFO:
            printf ("%s\n", pstrMessage);
            break;

        case ERR:
            printf ("%s\n", pstrMessage);
            break;

        case DBUG:
        default:
            printf ("%s\n", pstrMessage);
            break;
    }

}    /* end logMsg */


void CryptoSubSystem::logMessage (CryptoLogMsgType msgType, char *pstrMessage)
{
 
    logMsg (msgType, pstrMessage);

}    /* end logMessage */

