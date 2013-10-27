# $Id: makefile,v 1.1 1998/11/17 23:07:53 toddm Exp $

include $(MAKEFILE_LOCAL)

INCLUDE  = -I. -I$(GRANITE)/granitecore/include -I$(RW_INC) -I$(SOCK_INC) -I$(LOCAL_INC) -I$(DCORE)
CPPFLAGS = $(DBG_FLAG) $(FPIC) $(INCLUDE)
LIBS     = $(SOCKXXLIB) $(RWLIB)
LDDFLAGS = -L. -L$(GRANITE)/granitecore -L$(SOCK_DIR) -L$(RWDIR) $(LIBS)

PACKAGE = langutils
LIBSONAME = ../../lib$(PACKAGE).so
LIBANAME = ../../lib$(PACKAGE).a

OBJ_FILES = R_RSL_System.o RslObjectParser.o

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
