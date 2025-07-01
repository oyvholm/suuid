# Top Makefile in <oyvholm/suuid.git>
# File ID: c9f9bc1a-28d8-11e5-b53c-fefdb24f8e10
# Author: Øyvind A. Holm <sunny@sunbase.org>

PREFIX = /usr/local

# LONGLINES_FILES += conv-old-suuid
# LONGLINES_FILES += conv-suuid
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
LONGLINES_FILES  =
LONGLINES_FILES += .gitlab-ci.yml
LONGLINES_FILES += Makefile
LONGLINES_FILES += README.md
LONGLINES_FILES += add-missing-dates
LONGLINES_FILES += fileid
LONGLINES_FILES += finduuid
LONGLINES_FILES += needuuid
LONGLINES_FILES += sess
LONGLINES_FILES += sortuuid
LONGLINES_FILES += ti
LONGLINES_FILES += tjah
LONGLINES_FILES += uuiddate
LONGLINES_FILES += v
LONGLINES_FILES += wi

.PHONY: all
all:
	cd doc && $(MAKE) $@
	cd src && $(MAKE) $@

%.html: FORCE
	test -e "$*.md"
	echo '<html>' >$@.tmp
	echo '<head>' >>$@.tmp
	echo '<meta charset="UTF-8" />' >>$@.tmp
	echo '<title>$* - suuid</title>' >>$@.tmp
	echo '</head>' >>$@.tmp
	echo '<body>' >>$@.tmp
	cmark $*.md >>$@.tmp
	if test -n "$$(git log -1 --format=%h $*.md 2>/dev/null)"; then \
		(echo 'Generated from `$*.md`'; \
		git log -1 --format='revision `%h` (%ci)' \
		    $*.md) | cmark >>$@.tmp; \
	fi
	echo '</body>' >>$@.tmp
	echo '</html>' >>$@.tmp
	mv $@.tmp $@

%.pdf: FORCE
	$(MAKE) $*.html
	wkhtmltopdf $*.html $@.tmp
	mv $@.tmp $@

tags: src/*.[ch]
	ctags src/*.[ch]

.PHONY: cflags
cflags:
	@cd src && $(MAKE) -s $@

.PHONY: clean
clean:
	find . -name .testadd.tmp -type d -print0 | xargs -0r rm -rf
	rm -f README.html README.html.tmp
	rm -f README.pdf README.pdf.tmp
	cd doc && $(MAKE) $@
	cd src && $(MAKE) $@
	cd tests && $(MAKE) $@

.PHONY: distclean
distclean: clean

.PHONY: FORCE
FORCE:

.PHONY: html
html:
	$(MAKE) README.html
	cd src && $(MAKE) $@

.PHONY: install
install:
	mkdir -p $(PREFIX)/bin
	install $(EXECS) $(PREFIX)/bin
	cd src && $(MAKE) $@

.PHONY: longlines
longlines:
	@for f in $(LONGLINES_FILES); do \
		if [ -f "$$f" ]; then \
			d="$$(expand "$$f" | sed 's/ $$//;' \
			  | grep -E -n '.{80}')"; \
			echo "$$d" | grep -q . \
			&& (echo "$$f"; printf "%s\\n" "$$d"; echo " "); \
		fi; \
	done | grep . && exit 1 || true
	cd src && $(MAKE) -s $@

.PHONY: pdf
pdf:
	$(MAKE) README.pdf
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
	$(MAKE) -s testsrc
	cd src && $(MAKE) -s $@
	cd tests && $(MAKE) $@

.PHONY: testsrc
testsrc:
	@echo Check files for long lines
	@$(MAKE) -s longlines
	cd src && $(MAKE) -s $@

.PHONY: tlok
tlok:
	@cd src && $(MAKE) -s $@

.PHONY: tlokall
tlokall:
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
