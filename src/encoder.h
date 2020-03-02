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
 */
typedef uint8_t ledstate;
typedef uint8_t trybble;
typedef uint8_t tydbit;
typedef uint8_t unibit;

/* -------------------------------------
 * TRYBBLES
 * TODO: cleanup
 */
// The masks as they appear in HID standard
#define NUMLOCK         0x01    // (1<<0)
#define CAPITAL         0x02    // (1<<1)
#define SCROLL          0x04    // (1<<2)
#define _COMPOSE_       0x08    // (1<<3)
#define KANA            0x10    // (1<<4)
#define _POWER_         0x20    // (1<<5)
#define _SHIFT_         0x40    // (1<<6)
#define _DO_NOT_DISTURB_    0x80    // (1<<7)

#define LOCKKEYS        (SCROLL^CAPITAL^NUMLOCK)
#define TYDMASK         (CAPITAL^NUMLOCK)

#define KLOCKKEYS       (KANA^SCROLL^CAPITAL^NUMLOCK)
#define TRYBMASK        (SCROLL^CAPITAL^NUMLOCK)

#define ULOCKKEYS       (KANA^SCROLL)
#define UBITMASK        (KANA)

// The alternating bit
// XXX: KANA would be preferable, but doesn't seem to force the change
#define PARITY SCROLL

/* -------------------------------------
 * Tydbits-per-byte and Trybbles-per-byte
 */
#define TYDS  4
#define TRYBS 3
#define UBITS 8

/* -------------------------------------
 *  SHOW LEDSTATE
 */
char *      showLEDs( ledstate st );
char *      showLEDtyd( ledstate t );

/* -------------------------------------
 * TYDBITS PLUS SCROLL (ORIGINAL ENCODING)
 */
uint8_t     byteFromTydbits( tydbit z[TYDS] );
tydbit *    byteToTydbits( uint8_t z );

tydbit      stateToTydbit( ledstate st );
trybble     stateFromTydbit( tydbit tyd, ledstate st );

void        showTydbitArr( tydbit z[TYDS] );

/* -------------------------------------
 *  KANA PLUS TYDBITS PLUS SCROLL
 */
uint8_t     byteFromTrybbles( trybble z[TRYBS] );
trybble *   byteToTrybbles( uint8_t z );

trybble     stateToTrybble( ledstate st );
ledstate    stateFromTrybble( trybble t, ledstate st );

void        showTrybbleArr( unsigned char z[TRYBS] );

/* -------------------------------------
 *  KANA PLUS SCROLL
 */

uint8_t     byteFromUbits( unibit z[UBITS] );
unibit *    byteToUbits( uint8_t z );

unibit      stateToUbit( ledstate st );
ledstate    stateFromUbit( unibit t, ledstate st );

void        showUbitArr( unibit z[UBITS] );

/* -------------------------------------
 *  CALLBACKS
 */
//typedef uint8_t (*callback)(uint8_t, uint8_t);
typedef struct encoder {
    uint8_t   frames;
    uint8_t   mask;
    uint8_t * (*byteTo)   (uint8_t);
    uint8_t   (*byteFrom) (uint8_t *);
    uint8_t   (*stateTo)  (uint8_t);
    uint8_t   (*stateFrom)(uint8_t, uint8_t);
    void      (*showArr)  (uint8_t *);  
} encoder;

encoder unibitEncoder;
encoder tydbitEncoder;
encoder trybbleEncoder;
encoder nullEncoder;

#endif
// -----------------------------------------------------------------------------
// vi: et ts=4 ai
