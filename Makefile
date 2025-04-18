# Top Makefile in <oyvholm/suuid.git>
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
	cd doc && $(MAKE) $@
	cd src && $(MAKE) $@

tags: src/*.[ch]
	ctags src/*.[ch]

.PHONY: cflags
cflags:
	@cd src && $(MAKE) -s $@

.PHONY: clean
clean:
	find . -name .testadd.tmp -type d -print0 | xargs -0r rm -rf
	cd doc && $(MAKE) $@
	cd src && $(MAKE) $@
	cd tests && $(MAKE) $@

.PHONY: distclean
distclean: clean

.PHONY: install
install:
	mkdir -p $(PREFIX)/bin
	install $(EXECS) $(PREFIX)/bin
	cd src && $(MAKE) $@

.PHONY: testlock
testlock:
	cd src && $(MAKE) $@

.PHONY: test
test:
	cd doc && $(MAKE) $@
	cd src && $(MAKE) $@
	cd tests && $(MAKE) $@

.PHONY: testall
testall:
	cd src && $(MAKE) -s $@
	cd tests && $(MAKE) $@

.PHONY: tlok
tlok:
	@cd src && $(MAKE) -s $@

.PHONY: uninstall
uninstall:
	cd $(PREFIX)/bin && rm -f $(EXECS)
	cd src && $(MAKE) $@

.PHONY: valgrind
valgrind:
	cd src && $(MAKE) $@
	cd tests && $(MAKE) test

.PHONY: valgrindall
valgrindall:
	cd src && $(MAKE) $@
	cd tests && $(MAKE) testall
