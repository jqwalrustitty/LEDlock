# ------------------------------------------------------------------------------
# File		: src/Makefile
# Copyright : (c) Rodger Allen 2019
# Licence 	: BSD3
# ------------------------------------------------------------------------------

WinCC=i686-w64-mingw32-gcc
WinSTRIP=i686-w64-mingw32-strip
WinLIBS=-lws2_32

CC=gcc
STRIP=strip
CFLAGS=-Wall -Wextra -O3
# TODO: revisit -On flags
# The "-On" flags seemed to optimise out stuff to the point
# it didn't work at all. Doesn't really need any speed
# optimisations, and "strip" makes the binary smaller.

IMPLANTFLAGS=-DIMPLANT -DDEBUG=1
DAEMONFLAGS=-DLISTENER -DDEBUG=1

# ------------------------------------------------------------------------------

EXES=kbdinfil kbdexfil.exe

all: $(EXES)

.PHONY: clean real-clean
clean:
	rm -f $(EXFILobjs) $(INFILobjs) 2>/dev/null
	rm -f $(DEBUGobjs) $(EXFDBGobjs) 2>/dev/null

real-clean: clean
	rm -f $(EXES) 2>/dev/null
	@echo

# --------------------------------------

%.o: %.c
	$(CC) $(CFLAGS) $(DAEMONFLAGS) -c -o $@ $<

%.wo: %.c
	$(WinCC) $(CFLAGS) $(IMPLANTFLAGS) -c -o $@ $<

%.exe: %.c
	$(WinCC) $(CFLAGS) $(WinLIBS) -o $@ $<
	$(WinSTRIP) $@
	@echo

# --------------------------------------
EXFILobjs=kbdexfil.wo encoder.wo meta.wo crc32.wo winkbd.wo

kbdexfil.exe: $(EXFILobjs)
	$(WinCC) $(CFLAGS) $(WinLIBS) $(IMPLANTFLAGS) -o $@ $^
	$(WinSTRIP) $@
	@echo

# --------------------------------------
INFILobjs=kbdinfil.o encoder.o meta.o crc32.o cq.o filing.o

kbdinfil: $(INFILobjs)
	$(CC) $(CFLAGS) $(DAEMONFLAGS) -o $@ $^
	$(STRIP) $@
	@echo


# ------------------------------------------------------------------------------
# vi: ts=4 ai noet
