/* -----------------------------------------------------------------------------
 * File         : src/encoder.h
 * Description  : Encoding Trybbles
 * Copyright    : (c) Rodger Allen 2019
 * Licence      : BSD3
 * -------------------------------------------------------------------------- */

#ifndef __ENCODER_H__
#define __ENCODER_H__

#include "global.h"
#include <inttypes.h>       // uint8_t

/* -------------------------------------------------------------------------- */

/* -------------------------------------
 *  TYPES
 *  A 'tydbits' is two-bits
 *  A 'trybble' is three-bits
 *  Trybbles are also used to store the 'state' of the LEDs
 */
typedef uint8_t tydbit;
typedef uint8_t trybble;

/* -------------------------------------
 * TYDBITS
 */
#define TYDS 4

// big-endian
#define TYDBIT(b,x) ((b & ((1<<(2*(3-x)+1)) + (1<<(2*(3-x))))) >> (2*(3-x)))
#define _HI_        0
#define _HIMID_     1
#define _LOWMID_    2
#define _LOW_       3

#if 0
// little-endian
#define TYDBIT(b,x) ((b & ((1<<(2*x+1)) + (1<<(2*x)))) >> (2*x))
#define _HI_        3
#define _HIMID_     2
#define _LOWMID_    1
#define _LOW_       0
#endif

/* -------------------------------------
 * TRYBBLES
 */
// The mask as they appear in HID standard
#define __NUMLOCK__         0x01
#define __CAPITAL__         0x02
#define __SCROLL__          0x04
#define __LOCKKEYS__        (__SCROLL__^__CAPITAL__^__NUMLOCK__)
#define __TYDMASK__         (__CAPITAL__^__NUMLOCK__)

// Initialise LEDs to this state
#define INITSTATE __SCROLL__

#if 0
#define __MASK_SCROLL__     (__LOCKKEYS__ ^ __SCROLL__)
#define __MASK_CAPITAL__    (__LOCKKEYS__ ^ __CAPITAL__)
#define __MASK_NUMLOCK__    (__LOCKKEYS__ ^ __NUMLOCK__)
#define __MASK__            __MASK_SCROLL_
#endif

/* -------------------------------------
 *  CONVERSION ROUTINES
 */
uint8_t     fromTydbits( tydbit z[TYDS] );
tydbit *    toTydbits( uint8_t z );

tydbit  trybbleToTydbit( trybble t );
trybble tydbitToTrybble( tydbit tyd, trybble tryb );

/* -------------------------------------
 * DEBUGGING
 */
#if DEBUG
#ifdef IMPLANT
void showTydbitsArr( unsigned char z[4] );
#endif
char * showLEDstate( trybble t );
#endif

#endif
// -----------------------------------------------------------------------------
// vi: et ts=4 ai
