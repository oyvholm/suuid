# Makefile
# File ID: 6454cafe-4b04-11e6-8c85-02010e0a6634

EXEC = uuid

CC = cc
LD = cc
CFLAGS = -Wall -Wextra -Werror -O0 -c -g
CFLAGS += $$(test -n "$(GCOV)" && echo -n "-fprofile-arcs -ftest-coverage")
LDFLAGS = -Wall -Wextra -Werror
LIBS =
LIBS += $$(test -n "$(GCOV)" && echo "-lgcov --coverage");

OBJS = sysdep.o uuid.o
CFILES = uuid.c sysdep.c
HFILES = copyrt.h sysdep.h uuid.h
DEPS = $(HFILES) Makefile

.PHONY: all
all: $(EXEC)

$(EXEC): $(OBJS)
	$(LD) -o $(EXEC) $(LDFLAGS) $(OBJS) $(LIBS)

sysdep.o: sysdep.c $(DEPS)
	$(CC) $(CFLAGS) sysdep.c

uuid.o: uuid.c $(DEPS)
	$(CC) $(CFLAGS) uuid.c

.PHONY: clean
clean:
	rm -f $(EXEC) $(OBJS)
	rm -f *.gcda *.gcno *.gcov
	rm -f nodeid state

.PHONY: edit
edit: tags
	$(EDITOR) $$(git ls-files)
	rm tags

.PHONY: gcov
gcov:
	$(MAKE) test GCOV=1
	gcov $(CFILES)
	@echo $$(grep -E '^ +#####:' *.c.gcov | wc -l) untested lines >&2

.PHONY: gcov-cmt
gcov-cmt: gcov
	gcov-cmt $(CFILES)

.PHONY: gcov-cmt-clean
gcov-cmt-clean:
	gcov-cmt -d $(CFILES)

.PHONY: gdb
gdb: $(EXEC)
	gdb -x gdbrc $$(cat gdbopts 2>/dev/null) $(EXEC)

tags: $(CFILES) $(HFILES)
	ctags $(CFILES) $(HFILES)

.PHONY: test
test: $(EXEC)
	./$(EXEC)
