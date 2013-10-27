// *****************************************************************************
// *
// * RSA BSAFE Diffie-Hellman Cryptography Technique Classes
// *
// * History: Create - CKING 11/13/98
// * 
// * 
// * Copyright 1998 by Destiny Software Corporation.
// * 
// *******************************************************************************
#ifndef _CRYPTO_LOG_H_
#define _CRYPTO_LOG_H_

#include "CryptoSubSystem.h"

extern char logBuffer[2000];
extern void logMsg (CryptoLogMsgType msgType, char *pstrMessage);

#endif
