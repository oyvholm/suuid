# Makefile for suuid.c
# File ID: 27e0d738-2b93-11e6-8668-02010e0a6634
# Author: Øyvind A. Holm <sunny@sunbase.org>

OBJS   = environ.o io.o logfile.o rcfile.o sessvar.o string.o suuid.o \
         tag.o utf8.o uuid.o
CFILES = environ.c io.c logfile.c rcfile.c sessvar.c string.c suuid.c \
         tag.c utf8.c uuid.c
HFILES = suuid.h
DEPS = $(HFILES) Makefile
CC = gcc
LD = gcc
CCFLAGS = -Wall -Wextra -O2 -c -g -pedantic
LDFLAGS = -Wall
EXEC = suuid

$(EXEC): $(OBJS)
	$(LD) -o $(EXEC) $(LDFLAGS) $(OBJS)

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

sessvar.o: sessvar.c $(DEPS)
	$(CC) $(CCFLAGS) sessvar.c

string.o: string.c $(DEPS)
	$(CC) $(CCFLAGS) string.c

tag.o: tag.c $(DEPS)
	$(CC) $(CCFLAGS) tag.c

utf8.o: utf8.c $(DEPS)
	$(CC) $(CCFLAGS) utf8.c

uuid.o: uuid.c $(DEPS)
	$(CC) $(CCFLAGS) uuid.c

.PHONY: clean
clean:
	rm -fv $(EXEC) $(OBJS) *~ core core.$(EXEC) tags
	cd t && $(MAKE) clean

.PHONY: edit
edit: tags
	$(EDITOR) *.c *.h Makefile t/*.t

tags: $(CFILES) $(HFILES)
	ctags $(CFILES) $(HFILES)
