# Makefile for suuid.c
# File ID: 27e0d738-2b93-11e6-8668-02010e0a6634
# Author: Øyvind A. Holm <sunny@sunbase.org>

SUUID_EXEC = suuid
SESS_EXEC  = sess

TESTLOCKDIR = testlockdir
CTAGS = ctags

CC = gcc
LD = gcc
CCFLAGS = -Wall -Wextra -Werror -O0 -c -g
LDFLAGS = -Wall -Wextra -Werror
LIBS = -lpthread -ldl

# For regular use, it's not necessary to modify anything below this line.

COMMON_OBJS = database.o environ.o genuuid.o io.o logfile.o \
              rcfile.o sessvar.o sqlite3.o string.o tag.o uuid.o
SUUID_OBJS  = suuid.o
SESS_OBJS   = sess.o

COMMON_C = environ.c genuuid.c io.c logfile.c rcfile.c sessvar.c string.c \
           tag.c uuid.c

SUUID_HFILES = suuid.h common.h
SESS_HFILES  = sess.h common.h

SUUID_DEPS = $(SUUID_HFILES) Makefile
SESS_DEPS  = $(SESS_HFILES) $(SUUID_DEPS) Makefile

.PHONY: default
default: $(SUUID_EXEC) $(SESS_EXEC)

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

sess.o: sess.c $(SESS_DEPS)
	$(CC) $(CCFLAGS) sess.c

sessvar.o: sessvar.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) sessvar.c

sqlite3.o: sqlite3.c Makefile
	$(CC) $(CCFLAGS) sqlite3.c

string.o: string.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) string.c

tag.o: tag.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) tag.c

uuid.o: uuid.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) uuid.c

.PHONY: clean
clean:
	rm -fv $(COMMON_OBJS)
	rm -fv $(SUUID_EXEC) $(SUUID_OBJS)
	rm -fv $(SESS_EXEC) $(SESS_OBJS)
	rm -fv *~ core core.$(SUUID_EXEC) tags
	rm -frv $(TESTLOCKDIR)
	cd t && $(MAKE) clean

.PHONY: edit
edit: tags
	$(EDITOR) *.c *.h Makefile t/*.t

.PHONY: test
test: $(SUUID_EXEC)
	cd t && $(MAKE) test

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
	cd t && ./Genlog ./sess.t

tags: $(COMMON_C) suuid.c sess.c $(SUUID_HFILES) $(SESS_HFILES) t/suuid.t
	$(CTAGS) $(COMMON_C) suuid.c sess.c $(SUUID_HFILES) $(SESS_HFILES) \
	    t/suuid.t
