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
#ifndef VK_KANA
#define VK_KANA         0x15
#endif

// -------------------------------------

void sleeper( int timer );
void pressRelease( int key );
ledstate getKeyState();
ledstate setKeys( ledstate oldState, ledstate newState );

void flashBytes( ledstate *st, int timer, unsigned char *bs, int sz, encoder enc );
void flashByte( ledstate *st, int timer, uint8_t b, encoder enc );

ledstate sendInit( ledstate currState, int timer, encoder enc );

#if DEBUG
void FauxGetKeyboardState( unsigned char *buf );
#endif

void resetLEDs();

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
