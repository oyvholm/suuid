# Makefile for suuid.c
# File ID: 5cedd236-af56-11e6-92f6-279c2a0468a3
# Author: Øyvind A. Holm <sunny@sunbase.org>

PREFIX = /usr/local

EXEC = suuid

TESTLOCKDIR = testlockdir

CC = cc
LD = cc

CCFLAGS = -Wall -Wextra -Werror -O0 -c -g
CCFLAGS += $$(test -n "$(GCOV)" && \
              echo -n "-fprofile-arcs -ftest-coverage")

LDFLAGS = -Wall -Wextra -Werror

LIBS =
LIBS += $$(test -n "$(GCOV)" && echo "-lgcov --coverage");

# For regular use, it's not necessary to modify anything below this line.

OBJS = suuid.o environ.o genuuid.o io.o logfile.o \
       rcfile.o selftest.o sessvar.o string.o tag.o uuid.o

CFILES = suuid.c environ.c genuuid.c io.c logfile.c \
         rcfile.c selftest.c sessvar.c string.c tag.c uuid.c

HFILES = suuid.h uuid.h

DEPS = $(HFILES) Makefile
GNCOV_STR = $$(test -n "$(GNCOV)" && echo "-g")

.PHONY: all
all: $(EXEC)

$(EXEC): $(OBJS)
	$(LD) -o $(EXEC) $(LDFLAGS) $(OBJS) $(LIBS)

version.h: Gen-version $(CFILES) $(DEPS)
	./Gen-version

suuid.o: version.h suuid.c $(DEPS)
	$(CC) $(CCFLAGS) suuid.c

environ.o: environ.c $(DEPS)
	$(CC) $(CCFLAGS) environ.c

genuuid.o: genuuid.c $(DEPS)
	$(CC) $(CCFLAGS) genuuid.c

io.o: io.c $(DEPS)
	$(CC) $(CCFLAGS) io.c

logfile.o: logfile.c $(DEPS)
	$(CC) $(CCFLAGS) logfile.c

rcfile.o: rcfile.c $(DEPS)
	$(CC) $(CCFLAGS) rcfile.c

selftest.o: selftest.c $(DEPS)
	$(CC) $(CCFLAGS) selftest.c

sessvar.o: sessvar.c $(DEPS)
	$(CC) $(CCFLAGS) sessvar.c

string.o: string.c $(DEPS)
	$(CC) $(CCFLAGS) string.c

tag.o: tag.c $(DEPS)
	$(CC) $(CCFLAGS) tag.c

uuid.o: uuid.c $(DEPS)
	$(CC) $(CCFLAGS) uuid.c

.PHONY: clean
clean:
	rm -f $(EXEC) $(OBJS)
	rm -f *.gcda *.gcno *.gcov
	rm -f version.h
	rm -fr $(TESTLOCKDIR)
	cd t && $(MAKE) clean

.PHONY: edit
edit: tags
	$(EDITOR) $$(git ls-files | grep -v -e ^COPYING -e \\.gitignore)

.PHONY: gcov
gcov:
	$(MAKE) clean test GCOV=1
	gcov $(CFILES)
	@echo $$(grep -E '^ +#####:' *.c.gcov | wc -l) untested lines >&2
	@echo $$(grep -E '^ +#####:' *.c.gcov | grep -F '/* gncov */' | \
	         wc -l) gncov lines >&2
	@echo $$(grep -E '^ +#####:' *.c.gcov | grep -vF '/* gncov */' | \
	         wc -l) testable lines >&2

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
	install $(EXEC) $(PREFIX)/bin/$(EXEC)

tags: $(CFILES) $(HFILES) t/suuid.t
	ctags $(CFILES) $(HFILES) t/suuid.t

.PHONY: test
test: $(EXEC)
	cd t && $(MAKE) test

.PHONY: testall
testall:
	$(MAKE) testcomb WHAT=test

.PHONY: testboth
testboth:

.PHONY: testcomb
testcomb: clean $(WHAT)
	$(MAKE) clean $(WHAT) FAKE_HOST=1
	$(MAKE) clean $(WHAT) GCOV=1
	$(MAKE) clean $(WHAT) NDEBUG=1
	$(MAKE) clean $(WHAT) SELFTEST=1
	$(MAKE) clean $(WHAT) TEST_FUNC=1
	$(MAKE) clean $(WHAT) UNUSED=1
	$(MAKE) clean $(WHAT) VERIFY_UUID=1
	$(MAKE) clean $(WHAT) FAKE_HOST=1 GCOV=1 NDEBUG=1 SELFTEST=1 \
	                      TEST_FUNC=1 UNUSED=1 VERIFY_UUID=1

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

.PHONY: testnew
testnew: test

.PHONY: uninstall
uninstall:
	rm -f $(PREFIX)/bin/$(EXEC)

.PHONY: valgrind
valgrind:
	cd t && $(MAKE) valgrind

.PHONY: valgrindall
valgrindall:
	$(MAKE) testcomb WHAT=valgrind

.PHONY: valgrindboth
valgrindboth:
