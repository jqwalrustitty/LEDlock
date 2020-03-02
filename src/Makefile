# ------------------------------------------------------------------------------
# File		: src/Makefile
# Copyright : (c) Rodger Allen 2019
# Licence 	: BSD3
# ------------------------------------------------------------------------------

CC=gcc
STRIP=strip
CFLAGS=-Wall -Wextra -O2

WinCC=i686-w64-mingw32-gcc
WinSTRIP=i686-w64-mingw32-strip
WinLIBS=-lws2_32

IMPLANTFLAGS=-DIMPLANT -DDEBUG=1
DAEMONFLAGS=-DLISTENER -DDEBUG=1

# ------------------------------------------------------------------------------

EXES=kbdinfil kbdexfil.exe

all: $(EXES)

.PHONY: clean
clean:
	rm -f $(EXFILobjs) $(INFILobjs) 2>/dev/null
	rm -f $(EXES) 2>/dev/null
	rm -f *.o *.wo 2>/dev/null
	@echo

# --------------------------------------

%.o: %.c %.h global.h
	$(CC) $(CFLAGS) $(DAEMONFLAGS) -c -o $@ $<

%.wo: %.c %.h global.h
	$(WinCC) $(CFLAGS) $(IMPLANTFLAGS) -c -o $@ $<

# --------------------------------------
EXFILobjs=encoder.wo meta.wo crc32.wo winkbd.wo

kbdexfil.exe: $(EXFILobjs) kbdexfil.wo
	$(WinCC) $(CFLAGS) $(IMPLANTFLAGS) $(WinLIBS) -o $@ $^
	$(WinSTRIP) $@
	@echo

# --------------------------------------
INFILobjs=encoder.o meta.o crc32.o cq.o filing.o

kbdinfil: $(INFILobjs) kbdinfil.o
	$(CC) $(CFLAGS) $(DAEMONFLAGS) -o $@ $^
	$(STRIP) $@
	@echo

# --------------------------------------
# *** pi only ***
.PHONY: install
install: kbdinfil
	exit
	#sudo systemctl daemon-reload
	#sudo systemctl stop kbdinfil
	#sudo cp kbdinfil /usr/local/sbin/
	#sudo systemctl start kbdinfil

# ------------------------------------------------------------------------------
# vi: ts=4 ai noet
