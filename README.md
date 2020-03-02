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
  - unapologetically, pandoc markdown

## TODOs

Higher priority:
- error-correction
- race conditions and/or timings
- improve encoding detection (cq and lastTrybble)
- general bugfixes
- guarantees on init of LED state

Lesser priority:
- `make test package install`
- file-less implant
- cleanup verbosity and debugging
- split implant vs daemon dependencies
- daemon signal handling
- write out raw hidg0 stream to file

Wish list:
- fully generic arbitrary encodings
- other usb-otg device emulation

## Licencing

- Copyright is Rodger Allen (2018-2020)

- Code is BSD3

- Docs are CC BY-SA 4.0


<!--
# vi: et ts=2 ai
-->
