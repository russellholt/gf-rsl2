system:
	cd dutil/src; $(MAKE)
	cd rsl; $(MAKE)
	cd rsl; $(MAKE) copy
	cd cloud; $(MAKE)
	cd cloud; $(MAKE) copy

static:
	cd dutil/src; $(MAKE) static
	cd rsl; $(MAKE) static

headers:
	cd dutil/src; $(MAKE) headers
	cd rsl; $(MAKE) headers
	
clean:
	rm -f *.so *.a bin/*
	cd dutil/src; $(MAKE) clean
	cd rsl; $(MAKE) clean
	cd cloud; $(MAKE) clean
