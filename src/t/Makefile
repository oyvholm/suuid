# src/t/Makefile
# File ID: f539e628-4346-11e6-9e79-02010e0a6634

.PHONY: test
test: build
	./suuid.t

.PHONY: all
all:
	./suuid.t
	./sess.t

.PHONY: build
build:
	cd .. && $(MAKE)

.PHONY: clean
clean:
	rm -f gmon.out gmon.sum
	rm -rf *.tmp .tmp-* rc-macaddr tmp-*
	if test ! -e log/.git; then rm -rf log; fi

.PHONY: valgrind
valgrind: build
	./suuid.t --valgrind
