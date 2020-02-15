/* -----------------------------------------------------------------------------
 *
 * File         : src/global.h
 * Description  : Global variables and hardcoded whatnots
 * Copyright    : (c) Rodger Allen 2019
 * Licence      : BSD3
 */
// -----------------------------------------------------------------------------

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

// most of the printf()s are for "verbose"
//#include <stdio.h>          // printf()
#include <inttypes.h>


// -------------------------------------
// Versioning
#define REVISION "0.4.1"

// -------------------------------------
// Protocol

// TODO: sort out the MAGIX
#define MAGIX "CQCQ"
#define MAGSZ 4

// -------------------------------------
// Defaults

// default delay timer for implant
#define DEFTIMER 20

// default hid device for listener (unused)
#define DEFDEVICE "/dev/hidg0"

// default store location for listener
#define DEFOUTDIR "/var/exfil"

// default (dup2) 'verbose' log location
#define DEBUGLOG "/var/exfil/debug.log"

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
extern uint8_t lastTrybble;


// -------------------------------------
// Values from standard sources
/*
Usage ID    Usage ID    Usage Name
 (Dec)       (Hex)

  57        0x39        Keyboard Caps Lock
  71        0x47        Keyboard Scroll Lock
  83        0x53        Keypad Num Lock and Clear
 130        0x82        Keyboard Locking Caps Lock
 131        0x83        Keyboard Locking Num Lock
 132        0x84        Keyboard Locking Scroll Lock
*/

#define __VK_NUMLOCK__      0x90
#define __VK_CAPITAL__      0x14
#define __USB_SCROLL__      0x91


// -------------------------------------
// Junk

#if 0
#define xREVISION "$Revision: 2.18 $"
char revision[32] = {0};
void setRevision()
{
    char *ver = xREVISION;
    for (int i=0; i<(int)strlen(ver)-11 && ver[i+11]!=' '; i++)
        revision[i] = ver[i+11];
}
#endif

#endif
// -----------------------------------------------------------------------------
// vi: et ts=4 ai
