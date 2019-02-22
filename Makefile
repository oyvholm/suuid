# Top Makefile in <sunny256/suuid.git>
# File ID: c9f9bc1a-28d8-11e5-b53c-fefdb24f8e10
# Author: Øyvind A. Holm <sunny@sunbase.org>

.PHONY: all
all:
	cd doc && $(MAKE)
	cd src && $(MAKE)

.PHONY: clean
clean:
	rm -f synced.sqlite.20*.bck tags
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

.PHONY: testnew
testnew: test
	cd src && $(MAKE) testnew

.PHONY: test
test:
	cd doc && $(MAKE) test
	cd src && $(MAKE) test
	cd tests && $(MAKE) test

.PHONY: testall
testall:
	cd src && $(MAKE) testall
	cd tests && $(MAKE) testall

.PHONY: testboth
testboth:
	cd src && $(MAKE) testboth
	cd tests && $(MAKE) testboth

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

.PHONY: valgrindboth
valgrindboth:
	cd src && $(MAKE) valgrindboth
	cd tests && $(MAKE) testall
