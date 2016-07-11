# Makefile for suuid.c
# File ID: 27e0d738-2b93-11e6-8668-02010e0a6634
# Author: Øyvind A. Holm <sunny@sunbase.org>

SUUID_OBJS = environ.o genuuid.o io.o logfile.o rcfile.o sessvar.o string.o \
             suuid.o tag.o uuid.o
SESS_OBJS  = sess.o
CFILES     = environ.c genuuid.c io.c logfile.c rcfile.c sessvar.c string.c \
             suuid.c tag.c uuid.c \
             sess.c
SUUID_HFILES = suuid.h common.h
SESS_HFILES = sess.h common.h
SUUID_DEPS = $(SUUID_HFILES) Makefile
SESS_DEPS = $(SESS_HFILES) Makefile
CC = gcc
LD = gcc
CCFLAGS = -Wall -Wextra -Werror -O2 -c -g -pedantic
LDFLAGS = -Wall -Wextra -Werror

default: suuid sess

suuid: $(SUUID_OBJS)
	$(LD) -o suuid $(LDFLAGS) $(SUUID_OBJS)

sess: $(SESS_OBJS)
	$(LD) -o sess $(LDFLAGS) $(SESS_OBJS)

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

tags: $(CFILES) $(SUUID_HFILES) $(SESS_HFILES)
	ctags $(CFILES) $(SUUID_HFILES) $(SESS_HFILES)

.PHONY: test
test: suuid sess
	cd t && make
