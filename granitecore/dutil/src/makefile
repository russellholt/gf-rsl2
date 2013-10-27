# tech/util/makefile
#
# 6/10/98 CP SUNWspro mods
# - replaced -shared with $(SHARE) add SHARE to makefile.local.xxx to 
#   define SHARE as -G or -shared depending on the compiler
# - removed Socket.o from build
# - Replace -g in CPPFLAGS with $(DBG_FLAG)
# - Add DBG_FLAG = -g into makefile.local.xxx 
# - Change AR in makefile.local.xxx 
#	 define AR = $(CC) -xar -o for SUN
#	 define AR = ar cr for gnu and cyg
#
# 5/01/98 CP SUNWspro mods
# - expanded gmake clean to remove Templates.DB files
#
# $Id: makefile,v 1.1 1998/11/17 23:49:18 toddm Exp $

include $(MAKEFILE_LOCAL)

INCLUDE  = -I. -I$(GRANITE)/granitecore/include -I$(RW_INC) -I$(LOCAL_INC) -I$(DCORE)
CPPFLAGS = $(DBG_FLAG) $(FPIC) $(INCLUDE)
LIBS     = $(RWLIB) -lD
LDDFLAGS = -L. -L$(GRANITE)/granitecore -L$(RWDIR) -L$(DCORE) $(LIBS)

PACKAGE = dutil
LIBSONAME = ../../lib$(PACKAGE).so
LIBANAME = ../../lib$(PACKAGE).a

UTILS = StatMessages.o drwcstring.o slog.o rw_utils.o \
	SharedLibrary.o DirContents.o R_DateTime.o R_File.o R_Log.o \
	R_Table.o ResStream.o

TM_STUFF = timeout_manager.o R_TimeoutManager.o
OBJ_FILES = $(UTILS) $(TM_STUFF)

# **************************************************************** #

system: $(LIBSONAME)
static: $(LIBANAME)

# **************************
# * Build the util library *
# **************************
$(LIBSONAME): $(OBJ_FILES)
	$(CCC) $(LDDFLAGS) $(SHARE) $(OBJ_FILES) -o $@

$(LIBANAME): $(OBJ_FILES)
	$(AR) $@ $?

headers:
	$(CP) *.h $(GRANITE)/granitecore/include
	
clean:
	$(RM) *.o $(LIBANAME) $(LIBSONAME) 
	$(RM) -rf Templates.DB
