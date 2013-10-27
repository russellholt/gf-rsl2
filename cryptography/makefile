# $Id: makefile,v 1.2 1998/11/24 20:42:56 cking Exp $

include $(MAKEFILE_LOCAL)

INCLUDE  = -I. -I$(RW_INC) -I$(SOCK_INC) -I$(LOCAL_INC) -I$(BSAFE_INC)
CPPFLAGS = $(DBG_FLAG) $(FPIC) $(INCLUDE)
LIBS =  $(LIBBSAFE) $(SOCKXXLIB) $(RWLIB) $(LIBSOCKET)
LDDFLAGS = -L. -L$(RWDIR) -L$(SOCK_DIR) -L$(BSAFE_DIR) $(LIBS)

PACKAGE = encrypt
LIBSONAME = ../lib$(PACKAGE).so
LIBANAME = ../lib$(PACKAGE).a

OBJ_FILES = CryptoSubSystem.o CryptoDHDE.o CryptoSession.o CryptoDH.o CryptoRC4.o CryptoRSApkcs.o CryptoLog.o CryptoBsafeUtil.o


# **************************************************************** #

system: $(LIBSONAME)
static: $(LIBANAME)

# *********************
# * Build the library *
# *********************
$(LIBSONAME): $(OBJ_FILES)
	$(CCC) $(LDDFLAGS) $(SHARE) $(OBJ_FILES) -o $@

$(LIBANAME): $(OBJ_FILES)
	$(AR) $@ $?

clean:
	$(RM) *.o LIBANAME LIBSONAME
	$(RM) -r Templates.DB
