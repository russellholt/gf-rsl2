# um
# $Id: makefile,v 1.1 1998/11/17 23:04:37 toddm Exp $

include $(MAKEFILE_LOCAL)


INCLUDE  = -I. -I$(GRANITE)/granitecore/include -I$(RW_INC) -I$(LOCAL_INC) -I$(DCORE)
CPPFLAGS = $(DBG_FLAG) $(FPIC) $(INCLUDE)
LIBS     = $(RWLIB) -lD
LDDFLAGS = -L. -L$(GRANITE)/granitecore -L$(RWDIR) -L$(DCORE) $(LIBS) 

PACKAGE = fileutils
LIBSONAME = ../../lib$(PACKAGE).so
LIBANAME = ../../lib$(PACKAGE).a

OBJ_FILES = R_FileInfo.o R_Folder.o RDirectory.o


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

clean:
	$(RM) *.o $(LIBANAME) $(LIBSONAME)
	$(RM) -rf Templates.DB
