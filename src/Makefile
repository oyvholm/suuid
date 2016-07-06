# Makefile for suuid.c
# File ID: 27e0d738-2b93-11e6-8668-02010e0a6634
# Author: Øyvind A. Holm <sunny@sunbase.org>

OBJS   = environ.o io.o logfile.o rcfile.o sessvar.o string.o suuid.o \
         tag.o uuid.o
CFILES = environ.c io.c logfile.c rcfile.c sessvar.c string.c suuid.c \
         tag.c uuid.c
HFILES = suuid.h
DEPS = $(HFILES) Makefile
CC = gcc
LD = gcc
CCFLAGS = -Wall -Wextra -Werror -O2 -c -g -pedantic
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

uuid.o: uuid.c $(DEPS)
	$(CC) $(CCFLAGS) uuid.c

.PHONY: addtest
addtest:
	test -d ../.git
	rm -rf ../x
	cd .. && git clone . x
	git diff --cached --binary | (cd ../x && git apply)
	cd ../x/src && make test

.PHONY: clean
clean:
	rm -fv $(EXEC) $(OBJS) *~ core core.$(EXEC) tags
	rm -frv locktestdir
	# cd t && $(MAKE) clean

.PHONY: edit
edit: tags
	$(EDITOR) *.c *.h Makefile t/*.t

tags: $(CFILES) $(HFILES)
	ctags $(CFILES) $(HFILES)

.PHONY: test
test: $(EXEC)
	@echo Temporary disabled
	# cd t && make

.PHONY: locktest
locktest: $(EXEC)
	rm -rf locktestdir
	mkdir locktestdir
	for f in $$(seq 1 500); do ./suuid -wn -l locktestdir; done &
	for f in $$(seq 1 500); do ./suuid -wn -l locktestdir; done &
	for f in $$(seq 1 500); do ./suuid -wn -l locktestdir; done
