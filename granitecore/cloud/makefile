#--------------------------------------------------------------
# Makefile for Registrar, Broker and Admin
#
# usage: make [ system | install | clean | new ]
#
# Assumes existence of environment variable 'ROOT'.
#
# $Id: makefile,v 1.1 1998/11/17 23:44:53 toddm Exp $
#--------------------------------------------------------------

# Include compiler specific makefile.
include $(MAKEFILE_LOCAL)

# Includes
INCLUDE = -I. -I$(GRANITE)/granitecore/include -I$(SOCK_DIR) -I$(CRYPTO) -I$(BSAFE_INC)

# Compile flags
CPPFLAGS = $(DBG_FLAG) $(FPIC) $(INCLUDE) $(USE_EXCEPTIONS) \
	-D_R_MAIN -D_B_MAIN

# Libraries
LIBS = $(SOCKXXLIB) -lencrypt $(LIBBSAFE) $(RWLIB) $(LIBSOCKET)
LDDFLAGS = -L. -L$(GRANITE)/granitecore -L$(RWDIR) -L$(CRYPTO) -L$(BSAFE_DIR) -L$(SOCK_DIR) $(LIBS)

#--------------------- Make Commands --------------------------

EXECS = Registrar Broker

system: $(EXECS)

copy: system
	$(CP) $(EXECS) $(GRANITE)/granitecore/bin

headers:
	$(CP) *.h $(GRANITE)/granitecore/include
						 
clean:
	$(RM) *.o $(EXECS)
	$(RM) -rf Templates.DB


#----------------- Registrar Specific -------------------------

REGISTRAR_OBJS = ConfigFileParser.o Log.o NetAddress.o \
	Registrar.o RegistrarConfigFileParser.o \
        RegistrarRequestHandler.o RegistrationList.o SessionId.o

Registrar: $(REGISTRAR_OBJS)
	$(CC) $(LDDFLAGS) -o Registrar $(REGISTRAR_OBJS)

ConfigFileParser.o: ConfigFileParser.cc ConfigFileParser.h
	$(CC) -c $(CPPFLAGS) ConfigFileParser.cc

Log.o: Log.cc Log.h
	$(CC) -c $(CPPFLAGS) Log.cc

NetAddress.o: NetAddress.cc NetAddress.h
	$(CC) -c $(CPPFLAGS) NetAddress.cc

Registrar.h: NetAddress.h RegistrationList.h

Registrar.o: Registrar.h Registrar.cc RegistrationList.h NetAddress.h \
	Log.h
	$(CC) -c $(CPPFLAGS) Registrar.cc

RegistrarConfigFileParser.h: ConfigFileParser.h NetAddress.h

RegistrarConfigFileParser.o: RegistrarConfigFileParser.cc \
	RegistrarConfigFileParser.h
	$(CC) -c $(CPPFLAGS) RegistrarConfigFileParser.cc

RegistrarRequestHandler.h: RegistrationList.h

RegistrarRequestHandler.o: RegistrarRequestHandler.cc \
	RegistrarRequestHandler.h Log.h
	$(CC) -c $(CPPFLAGS) RegistrarRequestHandler.cc

RegistrationList.h: NetAddress.h

RegistrationList.o: RegistrationList.cc RegistrationList.h NetAddress.h
	$(CC) -c $(CPPFLAGS) RegistrationList.cc

SessionId.o: SessionId.cc SessionId.h
	$(CC) -c $(CPPFLAGS) SessionId.cc

#--------------------- Broker Specific -------------------------

BROKER_OBJS = ConfigFileParser.o NetAddress.o SessionId.o Log.o \
	BrokerConfigFileParser.o GcpManager.o Broker.o \
        RegistrationList.o ProcessMgr.o

Broker: $(BROKER_OBJS)
	$(CC) $(LDDFLAGS) -o Broker $(BROKER_OBJS)

BrokerConfigFileParser.o: BrokerConfigFileParser.cc BrokerConfigFileParser.h
	$(CC) -c $(CPPFLAGS) BrokerConfigFileParser.cc

Broker.o: Broker.cc Broker.h
	$(CC) -c $(CPPFLAGS) Broker.cc

GcpManager.o: GcpManager.cc GcpManager.h
	$(CC) -c $(CPPFLAGS) GcpManager.cc

ProcessMgr.o: ProcessMgr.cc ProcessMgr.h
	$(CC) -c $(CPPFLAGS) ProcessMgr.cc
