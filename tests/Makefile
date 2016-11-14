# tests/Makefile
# File ID: 4906ed12-fb76-11dd-b5ce-000475e441b9

.PHONY: test
test:
	./Genlog *.t

.PHONY: clean
clean:
	rm -rf tmp-* *-stderr.tmp
	if test ! -e log/.git; then rm -rf log; fi
	cd conv-suuid-files && $(MAKE) clean
