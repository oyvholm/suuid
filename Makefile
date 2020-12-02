# Top Makefile in <sunny256/suuid.git>
# File ID: c9f9bc1a-28d8-11e5-b53c-fefdb24f8e10
# Author: Øyvind A. Holm <sunny@sunbase.org>

PREFIX = /usr/local

EXECS  =
EXECS += conv-suuid
EXECS += fileid
EXECS += finduuid
EXECS += needuuid
EXECS += sess
EXECS += sortuuid
EXECS += ti
EXECS += tjah
EXECS += uuiddate
EXECS += v
EXECS += wi

.PHONY: all
all:
	cd doc && $(MAKE)
	cd src && $(MAKE)

tags: src/*.[ch]
	ctags src/*.[ch]

.PHONY: clean
clean:
	find . -name .testadd.tmp -type d -print0 | xargs -0r rm -rf
	cd doc && $(MAKE) clean
	cd src && $(MAKE) clean
	cd tests && $(MAKE) clean

.PHONY: distclean
distclean: clean

.PHONY: install
install:
	mkdir -p $(PREFIX)/bin
	install $(EXECS) $(PREFIX)/bin
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

.PHONY: uninstall
uninstall:
	cd $(PREFIX)/bin && rm -f $(EXECS)
	cd src && $(MAKE) uninstall

.PHONY: valgrind
valgrind:
	cd src && $(MAKE) valgrind
	cd tests && $(MAKE) test

.PHONY: valgrindall
valgrindall:
	cd src && $(MAKE) valgrindall
	cd tests && $(MAKE) testall
