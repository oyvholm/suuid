# Makefile
# File ID: 6454cafe-4b04-11e6-8c85-02010e0a6634

EXEC = uuid

CC = cc
LD = cc
CCFLAGS = -Wall -Wextra -Werror -O0 -c -g
LDFLAGS = -Wall -Wextra -Werror
LIBS =

OBJS = sysdep.o uuid.o
CFILES = uuid.c sysdep.c
HFILES = copyrt.h sysdep.h uuid.h
DEPS = $(HFILES) Makefile

.PHONY: all
all: $(EXEC)

$(EXEC): $(OBJS)
	$(LD) -o $(EXEC) $(LDFLAGS) $(OBJS) $(LIBS)

sysdep.o: sysdep.c $(DEPS)
	$(CC) $(CCFLAGS) sysdep.c

uuid.o: uuid.c $(DEPS)
	$(CC) $(CCFLAGS) uuid.c

.PHONY: clean
clean:
	rm -f $(EXEC) $(OBJS)

.PHONY: edit
edit: tags
	$(EDITOR) $$(git ls-files)

.PHONY: gdb
gdb: $(EXEC)
	gdb -x gdbrc $$(cat gdbopts 2>/dev/null) $(EXEC)

tags: $(CFILES) $(HFILES)
	ctags $(CFILES) $(HFILES)

.PHONY: test
test: $(EXEC)
	./$(EXEC)
