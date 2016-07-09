# Makefile for suuid.c
# File ID: 27e0d738-2b93-11e6-8668-02010e0a6634
# Author: Øyvind A. Holm <sunny@sunbase.org>

SUUID_OBJS = environ.o io.o logfile.o rcfile.o sessvar.o string.o suuid.o \
             tag.o uuid.o
SESS_OBJS =  sess.o
CFILES = environ.c io.c logfile.c rcfile.c sessvar.c string.c suuid.c \
         tag.c uuid.c \
         sess.c
SUUID_HFILES = suuid.h
DEPS = $(SUUID_HFILES) Makefile
CC = gcc
LD = gcc
CCFLAGS = -Wall -Wextra -Werror -Og -c -g -pedantic
LDFLAGS = -Wall

default: suuid sess

suuid: $(SUUID_OBJS)
	$(LD) -o suuid $(LDFLAGS) $(SUUID_OBJS)

sess: $(SESS_OBJS)
	$(LD) -o sess $(LDFLAGS) $(SESS_OBJS)

suuid.o: suuid.c $(DEPS)
	$(CC) $(CCFLAGS) suuid.c

environ.o: environ.c $(DEPS)
	$(CC) $(CCFLAGS) environ.c

io.o: io.c $(DEPS)
	$(CC) $(CCFLAGS) io.c

logfile.o: logfile.c $(DEPS)
	$(CC) $(CCFLAGS) logfile.c

rcfile.o: rcfile.c $(DEPS)
	$(CC) $(CCFLAGS) rcfile.c

sess.o: sess.c $(DEPS)
	$(CC) $(CCFLAGS) sess.c

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
	rm -fv suuid $(SUUID_OBJS) $(SESS_OBJS) *~ core core.suuid tags
	rm -frv locktestdir
	cd t && $(MAKE) clean

.PHONY: edit
edit: tags
	$(EDITOR) *.c *.h Makefile t/*.t

.PHONY: locktest
locktest: suuid
	rm -rf locktestdir
	mkdir locktestdir
	for f in $$(seq 1 500); do ./suuid -wn -l locktestdir; done &
	for f in $$(seq 1 500); do ./suuid -wn -l locktestdir; done &
	for f in $$(seq 1 500); do ./suuid -wn -l locktestdir; done

tags: $(CFILES) $(SUUID_HFILES)
	ctags $(CFILES) $(SUUID_HFILES)

.PHONY: test
test: suuid
	cd t && make
