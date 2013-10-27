# tech/rsl/base/makefile
# the RSL2 executable makefile
#
# 4/28/98 CP SUNWspro mods
# - added -I/usr/local/include to INCLUDE symbol to find <FlexLexer.h>
#
# 4/29/98 MB SUNWspro mods
# - changed librslbase.a build cmd line to handle template instantiations.  
#
# 5/01/98 CP SUNWspro mods
# - expanded gmake clean to remove Templates.DB files
# - added -I$(RW) to INCLUDE path
# - moved $(RWLIB) to LDDFLAGS from LIBS and SLIBS
# 
# 6/11/98 TM SUNWspro mods
# - Change -g in CPPFLAGS to $(DBG_FLAG)
# - Redefined AR in the makefile.local.xxx

# $Id: makefile,v 1.2 1998/12/22 21:43:55 toddm Exp $
# 
# mod for release_mgt

include $(MAKEFILE_LOCAL)
include ../rsl_debug_flags
include $(DCORE)/d_debug

INCLUDE  = -I. -I$(GRANITE)/granitecore/include -I$(RW_INC) -I$(SOCK_INC) -I$(LOCAL_INC) -I$(DCORE) -I$(CRYPTO) -I$(BSAFE_INC)
CPPFLAGS = $(DBG_FLAG) $(INCLUDE) $(RSL_DEBUG_FLAGS)
#LDDFLAGS = -L. -L.. -L$(GRANITE)/granitecore -lrsl -L$(RWDIR) -L$(SOCK_DIR) -L/sw/opt/SUNWspro/SC4.2/lib -L$(DCORE)
LDDFLAGS = -L. -L.. -L$(GRANITE)/granitecore -L$(RWDIR) -L$(SOCK_DIR) -L/sw/opt/SUNWspro/SC4.2/lib -L$(DCORE) -L$(CRPYTO) -L$(BSAFE_DIR)

UTIL  = -ldutil
SUTIL = -ldutil.a

LIBS  = $(LOCALDYNAMIC) $(UTIL) $(LIBSOCKET) $(RWLIB) $(SOCKXXLIB) $(EXTRA_LOCAL_LIBS) -lrsl -lD -ldl
SLIBS = -ldl $(LIBSOCKET) $(RWLIB) $(SOCKXXLIB) librsl.a libdutil.a

LIBANAME = librslbase.a

OBJ_FILES = runtime.o res_class.o lexer_context.o b.tab.o lex.yy.o R_D.o listcallback.o

# **************************************************************** #

system: ../srsl

# *****************************
# * Build the rslbase library *
# *****************************
$(LIBANAME): $(OBJ_FILES)
	$(AR) $@ $?
#	$(CP) $@ ..

# *************************
# * Build the Executables *
# *************************
# shared version
../srsl: $(LIBANAME) rslmain.o
	$(CCC) rslmain.o $(LDDFLAGS) -lencrypt -lbsafe -lrslbase $(LIBS) -o $@
#	$(MV) srsl ..

# purify shared version
../prsl: $(LIBANAME) rslmain.o
	purify $(CCC) rslmain.o $(LDDFLAGS) -lencrypt -lbsafe -lrslbase $(LIBS) -o $@

# static version
../qrsl: $(LIBANAME) rslmain.o
	$(CCC) rslmain.o $(EXTRA_STATICS) $(LDDFLAGS) -lencrypt -lbsafe -lrslbase $(SLIBS) -o $@
	$(CP) $@ ..

# ************************
# build lexer and parser *
# ************************
# $? means dependencies which are newer than the target.
# -Pb means use 'b' as the prefix instead of yy
lex.yy.o: b.l
	$(FLEX) -I $?
	$(CCC) $(CPPFLAGS) -DSMARTKEYWORDS -c lex.yy.cc

# -p b means use 'b' as the prefix for external calls
b.tab.o: b.y
	$(BISON) -d -v -p b $?
	$(CCC) $(CPPFLAGS) -c b.tab.c

headers:
	$(CP) *.h $(GRANITE)/granitecore/include

clean:
	$(RM) *.o $(LIBANAME) 
	$(RM) ../srsl ../qrsl ../prsl
	$(RM) -r Templates.DB

distclean:
	$(RM) *.o lex.yy.cc b.tab.*

