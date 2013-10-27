# makefile.local.gnu
# Sets environment variables, to be included by sub-makefiles
#
# $Id: makefile.local.gnu,v 1.1 1998/11/17 23:32:58 toddm Exp $
# Copyright (c) 1997, Destiny Software Corporation


# ********************
# * various commands *
# ********************
CCC = g++
CC = g++
LD = ld
MAKE = gmake
FLEX = flex++
BISON = bison
AR = ar cr
MV = mv -f
CP = cp -f
RM = rm -f
CD = cd

# ******************
# * Compiler Flags *
# ******************
DBG_FLAG = -g

# compiler option to generate position independent code
FPIC = -fPIC

# compiler option to generate shared libraries
SHARE = -shared

# specifes library search dirs to runtime linker
RPATH = -R

USE_EXCEPTIONS = -fhandle-exceptions

# ************************************
# * directory trees for installation *
# ************************************
TOOLS= /dest/tools

# ************************************************
# * Location of Rogue Wave Tools.h++ (librwtool) *
# ************************************************
RW_INC = /usr/local/lib/rogue
RWDIR = /usr/local/lib/rogue/lib

# link command for Rogue Wave Tools.h++ shared library (.so)
RWLIB = -lrwtool

# pathname to Rogue Wave Tools.h++ static library (.a)
SRWLIB = -lrwtool.a

# **********************************
# * Location of the BSAFE toolkit  *
# **********************************
BSAFE_INC = /sw/bsafe40/solar26/library/include
BSAFE_DIR = /sw/bsafe40/solar26/library/lib
 
LIBBSAFE = -lbsafe
 
# ******************************
# * Location of Socket++ v1.10 *
# ******************************
SOCK_INC = /dest/tools/socket++-1.10
SOCK_DIR = $(ROOT)/tools/socket++-1.10

# link command for socket++ library (.so)
SOCKXXLIB = -lsocket++

# pathname to Socket++ static library (.a)
SSOCKXXLIB = -lsocket++.a

# *****************************
# * Standard socket libraries *
# *****************************
LIBSOCKET = -lsocket -lnsl

# **************************
# * User local include dir *
# **************************
LOCAL_INC = /usr/local/include

# ******************************************
# * location of Purify headers and library *
# * comment out if not using Purify.       *
# ******************************************
#export PURIFY_DIR = `purify -print-home-dir`







