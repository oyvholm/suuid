# Makefile for suuid.c
# File ID: 27e0d738-2b93-11e6-8668-02010e0a6634
# Author: Øyvind A. Holm <sunny@sunbase.org>

SUUID_EXEC   = suuid
SESS_EXEC    = sess
COMMON_OBJS  = environ.o genuuid.o io.o logfile.o rcfile.o sessvar.o string.o \
               tag.o uuid.o
SUUID_OBJS   = suuid.o
SESS_OBJS    = sess.o
COMMON_C     = environ.c genuuid.c io.c logfile.c rcfile.c sessvar.c string.c \
               tag.c uuid.c
SUUID_HFILES = suuid.h common.h
SESS_HFILES  = sess.h common.h
SUUID_DEPS   = $(SUUID_HFILES) Makefile
SESS_DEPS    = $(SESS_HFILES) $(SUUID_DEPS) Makefile
LOCKTESTDIR  = locktestdir
CC = gcc
LD = gcc
CCFLAGS = -Wall -Wextra -Werror -O2 -c -g -pedantic
LDFLAGS = -Wall -Wextra -Werror

default: $(SUUID_EXEC) $(SESS_EXEC)

$(SUUID_EXEC): $(SUUID_OBJS) $(COMMON_OBJS)
	$(LD) -o $(SUUID_EXEC) $(LDFLAGS) $(SUUID_OBJS) $(COMMON_OBJS)

$(SESS_EXEC): $(SESS_OBJS) $(COMMON_OBJS)
	$(LD) -o $(SESS_EXEC) $(LDFLAGS) $(SESS_OBJS) $(COMMON_OBJS)

suuid.o: suuid.c $(SUUID_DEPS)
	$(CC) $(CCFLAGS) suuid.c

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
	rm -frv $(LOCKTESTDIR)
	cd t && $(MAKE) clean

.PHONY: edit
edit: tags
	$(EDITOR) *.c *.h Makefile t/*.t

.PHONY: locktest
locktest: $(SUUID_EXEC)
	rm -rf $(LOCKTESTDIR)
	mkdir $(LOCKTESTDIR)
	for f in $$(seq 1 500); do ./$(SUUID_EXEC) -wn -l $(LOCKTESTDIR); done &
	for f in $$(seq 1 500); do ./$(SUUID_EXEC) -wn -l $(LOCKTESTDIR); done &
	for f in $$(seq 1 500); do ./$(SUUID_EXEC) -wn -l $(LOCKTESTDIR); done

.PHONY: newtest
newtest: test $(SESS_EXEC)
	cd t && ./Genlog ./sess.t

tags: $(COMMON_C) suuid.c sess.c $(SUUID_HFILES) $(SESS_HFILES)
	ctags $(COMMON_C) suuid.c sess.c $(SUUID_HFILES) $(SESS_HFILES)

.PHONY: test
test: $(SUUID_EXEC)
	cd t && $(MAKE)
