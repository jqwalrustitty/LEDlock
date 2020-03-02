/* -----------------------------------------------------------------------------
 * File         : src/encoder.c
 * Description  : Encoding Trybbles
 * Copyright    : (c) Rodger Allen 2019
 * Licence      : BSD3
 * -------------------------------------------------------------------------- */

#include "encoder.h"

#include <inttypes.h>       // uint32_t, uint8_t
#include <stdio.h>          // printf()

/* -----------------------------------------------------------------------------
 *  TYDBITS
 *
 * Care should be taken to convert appropriately
 *    bytes <-> tydbits <-> ledstate
 *
 * A byte maps to four tydbits.
 *
 * A tydbit maps to an ledstate 
 * SCROLL is used as the parity bit
 * NUMLOCK and CAPS are used to encode a ledstate
 *    12 <-> *12
 * where, * is the parity bit
 *        12 are the bit positions
 */

/* -------------------------------------
 *  Convert byte to a quad of tydbits
 */
#if 1   // big-endian
enum tydNDX { _HI_, _HIMID_, _LOWMID_, _LOW_ };
#define TYDBIT(b,x) ((b & ((1<<(2*(3-x)+1)) + (1<<(2*(3-x))))) >> (2*(3-x)))
#else // little-endian
enum tydNDX { _LOW_, _LOWMID_, _HIMID_, _HI_ };
#define TYDBIT(b,x) ((b & ((1<<(2*x+1)) + (1<<(2*x)))) >> (2*x))
#endif
tydbit *    byteToTydbits( uint8_t z )
{
    static uint8_t res[TYDS] = { 0, 0, 0, 0 };
    res[0] = TYDBIT(z,_HI_);
    res[1] = TYDBIT(z,_HIMID_);
    res[2] = TYDBIT(z,_LOWMID_);
    res[3] = TYDBIT(z,_LOW_);
    return res;
}

/* -------------------------------------
 *  Convert a quad of tydbits to a byte
 */
#define bytefromTydbitsM(z) ((z[_HI_]<<6) + (z[_HIMID_]<<4) + (z[_LOWMID_]<<2) + (z[_LOW_]))
uint8_t     byteFromTydbits( tydbit z[TYDS] )
{
    return bytefromTydbitsM(z);
}

/* -------------------------------------
 *  Convert a ledstate to a tydbit
 */
#define stateToTydbitM(b) (b&(NUMLOCK^CAPITAL))
tydbit  stateToTydbit( ledstate st )
{
    return stateToTydbitM(st);
}

/* -------------------------------------
 *  Convert current ledstate and tydbit to a new ledstate
 */
#define stateFromTydbitM(t,s) (((s&SCROLL)^SCROLL)+t)
trybble stateFromTydbit( tydbit tyd, ledstate st )
{
    return stateFromTydbitM(tyd, st);
}

/* -------------------------------------
 * Show values of array of tydbits
 */
void showTydbitArr( tydbit z[4] )
{
    fprintf(stdout,"(");
    fprintf(stdout,"%d,", z[0]);
    fprintf(stdout,"%d,", z[1]);
    fprintf(stdout,"%d,", z[2]);
    fprintf(stdout,"%d", z[3]);
    fprintf(stdout,")");
}

/* -----------------------------------------------------------------------------
 *  KANA + TRYBBLES
 *
 * Care should be taken to convert appropriately
 *    bytes <-> trybbles <-> ledstate
 *
 * A byte maps to three trybbles, sacrificing one bit
 * The third trybble is aligned low for convenience
 * so the "reserved" but the third-kana bit.
 *
 * A trybble maps to an ledstate 
 * SCROLL is used as the parity bit
 * NUMLOCK,CAPS and KANA are used to encode a ledstate
 *    123 <-> 10*23
 * where, * is the parity bit
 *        0 is the reserved bit
 *        123 are the bit positions
 */

/* -------------------------------------
 *  Convert byte to triplet of trybbles
 */
#define TRYB0(b) ((b & (128+64+32)) >> 5)
#define TRYB1(b) ((b & (16+8+4)) >> 2)
#define TRYB2(b) ((b & (2+1)))
trybble * byteToTrybbles( uint8_t b )
{
    static uint8_t res[TRYBS] = { 0, 0, 0 };
    res[0] = TRYB0(b);
    res[1] = TRYB1(b);
    res[2] = TRYB2(b);
    return res;
}

/* -------------------------------------
 *  Convert a triplet of trybbles to a byte
 */
#define byteFromTrybblesM0(z)   ((z[0]<<5)+(z[1]<<2)+(z[2]))
#define byteFromTrybblesM(z) \
    (((z[0]&TRYBMASK)<<5)+((z[1]&TRYBMASK)<<2)+((z[2]&TRYBMASK)))
uint8_t byteFromTrybbles( trybble z[3] )
{
    return byteFromTrybblesM(z);
}

/* -------------------------------------
 *  Convert a ledstate to a trybble
 */
#define stateToTrybbleM(b) (((b&KANA)>>2)+(b&(CAPITAL^NUMLOCK)))
trybble stateToTrybble( ledstate st )
{
    return stateToTrybbleM(st);
}

/* -------------------------------------
 *  Convert current ledstate and trybble to a new ledstate
 */
#define stateFromTrybbleM(t,s) \
    ((t&NUMLOCK)+(t&CAPITAL)+((t&SCROLL)<<2)+((st&SCROLL)^SCROLL))
ledstate stateFromTrybble( trybble t, ledstate st )
{
    return stateFromTrybbleM(t, st);
}

/* -------------------------------------
 * Show values of array of trybbles
 */
void showTrybbleArr( trybble z[3] )
{
    fprintf(stdout,"(%d,%d,%d)", z[0],z[1],z[2]);
}

/* -------------------------------------
 * maybe useful?
 */
uint8_t countBits( uint8_t b )
{
    uint8_t c;
    c = b - ((b>>1)&0x55);
    c = ((c>>2)&0x33) + (c&0x33);
    c = ((c>>4) + c) & 0x0f;
    return c;
}

/* -----------------------------------------------------------------------------
 *  Unibit (Kana + Scroll)
 *
 * Care should be taken to convert appropriately
 *    bytes <-> unibits <-> ledstate
 *
 * A unibit maps to an ledstate 
 * SCROLL is used as the parity bit
 * KANA is used to encode a ledstate
 *    1 <-> 10*00
 * where, * is the parity bit
 *        0 is the reserved bit
 *        1 is the bit
 */

/* -------------------------------------
 *  Convert a quad of tydbits to a byte
 */
uint8_t     byteFromUbits( unibit z[8] )
{
    uint8_t b = 0;
    for(int i=0;i<UBITS;i++) {
        b += z[i]<<(UBITS-i-1);
    }
    return b;
}

/* -------------------------------------
 *  Convert byte to octet of unibits
 */
unibit *      byteToUbits( uint8_t z )
{
    static uint8_t res[UBITS] = { 0 };
    for(int i=0;i<UBITS;i++) {
        res[i] = (z >> (UBITS-i-1)) & 1;
    }
    return res;
}

/* -------------------------------------
 *  Convert a ledstate to a unibit
 */
unibit        stateToUbit( ledstate st )
{
    return((st&KANA)>>4);
}

/* -------------------------------------
 *  Convert current ledstate and unibit to a new ledstate
 */
ledstate    stateFromUbit( unibit t, ledstate st )
{
    return(((st&SCROLL)^SCROLL) + (t<<4));
}

/* -------------------------------------
 *  Convert byte to octet of unibits
 */
void showUbitArr( unibit z[UBITS] )
{
    fprintf(stdout,"(%d", z[0]);
    for(int i=1; i<UBITS; i++) {
        fprintf(stdout,",%d", z[i]);
    }
    fprintf(stdout,")");
}

/* -----------------------------------------------------------------------------
 * Strings of ledstate as "10101"
 */

/* -------------------------------------
 * Show ledstate as string of 1s and 0s
 */
#define onoff(x) (x?'1':'0')
char * showLEDs( ledstate t )
{
    static char str[10] = {0};
    snprintf(str,10,"%c0%c%c%c",
                            onoff(t&KANA),
                            onoff(t&SCROLL),
                            onoff(t&CAPITAL),
                            onoff(t&NUMLOCK));
    return str;
}

char * showLEDtyd( ledstate t )
{
    static char str[8] = {0};
    snprintf(str,8,"%1hu %c%c%c", (uint8_t) t,
                            onoff(t&SCROLL),
                            onoff(t&CAPITAL),
                            onoff(t&NUMLOCK));
    return str;
}

/* -----------------------------------------------------------------------------
 *  ENCODER CALLBACKS
 *
 *  Care should be taken to convert appropriately
 *    bytes <-> encoder <-> ledstate
 *
 *  The chief abstraction is the array of tydbits or trybbles, etc,
 *  that are manipulated by the encoder.
 *  These are all uint8_t, so not storage efficient.
 *  Some more in-place bitmasking would be good.
 *
 *  typedef struct encoder {
 *    'frames'    : number of parts byte is broken into
 *    'mask'      : mask of all encdoed bit (incl scroll)
 *    'byteTo'    : byte to encoded
 *    '*byteFrom' : byte form encoded array
 *    'stateTo'   : encoded element to ledstate
 *    'stateFrom' : ledstate to encoded element
 *    'showArr'   : print breakdown of encoding (debugging)
 *  }
 *
 */

encoder unibitEncoder = {
    UBITS, ULOCKKEYS,
    byteToUbits, byteFromUbits,
    stateToUbit, stateFromUbit,
    showUbitArr
};

encoder tydbitEncoder = {
    TYDS, LOCKKEYS,
    byteToTydbits, byteFromTydbits,
    stateToTydbit, stateFromTydbit,
    showTydbitArr
};

encoder trybbleEncoder = {
    TRYBS, KLOCKKEYS,
    byteToTrybbles, byteFromTrybbles,
    stateToTrybble, stateFromTrybble,
    showTrybbleArr
};

encoder nullEncoder = {
    0, 0, 0, 0, 0, 0, 0
};


// -----------------------------------------------------------------------------
// vi: et ts=4 ai
