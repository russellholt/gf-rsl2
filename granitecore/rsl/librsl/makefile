# tech/rsl/librsl/makefile
# master makefile for librsl
#
# 4/28/98 CP SUNWspro mods
# - replaced -shared with -G on librsl $(CCC) build line
# - added -I/usr/local/include to INCLUDE symbol to find <FlexLexer.h>
#
# 4/29/98 MB SUNWspro mods
# - changed librsl.a build cmd line to handle template instantiations.  
#
# 5/01/98 CP SUNWspro mods
# - expanded gmake clean to remove Templates.DB files
# - added -I$(RW) to INCLUDE path
# - added $(RWLIB) to LDDFLAGS
#
# 6/11/98 TM SUNWspro mods
# - Change -g in CPPFLAGS to $(DBG_FLAG)
# - replaced -shared and -G with $(SHARE)
# - Redefine AR in the makefile.local.xxx
#
# $Id: makefile,v 1.2 1998/12/14 15:28:09 holtrf Exp $

include $(MAKEFILE_LOCAL)
include ../rsl_debug_flags
include $(DCORE)/d_debug

INCLUDE  = -I. -I$(GRANITE)/granitecore/include -I$(RW_INC) -I$(SOCK_INC) -I$(LOCAL_INC) -I$(DCORE) -I$(CRYPTO) -I$(BSAFE_INC)
CPPFLAGS = $(DBG_FLAG) $(FPIC) $(INCLUDE) $(RSL_DEBUG_FLAGS)
LIBS     = -ldutil $(LIBBSAFE) $(SOCKXXLIB) $(RWLIB) -lD
LDDFLAGS = -L. -L$(GRANITE)/granitecore -L$(RWDIR) -L$(BSAFE_DIR) -L$(SOCK_DIR) -L$(CRYPTO) -L$(DCORE) $(LIBS)

PACKAGE = rsl
LIBSONAME = ../../lib$(PACKAGE).so
LIBANAME = ../../lib$(PACKAGE).a

RSL_TYPES = R_String.o R_Integer.o R_Boolean.o R_System.o R_List.o \
	R_Queue.o R_Status.o rslServer.o R_Server.o

RSL_SYS = Resource.o res_param.o b.o ResContext.o rslMethod.o \
	rslEvents.o Request.o iteration.o valdecl.o \
	R__transfer_.o killevents.o

OBJ_FILES = $(RSL_SYS) $(RSL_TYPES)


# **************************************************************** #

system: $(LIBSONAME)
static: $(LIBANAME)

# *************************
# * Build the rsl library *
# *************************
$(LIBANAME): $(OBJ_FILES)
	$(AR) $@ $?

$(LIBSONAME): $(OBJ_FILES)
	$(CCC) $(LDDFLAGS) $(SHARE) $(OBJ_FILES) -o $@

headers:
	$(CP) *.h $(GRANITE)/granitecore/include

clean:
	$(RM) *.o $(LIBANAME) $(LIBSONAME) 
	$(RM) -rf Templates.DB
