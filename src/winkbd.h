/* -----------------------------------------------------------------------------
 * File         : winkbd.h
 * Description  : Windows Keyboard Routines
 * Copyright	: (c) Rodger Allen 2019
 * Licence		: BSD3
 * -------------------------------------------------------------------------- */

#ifndef __WINKBD_H__
#define __WINKBD_H__

#include <windows.h>
#include <inttypes.h>       // uint32_t, uint8_t

#include "global.h"
#include "encoder.h"

// -------------------------------------

#ifndef VK_NUMLOCK
#define VK_NUMLOCK      0x90
#endif
#ifndef VK_CAPTIAL
#define VK_CAPITAL      0x14
#endif
#ifndef VK_SCROLL
#define VK_SCROLL       0x91
#endif

// -------------------------------------

void sleeper( int timer );
trybble getKeyState();
trybble setKeys( trybble oldState, trybble newState );

void flashBytes( trybble *currState, int timer, uint8_t *bs, int sz );
void flashByte ( trybble *currState, int timer, uint8_t b );

//trybble clearKeys( trybble st );

// -------------------------------------
#if 0
// holdover from <windows.h>
typedef unsigned char BYTE, *PBYTE, *LPBYTE
typedef unsigned char UINT8;

typedef int BOOL;
#ifndef FALSE
#define FALSE   0
#endif
#ifndef TRUE
#define TRUE    1
#endif
#endif

#endif
// -----------------------------------------------------------------------------
// vi: et ts=4 ai
