# Makefile for suuid.c
# File ID: 5cedd236-af56-11e6-92f6-279c2a0468a3
# Author: Øyvind A. Holm <sunny@sunbase.org>

PREFIX = /usr/local

EXEC = suuid

TESTLOCKDIR = testlockdir

CC = cc
LD = cc

CFLAGS  =
CFLAGS += $$(test -n "$(GCOV)" && echo -n "-fprofile-arcs -ftest-coverage")
CFLAGS += $$(test -n "$(PROF)" && echo -n "-pg")
CFLAGS += -O0 -c -g
CFLAGS += -Wall
CFLAGS += -Werror
CFLAGS += -Wextra

LDFLAGS  =
LDFLAGS += $$(test -n "$(PROF)" && echo -n "-pg")
LDFLAGS += -Wall
LDFLAGS += -Werror
LDFLAGS += -Wextra

LIBS  =
LIBS += $$(test -n "$(GCOV)" && echo "-lgcov --coverage")

# For regular use, it's not necessary to modify anything below this line.

OBJS  =
OBJS += suuid.o

OBJS += environ.o
OBJS += genuuid.o
OBJS += io.o
OBJS += logfile.o
OBJS += rcfile.o
OBJS += selftest.o
OBJS += sessvar.o
OBJS += string.o
OBJS += tag.o
OBJS += uuid.o

CFILES  =
CFILES += environ.c
CFILES += genuuid.c
CFILES += io.c
CFILES += logfile.c
CFILES += rcfile.c
CFILES += selftest.c
CFILES += sessvar.c
CFILES += string.c
CFILES += suuid.c
CFILES += tag.c
CFILES += uuid.c

HFILES  =
HFILES += suuid.h
HFILES += uuid.h

DEPS = $(HFILES) Makefile
GNCOV_STR = $$(test -n "$(GNCOV)" && echo "-g")
TESTABLE_LINES = $$(grep -E '^ +\#\#\#\#\#:' *.c.gcov \
                    | grep -vF '/* gncov */' | wc -l)

.PHONY: all
all: $(EXEC)

$(EXEC): $(OBJS)
	$(LD) -o $(EXEC) $(LDFLAGS) $(OBJS) $(LIBS)

version.h: Gen-version $(CFILES) $(DEPS)
	./Gen-version

suuid.o: version.h suuid.c $(DEPS)
	$(CC) $(CFLAGS) suuid.c

environ.o: environ.c $(DEPS)
	$(CC) $(CFLAGS) environ.c

genuuid.o: genuuid.c $(DEPS)
	$(CC) $(CFLAGS) genuuid.c

io.o: io.c $(DEPS)
	$(CC) $(CFLAGS) io.c

logfile.o: logfile.c $(DEPS)
	$(CC) $(CFLAGS) logfile.c

rcfile.o: rcfile.c $(DEPS)
	$(CC) $(CFLAGS) rcfile.c

selftest.o: selftest.c $(DEPS)
	$(CC) $(CFLAGS) selftest.c

sessvar.o: sessvar.c $(DEPS)
	$(CC) $(CFLAGS) sessvar.c

string.o: string.c $(DEPS)
	$(CC) $(CFLAGS) string.c

tag.o: tag.c $(DEPS)
	$(CC) $(CFLAGS) tag.c

uuid.o: uuid.c $(DEPS)
	$(CC) $(CFLAGS) uuid.c

.PHONY: clean
clean:
	rm -f $(EXEC) $(OBJS)
	rm -f *.gcda *.gcno *.gcov
	rm -f gmon.out gmon.sum
	rm -f version.h
	rm -fr $(TESTLOCKDIR)
	cd t && $(MAKE) clean

.PHONY: edit
edit: tags
	$(EDITOR) $$(git ls-files | grep -v -e ^COPYING)
	rm tags

.PHONY: gcov
gcov:
	$(MAKE) clean test GCOV=1
	gcov $(CFILES)
	@echo $$(grep -E '^ +#####:' *.c.gcov | wc -l) untested lines >&2
	@echo $$(grep -E '^ +#####:' *.c.gcov | grep -F '/* gncov */' \
	         | wc -l) gncov lines >&2
	@echo $(TESTABLE_LINES) testable lines >&2

.PHONY: gcov-cmt
gcov-cmt: gcov
	gcov-cmt $(GNCOV_STR) $(CFILES)

.PHONY: gcov-cmt-clean
gcov-cmt-clean:
	gcov-cmt -d $(CFILES)

.PHONY: gdb
gdb: $(EXEC)
	gdb -x gdbrc $$(cat gdbopts 2>/dev/null) $(EXEC)

.PHONY: install
install: $(PREFIX)/bin/$(EXEC)

$(PREFIX)/bin/$(EXEC): $(EXEC)
	mkdir -p $(PREFIX)/bin
	install $(EXEC) $(PREFIX)/bin/$(EXEC)

tags: $(CFILES) $(HFILES)
	ctags $(CFILES) $(HFILES)

.PHONY: test
test: $(EXEC)
	cd t && $(MAKE) test

.PHONY: testall
testall:
	$(MAKE) testcomb WHAT=test

.PHONY: testcomb
testcomb:
	$(MAKE) clean $(WHAT) FAKE_HOST=1
	$(MAKE) clean $(WHAT) NDEBUG=1
	$(MAKE) clean $(WHAT) SELFTEST=1
	$(MAKE) clean $(WHAT) TEST_FUNC=1
	$(MAKE) clean $(WHAT) UNUSED=1
	$(MAKE) clean $(WHAT) VERIFY_UUID=1
	$(MAKE) clean $(WHAT) FAKE_HOST=1 NDEBUG=1 SELFTEST=1 \
	                      TEST_FUNC=1 UNUSED=1 VERIFY_UUID=1
	$(MAKE) clean $(WHAT)

.PHONY: testhist
testhist: test
	while :; do \
		git checkout HEAD^; \
		echo; \
		GIT_PAGER=cat git log -1 \
		--pretty=tformat:'%C(green)%h - %s%C(reset)'; \
		$(MAKE) test || break; \
	done

.PHONY: testlock
testlock: $(EXEC)
	rm -rf $(TESTLOCKDIR)
	mkdir $(TESTLOCKDIR)
	for f in $$(seq 1 500); do ./$(EXEC) -wn -l $(TESTLOCKDIR); done &
	for f in $$(seq 1 500); do ./$(EXEC) -wn -l $(TESTLOCKDIR); done &
	for f in $$(seq 1 500); do ./$(EXEC) -wn -l $(TESTLOCKDIR); done

.PHONY: testsrc
testsrc:
	@echo Check files for long lines
	@for f in $(CFILES) $(HFILES) Gen-version Makefile; do \
		[ -f "$$f" ] && expand "$$f" | sed 's/ $$//;' \
		| grep -q -E '.{80}' && echo "$$f"; \
	done | grep . && exit 1 || true

.PHONY: tlok
tlok:
	@$(MAKE) -s gcov 2>.make-tlok.tmp >&2; res=$$?; \
	if test $$res -eq 0; then \
		printf '%u:%u\n' \
		       "$(TESTABLE_LINES)" \
		       "$$(grep ^1\\.\\. .make-tlok.tmp \
		           | perl -pe 's/^1\.\.(\d+).*/$$1/;')"; \
	else \
		grep "Looks like you failed" <.make-tlok.tmp \
		| perl -p \
		  -e  's/.*failed (\d+) tests? of (\d+).*/$$1\/$$2 FAIL/;'; \
	fi; \
	rm .make-tlok.tmp

.PHONY: uninstall
uninstall:
	rm -f $(PREFIX)/bin/$(EXEC)

.PHONY: valgrind
valgrind:
	cd t && $(MAKE) valgrind

.PHONY: valgrindall
valgrindall:
	$(MAKE) testcomb WHAT=valgrind
