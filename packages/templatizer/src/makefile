# $Id: makefile,v 1.1 1998/11/17 23:27:38 toddm Exp $

include $(MAKEFILE_LOCAL)

INCLUDE =	-I. -I$(GRANITE)/granitecore/include -I$(LOCAL_INC) -I$(SOCK_DIR)
CPPFLAGS = 	-DBOOL -g $(FPIC) $(INCLUDE) -I$(DCORE)
LDDFLAGS = -L. -L$(GRANITE)/granitecore -L$(SOCK_DIR) -L$(RWDIR) $(RWLIB) -L$(DCORE) -lD
LOCALSOCK = -lsocket -lnsl

PACKAGE = templatizer
LIBSONAME = ../../lib$(PACKAGE).so
LIBANAME = ../../lib$(PACKAGE).a

OBJ_FILES = R_templatizer.o rclass_desc.o

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

