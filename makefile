system:
	cd D; $(MAKE) system
	cd cryptography; $(MAKE) system
	cd granitecore; $(MAKE) system
	cd packages; packagemake system

static:
	cd D; $(MAKE) static
	cd cryptography; $(MAKE) static
	cd granitecore; $(MAKE) static
	cd packages; packagemake static

headers:
	cd granitecore; $(MAKE) headers 

clean:
	cd D; $(MAKE) clean
	cd cryptography; $(MAKE) clean
	cd granitecore; $(MAKE) clean
	cd packages; packagemake clean
	cd nsapi; $(MAKE) clean

