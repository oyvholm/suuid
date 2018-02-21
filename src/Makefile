# Makefile for suuid.c
# File ID: 5cedd236-af56-11e6-92f6-279c2a0468a3
# Author: Øyvind A. Holm <sunny@sunbase.org>

PREFIX = /usr/local

EXEC = suuid

TESTLOCKDIR = testlockdir

CC = cc
LD = cc

UNAME := $(shell uname)

CCFLAGS = -Wall -Wextra -Werror -O0 -c -g
CCFLAGS += $$(test -n "$(GCOV)" && \
              echo -n "-fprofile-arcs -ftest-coverage")

# sqlite3.c gives warnings on NetBSD and OpenBSD, as described in 
# ../doc/netbsd.txt and ../doc/openbsd.txt. Temporarily disable -Werror for 
# sqlite3.c until this is sorted out.
SQLITE_CCFLAGS = -Wall -Wextra -O0 -c -g

ifndef USE_SQLITE
	CCFLAGS += -pedantic
endif

LDFLAGS = -Wall -Wextra -Werror

# When compiling with SQLite, add these libraries to LIBS:
# Linux (gcc)    : -lpthread -ldl
# FreeBSD (clang): -lpthread
LIBS =
ifdef USE_SQLITE
	LIBS += -lpthread
	ifeq ($(UNAME), Linux)
		LIBS += -ldl
	endif
endif
LIBS += $$(test -n "$(GCOV)" && echo "-lgcov --coverage");

# For regular use, it's not necessary to modify anything below this line.

# If compiling with USE_SQLITE, add sqlite3.o to OBJS
OBJS = suuid.o database.o environ.o genuuid.o io.o logfile.o \
       rcfile.o selftest.o sessvar.o string.o tag.o uuid.o
ifdef USE_SQLITE
	OBJS += sqlite3.o
endif

CFILES = suuid.c database.c environ.c genuuid.c io.c logfile.c \
         rcfile.c selftest.c sessvar.c string.c tag.c uuid.c

HFILES = suuid.h

DEPS = $(HFILES) Makefile

.PHONY: all
all: $(EXEC)

version.h: Gen-version $(CFILES) $(DEPS)
	./Gen-version

$(EXEC): $(OBJS)
	$(LD) -o $(EXEC) $(LDFLAGS) $(OBJS) $(LIBS)

suuid.o: version.h suuid.c $(DEPS)
	$(CC) $(CCFLAGS) suuid.c

database.o: database.c $(DEPS)
	$(CC) $(CCFLAGS) database.c

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

sqlite3.o: sqlite3.c
	$(CC) $(SQLITE_CCFLAGS) -DSQLITE_ENABLE_JSON1 sqlite3.c

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
	rm -f sqlite3.o
	rm -fr $(TESTLOCKDIR)
	cd t && $(MAKE) clean

.PHONY: edit
edit: tags
	$(EDITOR) $$(git ls-files | grep -v -e ^COPYING -e ^sqlite3 \
	                                    -e \\.gitignore)

.PHONY: gcov
gcov:
	$(MAKE) clean test GCOV=1
	gcov $(CFILES)
	@echo $$(grep -E '^ +#####:' *.c.gcov | wc -l) untested lines >&2

.PHONY: gcov-cmt
gcov-cmt: gcov
	gcov-cmt $(CFILES)

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
	$(MAKE) clean test USE_SQLITE=1
	$(MAKE) clean test USE_SQLITE=

.PHONY: testcomb
testcomb: clean $(WHAT)
	# FAKE_HOST=
	$(MAKE) clean $(WHAT) FAKE_HOST=
	$(MAKE) clean $(WHAT) FAKE_HOST= USE_SQLITE=
	$(MAKE) clean $(WHAT) FAKE_HOST= USE_SQLITE=1
	# FAKE_HOST=1
	$(MAKE) clean $(WHAT) FAKE_HOST=1
	$(MAKE) clean $(WHAT) FAKE_HOST=1 USE_SQLITE=
	$(MAKE) clean $(WHAT) FAKE_HOST=1 USE_SQLITE=1
	# USE_SQLITE
	$(MAKE) clean $(WHAT) USE_SQLITE=
	$(MAKE) clean $(WHAT) USE_SQLITE=1
	# TEST_FUNC=
	$(MAKE) clean $(WHAT) TEST_FUNC=
	$(MAKE) clean $(WHAT) TEST_FUNC= USE_SQLITE=
	$(MAKE) clean $(WHAT) TEST_FUNC= USE_SQLITE=1
	# TEST_FUNC=1
	$(MAKE) clean $(WHAT) TEST_FUNC=1
	$(MAKE) clean $(WHAT) TEST_FUNC=1 USE_SQLITE=
	$(MAKE) clean $(WHAT) TEST_FUNC=1 USE_SQLITE=1

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
	$(MAKE) clean valgrind USE_SQLITE=1
	$(MAKE) clean valgrind USE_SQLITE=
