# README

A project to exfiltrate data from a Windows host to a Raspberry Pi Zero
emulating a USB keyboard, by flashing the LEDs.

## Getting started

- src is in [src](src)
- doc is in [doc](doc)
- scripts are in [scripts](scripts)
- not sure what's in [pi](pi)

Start with the docs.

## Prereqs

- A Raspberry Pi Zero
- mingw32
- pandoc (for the docs)
  - again, start with the docs

## TODOs

Higher priority:
- streamline implant vs daemon dependencies
- cleanup verbosity and debugging
- file-less implant
- error-correction and/or synchronous and/or race conditions

Lesser priority:
- remove IPv4 from Pi build (because!)
- cq detection (cq0 and lastTrybble/INITSTATE)

## Licencing

- Copyright is Rodger Allen (2018-2020)

- Code is BSD3

- Docs are CC BY-SA 4.0


<!--
# vi: et ts=2 ai
-->
