
# Makefile for suuid.c
# File ID: 27e0d738-2b93-11e6-8668-02010e0a6634
# Author: Øyvind A. Holm <sunny@sunbase.org>

OBJS = suuid.o
CC = gcc
LD = gcc
CCFLAGS = -Wall -O2 -c -g -ansi
LDFLAGS = -Wall
EXEC = suuid

$(EXEC): $(OBJS)
	$(LD) -o $(EXEC) $(LDFLAGS) $(OBJS)

suuid.o: suuid.c suuid.h version.h Makefile
	$(CC) $(CCFLAGS) suuid.c

.PHONY: clean
clean:
	rm -fv $(EXEC) $(OBJS) *~ core core.$(EXEC)
