include $(MAKEFILE_LOCAL)
include d_debug

######################################################################

SUNFLAGS= -features=rtti
LDDFLAGS = -L$(RWDIR) $(RWLIB) -L/sw/opt/SUNWspro/SC4.2/lib
CPPFLAGS =  $(FPIC) $(DBG_FLAG) $(SUNFLAGS) $(D_DEBUG)

PACKAGE = D
LIBSONAME = ../lib$(PACKAGE).so
LIBANAME = ../lib$(PACKAGE).a

OBJ_FILES = D.o DRef.o DObject.o DString.o DMessage.o DCollection.o DList.o \
	DEnumerator.o DKeyedCollection.o DDictionary.o DLibrary.o DClassGroup.o \
	hash.o DClass.o Drecycler.o DComposite.o DMagnitude.o DInt.o DBool.o \
	freed.o DOStream.o DECI_OStream.o DXML_OStream.o Dcomparator.o \
	DBinaryTree.o DSortedBinaryTree.o

######################################################################

system: $(LIBSONAME)
static: $(LIBANAME)

# **************************
# * Build the util library *
# **************************
$(LIBSONAME): $(OBJ_FILES)
	$(CCC) $(SHARE)  $(OBJ_FILES) -o $@

$(LIBANAME): $(OBJ_FILES)
	$(AR) $@ $?

clean:
	$(RM) *.o LIBANAME LIBSONAME
	$(RM) -rf Templates.DB
