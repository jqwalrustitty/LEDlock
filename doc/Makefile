# ------------------------------------------------------------------------------
#
# File			: doc/Makefile
#
# Copyright		: (c) Rodger Allen 2019
# Licence      	: BSD3
#
# For documentation and slides
#
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
# aspectratio - aspect  ratio of slides
# (for beamer only, 1610 for 16:10, 169 for 16:9, 149 for 14:9, 141 for
# 1.41:1, 54 for 5:4, 43 for 4:3 which is the default, and 32 for 3:2)
# [(5.0,4.0),(4.0,3.0),(141.0,100.0),(3.0,2.0),(14.0,9.0),(16.0,10.0),(16.0,9.0)]

A4=-V papersize:a4

# --------------------------------------

all: slides etal
	@echo

etal: paper.pdf speaker.pdf morse

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

# Generic pandoc
%.html: %.md
	pandoc -s -o $@ $<

# --------------------------------------

morse: morse.c
%: %.c
	cc -Wall -o $@ $<

# ------------------------------------------------------------------------------
# vi: ts=4 ai noet
