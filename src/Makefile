# Makefile for suuid.c
# File ID: 5cedd236-af56-11e6-92f6-279c2a0468a3
# Author: Øyvind A. Holm <sunny@sunbase.org>

NO_SQLITE = 1

SUUID_EXEC = suuid
SESS_EXEC  = sess

TESTLOCKDIR = testlockdir
CTAGS = ctags

CC = cc
LD = cc

# sqlite3.h uses long long, and gcc 4.8.4 and 4.9.2 (probably all other 
# versions too) abort when using -pedantic because it's not defined by ISO C90. 
# If you still want to use -pedantic with gcc, add -Wno-long-long to CCFLAGS. 
# Clang 3.8.0 on FreeBSD 11.0 works with -pedantic.
CCFLAGS = -Wall -Wextra -Werror -O0 -c -g
ifeq ($(NO_SQLITE), 1)
	CCFLAGS += -pedantic -DNO_SQLITE
endif
LDFLAGS = -Wall -Wextra -Werror

# When compiling with SQLite, add these libraries to LIBS:
# Linux (gcc)    : -lpthread -ldl
# FreeBSD (clang): -lpthread
LIBS =
ifneq ($(NO_SQLITE), 1)
	LIBS += -lpthread -ldl
endif

# For regular use, it's not necessary to modify anything below this line.

# If compiling without -DNO_SQLITE, add sqlite3.o to COMMON_OBJS
COMMON_OBJS = database.o environ.o genuuid.o io.o logfile.o \
              rcfile.o selftest.o sessvar.o string.o tag.o uuid.o
ifneq ($(NO_SQLITE), 1)
	COMMON_OBJS += sqlite3.o
endif

SUUID_OBJS  = suuid.o
SESS_OBJS   = sess.o

COMMON_C = environ.c genuuid.c io.c logfile.c rcfile.c \
           selftest.c sessvar.c string.c tag.c uuid.c

SUUID_HFILES = suuid.h common.h
SESS_HFILES  = sess.h common.h

SUUID_DEPS = $(SUUID_HFILES) Makefile
SESS_DEPS  = $(SESS_HFILES) $(SUUID_DEPS) Makefile

DEFAULT_DEPS = $(SUUID_EXEC) $(SESS_EXEC)
.PHONY: default
default: $(DEFAULT_DEPS)

$(SUUID_EXEC): $(SUUID_OBJS) $(COMMON_OBJS)
	$(LD) -o $(SUUID_EXEC) $(LDFLAGS) $(SUUID_OBJS) $(COMMON_OBJS) $(LIBS)

$(SESS_EXEC): $(SESS_OBJS) $(COMMON_OBJS)
	$(LD) -o $(SESS_EXEC) $(LDFLAGS) $(SESS_OBJS) $(COMMON_OBJS) $(LIBS)

suuid.o: suuid.c $(SUUID_DEPS)
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

sess.o: sess.c $(SESS_DEPS)
	$(CC) $(CCFLAGS) sess.c

sessvar.o: sessvar.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) sessvar.c

sqlite3.o: sqlite3.c
	$(CC) $(CCFLAGS) -DSQLITE_ENABLE_JSON1 sqlite3.c

string.o: string.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) string.c

tag.o: tag.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) tag.c

uuid.o: uuid.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) uuid.c

.PHONY: clean
clean:
	rm -f $(COMMON_OBJS)
	rm -f $(SUUID_EXEC) $(SUUID_OBJS)
	rm -f $(SESS_EXEC) $(SESS_OBJS)
	rm -f sqlite3.o
	rm -f *~ core core.$(SUUID_EXEC) tags
	rm -fr $(TESTLOCKDIR)
	cd t && $(MAKE) clean

.PHONY: edit
edit: tags
	$(EDITOR) $$(git ls-files | grep -v -e ^COPYING -e ^sqlite3 \
	                                    -e \\.gitignore)

.PHONY: test
test: $(SUUID_EXEC)
	cd t && $(MAKE) test

.PHONY: testall
testall: clean test
	@echo
	@echo Test with NO_SQLITE=0
	$(MAKE) clean test NO_SQLITE=0
	@echo
	@echo Test with NO_SQLITE=1
	$(MAKE) clean test NO_SQLITE=1

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
testnew: test $(SESS_EXEC)
	cd t && ./sess.t

tags: $(COMMON_C) suuid.c sess.c $(SUUID_HFILES) $(SESS_HFILES) t/suuid.t
	$(CTAGS) $(COMMON_C) suuid.c sess.c $(SUUID_HFILES) $(SESS_HFILES) \
	    t/suuid.t

.PHONY: valgrind
valgrind:
	cd t && $(MAKE) valgrind

.PHONY: valgrindall
valgrindall: clean test
	@echo
	@echo Test with NO_SQLITE=0
	$(MAKE) clean valgrind NO_SQLITE=0
	@echo
	@echo Test with NO_SQLITE=1
	$(MAKE) clean valgrind NO_SQLITE=1
