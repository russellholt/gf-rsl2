# $Id: makefile,v 1.1 1998/11/17 23:01:35 toddm Exp $

include $(MAKEFILE_LOCAL)

INCLUDE  =	-I. -I$(GRANITE)/granitecore/include -I$(LOCAL_INC) -I$(DCORE)
CPPFLAGS = 	$(DBG_FLAG) -features=rtti $(FPIC) $(INCLUDE)
LIBS     =  $(RWLIB) -lD
LDDFLAGS = -L. -L$(RWDIR) -L/sw/opt/SUNWspro/SC4.2/lib -L$(DCORE) $(LIBS) 

PACKAGE = D_rsl
LIBSONAME = ../../lib$(PACKAGE).so
LIBANAME = ../../lib$(PACKAGE).a

OBJ_FILES = Dlib_def.o

######################################################################

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

