/* -----------------------------------------------------------------------------
 * File         : src/encoder.c
 * Description  : Encoding Trybbles
 * Copyright    : (c) Rodger Allen 2019
 * Licence      : BSD3
 * -------------------------------------------------------------------------- */

#include "encoder.h"

#include <inttypes.h>       // uint32_t, uint8_t
#if DEBUG
#include <stdio.h>          // printf()
#endif

/* -----------------------------------------------------------------------------
 *  CONVERSION ROUTINES
 *  Basically just wrappers around macros for the typing.
 */
#define fromTydbitsM(z) ((z[_HI_]<<6) + (z[_HIMID_]<<4) + (z[_LOWMID_]<<2) + (z[_LOW_]))
#define trybbleToTydbitM(b) (b&(__NUMLOCK__^__CAPITAL__))
#define tydbitToTrybbleM(t,s) (((s&__SCROLL__)^__SCROLL__)+t)

/* -------------------------------------
 *  byte <--> tydbits[]
 *
 *  Effectively the macros are:
 *
 *  uint8_t fromTydbits( tydbit[TYDS] );
 */

tydbit * toTydbits( uint8_t z )
{
    static uint8_t res[TYDS] = { 0, 0, 0, 0 };
    res[0] = TYDBIT(z,_HI_);
    res[1] = TYDBIT(z,_HIMID_);
    res[2] = TYDBIT(z,_LOWMID_);
    res[3] = TYDBIT(z,_LOW_);
    return res;
}

uint8_t fromTydbits( tydbit z[TYDS] )
{
    return fromTydbitsM(z);
}

/* -------------------------------------
 * trybble <--> tydbit
 *
 *  Effectively the macros are:
 *
 *  tydbit trybbleToTydbit( trybble t );
 *  trybble tydbitToTrybble( tydbit, trybble );
 *
 *  The transform tybdit --> trybble
 *  requires the current state.
 */

tydbit  trybbleToTydbit( trybble tryb )
{
    return trybbleToTydbitM(tryb);
}

trybble tydbitToTrybble( tydbit tyd, trybble tryb)
{
    return tydbitToTrybbleM(tyd,tryb);
}

/* -----------------------------------------------------------------------------
 *  Debugging
 */
#if DEBUG

#ifdef IMPLANT
/* -------------------------------------
 *
 */
void showTydbitsArr( unsigned char z[4] )
{
    fprintf(stdout,"(");
    fprintf(stdout,"%d,", z[0]);
    fprintf(stdout,"%d,", z[1]);
    fprintf(stdout,"%d,", z[2]);
    fprintf(stdout,"%d", z[3]);
    fprintf(stdout,")");
}
#endif

/* -------------------------------------
 *
 */
#define onoff(x) (x?'1':'0')
char * showLEDstate( trybble t )
{
    static char str[8] = {0};
    snprintf(str,8,"%1hu %c%c%c", (uint8_t) t,
                            onoff(t&__SCROLL__),
                            onoff(t&__CAPITAL__),
                            onoff(t&__NUMLOCK__));
    return str;
}
#endif

// -----------------------------------------------------------------------------
// vi: et ts=4 ai
