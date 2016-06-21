# Makefile for suuid.c
# File ID: 27e0d738-2b93-11e6-8668-02010e0a6634
# Author: Øyvind A. Holm <sunny@sunbase.org>

OBJS = logfile.o suuid.o uuid.o
DEPS = suuid.h version.h Makefile
CC = gcc
LD = gcc
CCFLAGS = -Wall -O2 -c -g
LDFLAGS = -Wall
EXEC = suuid

$(EXEC): $(OBJS)
	$(LD) -o $(EXEC) $(LDFLAGS) $(OBJS)

suuid.o: suuid.c $(DEPS)
	$(CC) $(CCFLAGS) suuid.c

logfile.o: logfile.c $(DEPS)
	$(CC) $(CCFLAGS) logfile.c

uuid.o: uuid.c $(DEPS)
	$(CC) $(CCFLAGS) uuid.c

.PHONY: clean
clean:
	rm -fv $(EXEC) $(OBJS) *~ core core.$(EXEC)
	cd t && $(MAKE) clean

.PHONY: edit
edit: tags
	$(EDITOR) *.c *.h Makefile

tags: suuid.c suuid.h version.h
	ctags suuid.c suuid.h version.h
