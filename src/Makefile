# Makefile for suuid.c
# File ID: 5cedd236-af56-11e6-92f6-279c2a0468a3
# Author: Øyvind A. Holm <sunny@sunbase.org>

PREFIX = /usr/local

SUUID_EXEC = suuid

TESTLOCKDIR = testlockdir
CTAGS = ctags

CC = cc
LD = cc

UNAME := $(shell uname)

# sqlite3.h uses long long, and gcc 4.8.4 and 4.9.2 (probably all other 
# versions too) abort when using -pedantic because it's not defined by ISO C90. 
# If you still want to use -pedantic with gcc, add -Wno-long-long to CCFLAGS. 
# Clang 3.8.0 on FreeBSD 11.0 works with -pedantic.
CCFLAGS = -Wall -Wextra -Werror -O0 -c -g

CCFLAGS += $$(test -n "$(GCOV)" && \
              echo -n "-fprofile-arcs -ftest-coverage -DGCOV=1")
CCFLAGS += $$(test -n "$(NDEBUG)" && echo -n "-DNDEBUG=1")

# sqlite3.c gives warnings on NetBSD and OpenBSD, as described in 
# ../doc/netbsd.txt and ../doc/openbsd.txt. Temporarily disable -Werror for 
# sqlite3.c until this is sorted out.
SQLITE_CCFLAGS = -Wall -Wextra -O0 -c -g

# FAKE_HOST: Use "fake" as hostname to avoid conflicts with files created by 
# the Perl version.
ifdef FAKE_HOST
	CCFLAGS += -DFAKE_HOST
endif

# USE_SQLITE: Compile with SQLite.
ifdef USE_SQLITE
	CCFLAGS += -pedantic -DUSE_SQLITE
endif

# PERL_COMPAT: Compile a version with some changes to make it compatible with 
# the Perl version
ifdef PERL_COMPAT
	CCFLAGS += -DPERL_COMPAT
endif

# TEST_FUNC: Send non-option arguments to a function for testing. Doesn't break 
# anything, non-option arguments are ignored by the program.
ifdef TEST_FUNC
	CCFLAGS += -DTEST_FUNC
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

SUUID_OBJS  = suuid.o

COMMON_C = environ.c genuuid.c io.c logfile.c rcfile.c \
           selftest.c sessvar.c string.c tag.c uuid.c

SUUID_HFILES = suuid.h common.h

SUUID_DEPS = $(SUUID_HFILES) Makefile

DEFAULT_DEPS = $(SUUID_EXEC)

.PHONY: all
all: $(DEFAULT_DEPS)

version.h: Gen-version suuid.c $(COMMON_C) $(SUUID_DEPS)
	./Gen-version

$(SUUID_EXEC): $(SUUID_OBJS) $(COMMON_OBJS)
	$(LD) -o $(SUUID_EXEC) $(LDFLAGS) $(SUUID_OBJS) $(COMMON_OBJS) $(LIBS)

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
	rm -f $(SUUID_EXEC) $(SUUID_OBJS)
	rm -f *.gcda *.gcno *.gcov
	rm -f version.h
	rm -f $(COMMON_OBJS)
	rm -f sqlite3.o
	rm -fr $(TESTLOCKDIR)
	cd t && $(MAKE) clean

.PHONY: clean-gcov
clean-gcov:
	gcov-cmt -d suuid.c $(COMMON_C)

.PHONY: edit
edit: tags
	$(EDITOR) $$(git ls-files | grep -v -e ^COPYING -e ^sqlite3 \
	                                    -e \\.gitignore)

.PHONY: gcov
gcov: clean
	$(MAKE) GCOV=1
	$(MAKE) test
	gcov suuid.c $(COMMON_C)
	@echo $$(grep -E '^ +#####:' *.c.gcov | wc -l) gcov markers

.PHONY: gcov-clean
gcov-clean: clean-gcov

.PHONY: gcov-cmt
gcov-cmt: gcov
	gcov-cmt suuid.c $(COMMON_C)
	@echo $$( \
	    grep ' /\* gcov \*/' suuid.c $(COMMON_C) | wc -l \
	) gcov markers

.PHONY: gdb
gdb: $(SUUID_EXEC)
	gdb -x gdbrc $$(cat gdbopts 2>/dev/null) $(SUUID_EXEC)

.PHONY: install
install: $(PREFIX)/bin/$(SUUID_EXEC)

$(PREFIX)/bin/$(SUUID_EXEC): $(SUUID_EXEC)
	install $(SUUID_EXEC) $(PREFIX)/bin/$(SUUID_EXEC)

tags: $(COMMON_C) suuid.c $(SUUID_HFILES) t/suuid.t
	$(CTAGS) $(COMMON_C) suuid.c $(SUUID_HFILES) t/suuid.t

.PHONY: test
test: $(SUUID_EXEC)
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
	# PERL_COMPAT=
	$(MAKE) clean $(WHAT) PERL_COMPAT=
	$(MAKE) clean $(WHAT) PERL_COMPAT= USE_SQLITE=
	$(MAKE) clean $(WHAT) PERL_COMPAT= USE_SQLITE=1
	# PERL_COMPAT=1
	$(MAKE) clean $(WHAT) PERL_COMPAT=1
	$(MAKE) clean $(WHAT) PERL_COMPAT=1 USE_SQLITE=
	$(MAKE) clean $(WHAT) PERL_COMPAT=1 USE_SQLITE=1
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
testlock: $(SUUID_EXEC)
	rm -rf $(TESTLOCKDIR)
	mkdir $(TESTLOCKDIR)
	for f in $$(seq 1 500); do ./$(SUUID_EXEC) -wn -l $(TESTLOCKDIR); done &
	for f in $$(seq 1 500); do ./$(SUUID_EXEC) -wn -l $(TESTLOCKDIR); done &
	for f in $$(seq 1 500); do ./$(SUUID_EXEC) -wn -l $(TESTLOCKDIR); done

.PHONY: testnew
testnew: test

.PHONY: uninstall
uninstall:
	rm -f $(PREFIX)/bin/$(SUUID_EXEC)

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
