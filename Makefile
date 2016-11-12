# Top Makefile in <sunny256/suuid.git>
# File ID: c9f9bc1a-28d8-11e5-b53c-fefdb24f8e10
# Author: Øyvind A. Holm <sunny@sunbase.org>

.PHONY: default
default:
	cd src && $(MAKE)

.PHONY: clean
clean:
	rm -fv synced.sqlite.20*.bck tags
	find . -name .testadd.tmp -type d -print0 | xargs -0r rm -rf
	cd src && $(MAKE) clean
	cd tests && $(MAKE) clean

.PHONY: testlock
testlock:
	cd src && $(MAKE) testlock

.PHONY: testnew
testnew: test
	cd src && $(MAKE) testnew

.PHONY: test
test:
	cd src && $(MAKE) test
	cd tests && $(MAKE) test

tags: src/*.c src/*.h
	ctags src/*.c src/*.h
