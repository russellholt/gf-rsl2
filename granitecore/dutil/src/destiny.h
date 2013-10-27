// NAME: destiny.h
//
// PURPOSE: General purpose constants, macros, etc.
//
// PROGRAMMING NOTES:
//
// HISTORY:
//      11/03/95 - LJ Shuda - Created for TRUE, FALSE
//
// $Id: destiny.h,v 1.1 1998/11/17 23:48:44 toddm Exp $
//
// Copyright 1995-1997 by Destiny Software Corporation.
//

#ifndef _DESTINY_H_
#define _DESTINY_H_

#ifndef BOOL
#include <bool.h>
#else
typedef int bool;
#endif

#define SUCCEED  0
#define PASS     0
#define FAIL    -1

#define MAXBUFSIZE 512		        // Used for reading socket data.

#define EOS	'\0'

// Logging sub-systems
#define LOGRMG			0x0001		// a - For the RMG protocol
#define LOGSERVER 		0x0002		// b - For Server specific work
#define LOGAPPENV 		0x0004		// c - For Applications/Forms sub-system
#define LOGRESOURCE 	0x0008		// d - For Resources like VHB.
#define LOGRSL 			0x0010		// e - For RSL internals
#define LOGSCRIPT		0x0020		// f - For RSL script writers using Log
#define LOGTRAFFIC		0x0040		// g - protocol traffic
#define LOGWEBCHANNEL	0x0080		// h - Web Channel Logging
#define LOGREGISTRAR   	0x0100		// i - Registrar Logging
#define LOGBROKER  	    0x0200		// j - Broker Logging
#define LOGENCRYPTION	0x0400		// k - Encrytpion Logging
#define LOGSUB12		0x0800		// l - Unused
#define LOGSUB13		0x1000		// m - Unused
#define LOGSUB14		0x2000		// n - Unused
#define LOGSUB15		0x4000		// o - Unused
#define LOGPERFORMANCE	0x8000		// p - Performance Logging

#endif




