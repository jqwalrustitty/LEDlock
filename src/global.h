/* -----------------------------------------------------------------------------
 * File         : src/global.h
 * Description  : Global variables and hardcoded whatnots
 * Copyright    : (c) Rodger Allen 2019
 * Licence      : BSD3
 * -------------------------------------------------------------------------- */

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

// most of the printf()s are for "verbose"
//#include <stdio.h>          // printf()
#include <inttypes.h>

// -------------------------------------
// Versioning
#define REVISION "0.4.8"

// -------------------------------------
// Protocol

// Iterations to send encoder CQ
#define INIT_REPEATS 4

// TODO: sort out the MAGIX
#define MAGIX "CQCQ"
#define MAGSZ 4

// -------------------------------------
// Defaults

// default delay timer for implant
#define DEFTIMER 2

// default hid device for listener (unused)
#define DEFDEVICE "/dev/hidg0"

// default store location for listener
#define DEFOUTDIR "/var/exfil"

// default (dup2) 'verbose' log location
#define DEBUGLOG "/var/exfil/debug.log"

// timeout to (re)open hidg
#define RETRY_TIMEOUT 5

// -------------------------------------
// Debugging and verbosity

// TODO: repurpose or remove a lot of the 'verbose' code
#ifndef DEBUG
#define DEBUG 0
#endif

// verbosity flag
extern int verbose;
#define VERBOSE(d) if(verbose>=d)

// last seen trybble (the __SCROLL__ bit)
extern uint8_t parity;


// -------------------------------------
// Values from standard sources

// _KEY_        | _USB_       | _keybb_event()          | USB_HID codes
// NUM_LOCK     | 0x01  00001 |   VK_NUMLOCK  0x90  144 |   0x53
// CAPS_LOCK    | 0x02  00010 |   VK_CAPITAL  0x14   20 |   0x39
// SCROLL_LOCK  | 0x04  00100 |   VK_SCROLL   0x91  145 |   0x47
// KANA         | 0x10  10000 |   VK_KANA     0x15   21 |   0x92

#endif
// -----------------------------------------------------------------------------
// vi: et ts=4 ai
