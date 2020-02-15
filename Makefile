# ------------------------------------------------------------------------------
#
# File		: Makefile
#
# Copyright : (c) Rodger Allen 2019-2020
# Licence 	: BSD3
#
# ------------------------------------------------------------------------------

REVISION=0.4.1

.PHONY: kbdexfil kbdinfil doc clean
all: kbdinfil kbdexfil doc

kbdexfil:
	(cd src/ && make kbdexfil.exe)

kbdinfil:
	(cd src/ && make kbdinfil)

doc:
	(cd doc/ && make)

clean:
	(cd src/ && make clean)
	(cd doc/ && make clean)

package:
	darcs dist --dist-name ledlock-darcs-$(REVISION)


# ------------------------------------------------------------------------------
# vi: ts=4 ai noet
