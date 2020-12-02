# Top Makefile in <sunny256/suuid.git>
# File ID: c9f9bc1a-28d8-11e5-b53c-fefdb24f8e10
# Author: Øyvind A. Holm <sunny@sunbase.org>

.PHONY: all
all:
	cd doc && $(MAKE)
	cd src && $(MAKE)

.PHONY: clean
clean:
	rm -f tags
	find . -name .testadd.tmp -type d -print0 | xargs -0r rm -rf
	cd doc && $(MAKE) clean
	cd src && $(MAKE) clean
	cd tests && $(MAKE) clean

.PHONY: distclean
distclean: clean

.PHONY: install
install:
	cd src && $(MAKE) install

.PHONY: testlock
testlock:
	cd src && $(MAKE) testlock

.PHONY: test
test:
	cd doc && $(MAKE) test
	cd src && $(MAKE) test
	cd tests && $(MAKE) test

.PHONY: testall
testall:
	cd src && $(MAKE) testall
	cd tests && $(MAKE) testall

tags: src/*.c src/*.h
	ctags src/*.c src/*.h

.PHONY: uninstall
uninstall:
	cd src && $(MAKE) uninstall

.PHONY: valgrind
valgrind:
	cd src && $(MAKE) valgrind
	cd tests && $(MAKE) test

.PHONY: valgrindall
valgrindall:
	cd src && $(MAKE) valgrindall
	cd tests && $(MAKE) testall
