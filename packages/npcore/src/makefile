# $Id: makefile,v 1.1 1998/11/17 23:08:58 toddm Exp $

include $(MAKEFILE_LOCAL)

INCLUDE  = -I. -I$(GRANITE)/granitecore/include -I$(RW_INC) -I$(SOCK_INC) -I$(LOCAL_INC) -I$(CRYPTO) -I$(BSAFE_INC)
CPPFLAGS = $(DBG_FLAG) $(FPIC) $(INCLUDE)
LIBS = $(SOCKXXLIB) $(LIBBSAFE) -lencrypt $(RWLIB) $(LIBSOCKET)
LDDFLAGS = -L. -L$(GRANITE)/granitecore -L$(RWDIR) -L$(SOCK_DIR) -L$(CRYPTO) -L$(BSAFE_DIR) $(LIBS)

PACKAGE = npcore
LIBSONAME = ../../lib$(PACKAGE).so
LIBANAME = ../../lib$(PACKAGE).a

OBJ_FILES = R_NPServer.o

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
