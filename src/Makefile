# Makefile for suuid.c
# File ID: 5cedd236-af56-11e6-92f6-279c2a0468a3
# Author: Øyvind A. Holm <sunny@sunbase.org>

CC = cc
EXEC = suuid
LD = cc
PREFIX = /usr/local

CFILES  =
CFILES += binbuf.c
CFILES += environ.c
CFILES += genuuid.c
CFILES += io.c
CFILES += logfile.c
CFILES += rcfile.c
CFILES += selftest.c
CFILES += sessvar.c
CFILES += strings.c
CFILES += suuid.c
CFILES += tag.c
CFILES += uuid.c
CFLAGS  =
CFLAGS += $$($(IS_DEV) && echo -O0 || echo -O2)
CFLAGS += $$(test -n "$(GCOV)" && echo -n "-fprofile-arcs -ftest-coverage")
CFLAGS += $$(test -n "$(PROF)" && echo -n "-pg")
CFLAGS += $(DEVFLAGS_STR)
CFLAGS += -Wall
CFLAGS += -c
DEPS = version.h $(HFILES) Makefile
DEVEL =
DEVFLAGS  =
DEVFLAGS += $$(test "$$(uname -o)" = "Android" || echo -fcf-protection=full)
DEVFLAGS += -Wbad-function-cast
DEVFLAGS += -Wcast-align
DEVFLAGS += -Wcast-qual
DEVFLAGS += -Wchar-subscripts
DEVFLAGS += -Wconversion
DEVFLAGS += -Wdangling-else
DEVFLAGS += -Wdate-time
DEVFLAGS += -Wdouble-promotion
DEVFLAGS += -Werror
DEVFLAGS += -Werror=implicit-function-declaration
DEVFLAGS += -Wextra
DEVFLAGS += -Wfloat-conversion
DEVFLAGS += -Wformat
DEVFLAGS += -Wformat-overflow
DEVFLAGS += -Wformat-security
DEVFLAGS += -Wformat-signedness
DEVFLAGS += -Wformat-truncation
DEVFLAGS += -Wformat-y2k
DEVFLAGS += -Winit-self
DEVFLAGS += -Wint-in-bool-context
DEVFLAGS += -Wlogical-not-parentheses
DEVFLAGS += -Wmisleading-indentation
DEVFLAGS += -Wmissing-braces
DEVFLAGS += -Wold-style-definition
DEVFLAGS += -Woverlength-strings
DEVFLAGS += -Wpedantic
DEVFLAGS += -Wredundant-decls
DEVFLAGS += -Wshadow
DEVFLAGS += -Wstrict-aliasing
DEVFLAGS += -Wstrict-overflow=5
DEVFLAGS += -Wstrict-prototypes
DEVFLAGS += -Wunused-const-variable
DEVFLAGS += -Wunused-function
DEVFLAGS += -fno-common
DEVFLAGS += -fstack-protector-strong
DEVFLAGS += -fwrapv
DEVFLAGS += -g3
DEVFLAGS += -pedantic
DEVFLAGS_STR = $$($(IS_DEV) && echo $(DEVFLAGS))
GNCOV_STR = $$(test -n "$(GNCOV)" && echo "-g")
HFILES  =
HFILES += binbuf.h
HFILES += suuid.h
HFILES += uuid.h
LDFLAGS  =
LDFLAGS += $$(test -n "$(PROF)" && echo -n "-pg")
LIBS  =
LIBS += $$(test -n "$(GCOV)" && echo "-lgcov --coverage")
TESTLOCKDIR = testlockdir

OBJS  =
OBJS += suuid.o

OBJS += binbuf.o
OBJS += environ.o
OBJS += genuuid.o
OBJS += io.o
OBJS += logfile.o
OBJS += rcfile.o
OBJS += selftest.o
OBJS += sessvar.o
OBJS += strings.o
OBJS += tag.o
OBJS += uuid.o

#  Not in Clang
# DEVFLAGS += -Wc99-c11-compat
# DEVFLAGS += -Wduplicated-branches
# DEVFLAGS += -Wformat-overflow=2
# DEVFLAGS += -Wjump-misses-init
# DEVFLAGS += -Wlogical-op

#  Don't use yet
# DEVFLAGS += -Waggregate-return
# DEVFLAGS += -Wformat=2
# DEVFLAGS += -Wmissing-declarations
# DEVFLAGS += -Wpadded

CMD_VALGRIND  = valgrind
CMD_VALGRIND += -q
CMD_VALGRIND += --leak-check=full
CMD_VALGRIND += --show-leak-kinds=all
CMD_VALGRIND += --
IS_DEV = [ -z "$(NODEVEL)" \
           -a \( -e .devel -o -e ../.git/.devel -o -n "$(DEVEL)" \) ]
TESTABLE_LINES = $$(grep -E '^ +\#\#\#\#\#:' *.c.gcov \
                    | grep -vF '/* gncov */' | wc -l)

.PHONY: all
all: $(EXEC)

$(EXEC): $(OBJS)
	$(LD) -o $(EXEC) $(LDFLAGS) $(OBJS) $(LIBS)

$(PREFIX)/bin/$(EXEC): $(EXEC)
	mkdir -p $(PREFIX)/bin
	install $(EXEC) $(PREFIX)/bin/$(EXEC)

version.h: Gen-version $(CFILES) $(HFILES) Makefile
	./Gen-version

suuid.o: version.h suuid.c $(DEPS)
	$(CC) $(CFLAGS) suuid.c

binbuf.o: binbuf.c $(DEPS)
	$(CC) $(CFLAGS) binbuf.c

environ.o: environ.c $(DEPS)
	$(CC) $(CFLAGS) environ.c

genuuid.o: genuuid.c $(DEPS)
	$(CC) $(CFLAGS) genuuid.c

io.o: io.c $(DEPS)
	$(CC) $(CFLAGS) io.c

logfile.o: logfile.c $(DEPS)
	$(CC) $(CFLAGS) logfile.c

rcfile.o: rcfile.c $(DEPS)
	$(CC) $(CFLAGS) rcfile.c

selftest.o: selftest.c $(DEPS)
	$(CC) $(CFLAGS) selftest.c

sessvar.o: sessvar.c $(DEPS)
	$(CC) $(CFLAGS) sessvar.c

strings.o: strings.c $(DEPS)
	$(CC) $(CFLAGS) strings.c

tag.o: tag.c $(DEPS)
	$(CC) $(CFLAGS) tag.c

tags: $(CFILES) $(HFILES)
	ctags $(CFILES) $(HFILES)

uuid.o: uuid.c $(DEPS)
	$(CC) $(CFLAGS) uuid.c

.PHONY: cflags
cflags:
	@echo $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(EXEC) $(OBJS)
	rm -f *.gcda *.gcno *.gcov
	rm -f gmon.out gmon.sum
	rm -f version.h
	rm -fr $(TESTLOCKDIR)
	cd t && $(MAKE) $@

.PHONY: dupdesc
dupdesc:
	@$(MAKE) -s clean all \
	&& ./$(EXEC) --selftest 2>/dev/null | cut -f 4- -d ' ' | sort \
	   | uniq -c | sed 's/^[[:space:]]*//g;' | grep -v '^1 ' \
	   | grep . && exit 1 || true

.PHONY: edit
edit: tags
	$(EDITOR) $$(git ls-files | grep -v -e ^COPYING)
	rm tags

.PHONY: gcov
gcov:
	$(MAKE) clean test GCOV=1
	gcov $(CFILES)
	@echo $$(grep -E '^ +#####:' *.c.gcov | wc -l) untested lines >&2
	@echo $$(grep -E '^ +#####:' *.c.gcov | grep -F '/* gncov */' \
	         | wc -l) gncov lines >&2
	@echo $(TESTABLE_LINES) testable lines >&2

.PHONY: gcov-cmt
gcov-cmt: gcov
	gcov-cmt $(GNCOV_STR) $(CFILES)

.PHONY: gcov-cmt-clean
gcov-cmt-clean:
	gcov-cmt -d $(CFILES)

.PHONY: gdb
gdb: $(EXEC)
	gdb -x gdbrc $$(cat gdbopts 2>/dev/null) $(EXEC)

.PHONY: install
install: $(PREFIX)/bin/$(EXEC)

.PHONY: longlines
longlines:
	@for f in $(CFILES) $(HFILES) Gen-version Makefile; do \
		[ -f "$$f" ] && expand "$$f" | sed 's/ $$//;' \
		| grep -q -E '.{80}' && echo "$$f"; \
	done | grep . && exit 1 || true

.PHONY: test
test: $(EXEC)
	./$(EXEC) --selftest

.PHONY: testall
testall:
	$(MAKE) -s testcomb WHAT=testold
	$(MAKE) -s testcomb WHAT=test
	$(MAKE) -s testsrc

.PHONY: testold
testold: $(EXEC)
	cd t && $(MAKE) test

.PHONY: testoldall
testoldall:
	$(MAKE) testcomb WHAT=testold

.PHONY: testcomb
testcomb:
	$(MAKE) clean $(WHAT) FAKE_HOST=1
	$(MAKE) clean $(WHAT) NDEBUG=1
	$(MAKE) clean $(WHAT) TEST_FUNC=1
	$(MAKE) clean $(WHAT) UNUSED=1
	$(MAKE) clean $(WHAT) VERIFY_UUID=1
	$(MAKE) clean $(WHAT) FAKE_HOST=1 NDEBUG=1 \
	                      TEST_FUNC=1 UNUSED=1 VERIFY_UUID=1
	$(MAKE) clean $(WHAT)

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
testlock: $(EXEC)
	rm -rf $(TESTLOCKDIR)
	mkdir $(TESTLOCKDIR)
	for f in $$(seq 1 500); do ./$(EXEC) -wn -l $(TESTLOCKDIR); done &
	for f in $$(seq 1 500); do ./$(EXEC) -wn -l $(TESTLOCKDIR); done &
	for f in $$(seq 1 500); do ./$(EXEC) -wn -l $(TESTLOCKDIR); done

.PHONY: testsrc
testsrc:
	@echo Check files for long lines
	@$(MAKE) -s longlines
	@echo Check for duplicated test descriptions
	@$(MAKE) -s dupdesc

.PHONY: tlok
tlok:
	@$(MAKE) -s gcov DEVEL=1 2>.make-tlok.tmp >&2; res=$$?; \
	if test $$res -eq 0; then \
		printf '%u:%u\n' \
		       "$(TESTABLE_LINES)" \
		       "$$(grep ^1\\.\\. .make-tlok.tmp \
		           | perl -pe 's/^1\.\.(\d+).*/$$1/;')"; \
	else \
		grep "Looks like you failed" <.make-tlok.tmp \
		| perl -p \
		  -e  's/.*failed (\d+) tests? of (\d+).*/$$1\/$$2 FAIL/;'; \
	fi; \
	[ -e $(EXEC) ] || echo FAIL; \
	rm .make-tlok.tmp

.PHONY: uninstall
uninstall:
	rm -f $(PREFIX)/bin/$(EXEC)

.PHONY: valgrind
valgrind: $(EXEC)
	./$(EXEC) --valgrind
	$(CMD_VALGRIND) ./$(EXEC) --selftest | grep -v ^ok
	cd t && $(MAKE) $@

.PHONY: valgrindall
valgrindall:
	$(MAKE) testcomb WHAT=valgrind
