# README

A project to exfiltrate data from a Windows host to a Raspberry Pi Zero
emulating a USB keyboard, by flashing the LEDs.

## Getting started

- src is in [src](src)
- doc is in [doc](doc)
- scripts and configs are in [scripts](scripts)

Start with the docs.

## Prereqs

- A Raspberry Pi Zero
- mingw32
- pandoc (for the docs)
  - again, start with the docs
  - unapologetically, pandoc markdown

## TODOs

Higher priority:
- error-correction and/or synchronous and/or race conditions
- cq detection (cq0 and lastTrybble/INITSTATE)
- cleanup verbosity and debugging
- general bugfixes

Lesser priority:
- streamline implant vs daemon dependencies
- daemon signal handling
- file-less implant
- remove IPv4 from Pi build (because!)

## Licencing

- Copyright is Rodger Allen (2018-2020)

- Code is BSD3

- Docs are CC BY-SA 4.0


<!--
# vi: et ts=2 ai
-->
