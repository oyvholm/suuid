# Makefile for suuid.c
# File ID: 5cedd236-af56-11e6-92f6-279c2a0468a3
# Author: Øyvind A. Holm <sunny@sunbase.org>

PREFIX = /usr/local

EXEC = suuid

TESTLOCKDIR = testlockdir
CTAGS = ctags

CC = cc
LD = cc

UNAME := $(shell uname)

CCFLAGS = -Wall -Wextra -Werror -O0 -c -g
CCFLAGS += $$(test -n "$(GCOV)" && \
              echo -n "-fprofile-arcs -ftest-coverage -DGCOV=1")

# sqlite3.c gives warnings on NetBSD and OpenBSD, as described in 
# ../doc/netbsd.txt and ../doc/openbsd.txt. Temporarily disable -Werror for 
# sqlite3.c until this is sorted out.
SQLITE_CCFLAGS = -Wall -Wextra -O0 -c -g

# USE_SQLITE: Compile with SQLite. sqlite3.h uses long long, and gcc 4.8.4 and 
# 4.9.2 (probably all other versions too) abort when using -pedantic because 
# it's not defined by ISO C90. If you still want to use -pedantic with gcc, add 
# -Wno-long-long to CCFLAGS. Clang 3.8.0 on FreeBSD 11.0 works with -pedantic.
ifdef USE_SQLITE
	CCFLAGS += -DUSE_SQLITE
else
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

# If compiling with -DUSE_SQLITE, add sqlite3.o to COMMON_OBJS
COMMON_OBJS = database.o environ.o genuuid.o io.o logfile.o \
              rcfile.o selftest.o sessvar.o string.o tag.o uuid.o
ifdef USE_SQLITE
	COMMON_OBJS += sqlite3.o
endif

OBJS  = suuid.o

CFILES = database.c environ.c genuuid.c io.c logfile.c \
         rcfile.c selftest.c sessvar.c string.c tag.c uuid.c

SUUID_HFILES = suuid.h common.h

SUUID_DEPS = $(SUUID_HFILES) Makefile

DEFAULT_DEPS = $(EXEC)

.PHONY: all
all: $(DEFAULT_DEPS)

version.h: Gen-version suuid.c $(CFILES) $(SUUID_DEPS)
	./Gen-version

$(EXEC): $(OBJS) $(COMMON_OBJS)
	$(LD) -o $(EXEC) $(LDFLAGS) $(OBJS) $(COMMON_OBJS) $(LIBS)

suuid.o: version.h suuid.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) suuid.c

database.o: database.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) database.c

environ.o: environ.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) environ.c

genuuid.o: genuuid.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) genuuid.c

io.o: io.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) io.c

logfile.o: logfile.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) logfile.c

rcfile.o: rcfile.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) rcfile.c

selftest.o: selftest.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) selftest.c

sessvar.o: sessvar.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) sessvar.c

sqlite3.o: sqlite3.c
	$(CC) $(SQLITE_CCFLAGS) -DSQLITE_ENABLE_JSON1 sqlite3.c

string.o: string.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) string.c

tag.o: tag.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) tag.c

uuid.o: uuid.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) uuid.c

.PHONY: clean
clean:
	rm -f $(EXEC) $(OBJS)
	rm -f *.gcda *.gcno *.gcov
	rm -f version.h
	rm -f $(COMMON_OBJS)
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
	gcov suuid.c $(CFILES)
	@echo $$(grep -E '^ +#####:' *.c.gcov | wc -l) untested lines >&2

.PHONY: gcov-cmt
gcov-cmt: gcov
	gcov-cmt suuid.c $(CFILES)

.PHONY: gcov-cmt-clean
gcov-cmt-clean:
	gcov-cmt -d suuid.c $(CFILES)

.PHONY: gdb
gdb: $(EXEC)
	gdb -x gdbrc $$(cat gdbopts 2>/dev/null) $(EXEC)

.PHONY: install
install: $(PREFIX)/bin/$(EXEC)

$(PREFIX)/bin/$(EXEC): $(EXEC)
	install $(EXEC) $(PREFIX)/bin/$(EXEC)

tags: $(CFILES) suuid.c $(SUUID_HFILES) t/suuid.t
	$(CTAGS) $(CFILES) suuid.c $(SUUID_HFILES) t/suuid.t

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
