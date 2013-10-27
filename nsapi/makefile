# Defines for example NSAPI programs running under SOLARIS

include $(ROOT)/makefile.local.cyg

NETSCAPE  =/sw/apps/enterprise3.0
NSAPI_INC = $(NETSCAPE)/include
INCLUDE   = -I$(NSAPI_INC) -I$(NSAPI_INC)/base -I$(NSAPI_INC)/frame -I$(RW_INC)

NSAPI_DEFS = -DNET_SSL -DSOLARIS -D_REENTRANT
COMMON_DEFS = -DMCC_HTTPD -DXP_UNIX
#DEBUG_DEFS = -DDEBUG -DDEBUGRES -DDEBUGREQ -DDEBUGHTTP -DDEBUGSESS
DEBUG_DEFS = 
 
CPPFLAGS = $(DBG_FLAG) $(FPIC) $(INCLUDE) $(NSAPI_DEFS) $(COMMON_DEFS) $(DEBUG_DEFS)
LIBS=$(RWLIB) $(LIBSOCKET)
LDDFLAGS = -L. -L$(RWDIR) $(LIBS)

LIBSONAME = ../WebHarness.so

OBJ_FILES = WebHarness.o Http.o HttpRequest.o HttpResponse.o HttpSession.o

# **************************************************************** #

system: $(LIBSONAME)

$(LIBSONAME): $(OBJ_FILES)
	$(LD) $(LDDFLAGS) $(SHARE) $(RPATH) $(RWDIR) $(OBJ_FILES) -o $@ 

WebHarness.o: WebHarness.cc
	$(CC) -c $(CPPFLAGS) WebHarness.cc

Http.o: Http.cc Http.h HttpSession.h
	$(CC) -c $(CPPFLAGS) Http.cc

HttpRequest.o: HttpRequest.cc HttpRequest.h HttpSession.h
	$(CC) -c $(CPPFLAGS) HttpRequest.cc

HttpResponse.o: HttpResponse.cc HttpResponse.h HttpSession.h
	$(CC) -c $(CPPFLAGS) HttpResponse.cc

HttpSession.o: HttpSession.cc HttpSession.h
	$(CC) -c $(CPPFLAGS) HttpSession.cc

clean:
	$(RM) *.o $(LIBSONAME)
	$(RM) -rf Templates.DB
