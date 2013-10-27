# this makefile doesn't build a library, only the .o's.
# use ../makefile to build the lib.
# $Id: makefile,v 1.1 1998/11/17 23:30:45 toddm Exp $

include $(MAKEFILE_LOCAL)

INCLUDE  = -I. -I$(GRANITE)/granitecore/include -I$(GRANITE)/packages/remote/src -I$(RW_INC) -I$(SOCK_INC) -I$(LOCAL_INC) -I$(DCORE) -I$(CRYPTO) -I$(BSAFE_INC)
CPPFLAGS = $(DBG_FLAG) $(FPIC) $(INCLUDE)
LIBS     = -lremote $(LIBBSAFE) -lencrypt $(SOCKXXLIB) $(RWLIB) $(LIBSOCKET)
LDDFLAGS = -L. -L$(GRANITE)/granitecore -L$(RWDIR) -L$(SOCK_DIR) -L$(CRYPTO) -L$(BSAFE_DIR) $(LIBS)

PACKAGE = web
LIBSONAME = ../../lib$(PACKAGE).so
LIBANAME = ../../lib$(PACKAGE).a

OBJ_FILES = R_WebServer.o transHTTP.o TListSub.o

# **************************************************************** #

system: $(LIBSONAME)
static: $(LIBANAME)

# *************************
# * Build the web library *
# *************************
$(LIBANAME): $(OBJ_FILES)
	$(AR) $@ $?

$(LIBSONAME): $(OBJ_FILES)
	$(CCC) $(LDDFLAGS) $(SHARE) $(OBJ_FILES) -o $@

clean:
	$(RM) *.o $(LIBANAME) $(LIBSONAME)
	$(RM) -rf Templates.DB

