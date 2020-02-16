# ------------------------------------------------------------------------------
# File			: doc/Makefile
# Copyright		: (c) Rodger Allen 2019
# Licence      	: BSD3
#
# For documentation and slides
# ------------------------------------------------------------------------------

# --------------------------------------
# Beamer Slides Options
THEME=Szeged
COLORTHEME=crane
FONTTHEME=structurebold

BEAMEROPTS=-t beamer
#BEANEROPTS=--verbose
BEAMEROPTS+=-V theme:$(THEME)
BEAMEROPTS+=-V colortheme:$(COLORTHEME)
BEAMEROPTS+=-V fonttheme:$(FONTTHEME)
#BEAMEROPTS+=-V section-titles:false
BEAMEROPTS+=-V aspectratio:43

A4=-V papersize:a4

# --------------------------------------

all: slides paper.pdf
	@echo

etal: speaker.pdf morse

clean:
	rm -f *.pdf 2>/dev/null
	rm -f morse 2>/dev/null
	@echo

# --------------------------------------

.PHONY=slides
slides: slides.pdf
slides.pdf: slides.md
	pandoc -s $(BEAMEROPTS) -o $@ $<

# --------------------------------------

# Beamer pandoc
slides-%.pdf: %.md
	pandoc -s $(BEAMEROPTS) -o $@ $<

# Generic pandoc
%.pdf: %.md
	pandoc -s $(A4) -o $@ $<

%.html: %.md
	pandoc -s -o $@ $<

# --------------------------------------

morse: morse.c
%: %.c
	cc -Wall -o $@ $<

# ------------------------------------------------------------------------------
# vi: ts=4 ai noet
