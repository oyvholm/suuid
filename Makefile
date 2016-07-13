#!/bin/bash

#=======================================================================
# Makefile
# File ID: c9f9bc1a-28d8-11e5-b53c-fefdb24f8e10
#
# Author: Øyvind A. Holm <sunny@sunbase.org>
# License: GNU General Public License version 2 or later.
#=======================================================================

.PHONY: default
default:
	cd src && make

.PHONY: testnew
testnew: test
	cd src && make testnew

.PHONY: test
test:
	cd src && make test
	cd tests && make

.PHONY: test
clean:
	rm -fv synced.sqlite.20*.bck
	cd src && make clean
	cd tests && make clean
