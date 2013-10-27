# master Makefile for rsl2
# $Id: makefile,v 1.1 1998/11/17 23:52:03 toddm Exp $

include $(MAKEFILE_LOCAL)

system:
	$(CD) librsl; $(MAKE) system
	$(CD) base; $(MAKE) system
#	$(MV) librsl.so ..

copy:
	$(CP) srsl $(GRANITE)/granitecore/bin/rsl

static:
	$(CD) librsl; $(MAKE) static
	$(CD) base; $(MAKE) static

headers:
	$(CD) librsl; $(MAKE) headers
	$(CD) base; $(MAKE) headers

clean:
	$(CD) librsl; $(MAKE) clean
	$(CD) base; $(MAKE) clean
