# src/t/Makefile
# File ID: f539e628-4346-11e6-9e79-02010e0a6634

.PHONY: test
test: build
	./Genlog suuid.t

.PHONY: all
all:
	./Genlog *.t

.PHONY: build
build:
	cd .. && $(MAKE)

.PHONY: clean
clean:
	rm -rf tmp-* *-stderr.tmp
	if test ! -e log/.git; then rm -rfv log; fi
