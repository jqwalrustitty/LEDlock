/* -----------------------------------------------------------------------------
 * File         : src/winkbd.c
 * Description  : Windows Keyboard Routines
 * Copyright    : (c) Rodger Allen 2019
 * Licence      : BSD3
 * -------------------------------------------------------------------------- */

#include <windows.h>
#include <stdio.h>          // printf()
#include <unistd.h>         // usleep(), sleep()
//#include <time.h>           // nanosleep()

#include "winkbd.h"
#include "encoder.h"

/* -------------------------------------------------------------------------- */
/*
 *  The next two functions should be the only User32.dll call (maybe sleeper())
 */

/* -------------------------------------
 * Derive current ledstate
 */
trybble getKeyState()
{
    trybble t = 0;

    unsigned char keyState[256];
    GetKeyboardState(keyState);
    t += (keyState[VK_SCROLL])?__SCROLL__:0;
    t += (keyState[VK_CAPITAL])?__CAPITAL__:0;
    t += (keyState[VK_NUMLOCK])?__NUMLOCK__:0;

    return t;
}

/* -------------------------------------
 * Press and Release a key
 *    'key'         : a 'VK_*' code
 */
void pressRelease( int key )
{
    keybd_event(key, key + 0x80, KEYEVENTF_EXTENDEDKEY | 0, 0);
    keybd_event(key, key + 0x80, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
}

/* -------------------------------------
 *  Press the three LOCK keys to the trybble state
 *    'oldState'    : current state of keyboard
 *                    this is updated
 *    'newState'    : updated to this state
 *  returns         : newState
 *
 * TODO:  check the actual keystate first; but then what?
 * TODO:  send scroll last to signal change
 *        avoiding race condition (somewhat)
 * XXX:   implicit that the new trybble has an updated hi-bit
 */

trybble setKeys( trybble old, trybble new )
{
    trybble res = (old ^ new) & (__LOCKKEYS__);
    if (res&__NUMLOCK__) pressRelease(VK_NUMLOCK);
    if (res&__CAPITAL__) pressRelease(VK_CAPITAL);
    if (res&__SCROLL__)  pressRelease(VK_SCROLL);
    return new;
}

/* -------------------------------------
 * Flash a byte as sequence of trybbles
 *  'currState' : pointer to keyboard state as 'struct trybble'
 *                this state is updated
 *  'timer'     : milliseconds
 *  'b'         : a byte
 */
void flashByte( trybble *st, int timer, uint8_t b )
{
    tydbit *q = toTydbits(b);

#if DEBUG
    if (verbose>=2) {
        fprintf(stdout,">  %c  (%#04x)\t", isprint(b)?b:'_', b);
        showTydbitsArr(q); fprintf(stdout,"  ");
    }
#endif
    for (int j=0; j < TYDS ; j++) {
        *st = setKeys(*st, tydbitToTrybble(q[j], *st));
#if DEBUG
        if (verbose>=2) {
# if DEBUG > 2
            fprintf(stdout,"%s  ", showLEDstate(*st));
# else
            fprintf(stdout,"%d ",*st);
# endif
        }
#endif
        sleeper(timer);
    }
#if DEBUG
    if (verbose>=2) fprintf(stdout,"\n");
#endif
}

/* -------------------------------------
 * Flash all the bytes
 *  'currState' : pointer to keyboard state as 'struct ledstate'
 *  'timer'     : milliseconds
 *  'bs'        : array of bytes
 *  'sz'        : number of elements in 'bs'
 */
void flashBytes( trybble *currState, int timer, unsigned char *bs, int sz )
{
    for (int i=0; i<sz; i++)
        flashByte(currState, timer, bs[i]);
}

#if 0
/* -------------------------------------
 * Reset LED state
 *  'st'        : current state
 *
 *  completely redundant and unused
 */
trybble clearKeys( trybble st )
{
    trybble res = __SCROLL__;

    if (st != res) 
      res = setKeys(st, res);

#if DEBUG
    if (verbose>=2) fprintf(stderr,"clearKeys(): res=%#x st=%#x \n", res, st);
#endif

    return res;
}
#endif

/* -------------------------------------
 * Sleep for a bit
 *  'timer'     : in milliseconds
 */
#define MILLISECONDS 1000000
#define MILLIS       1000
void sleeper( int timer )
{
    //Sleep(timer);
    usleep( timer * MILLIS );
}

#if 0
#define NANOSECONDS 1000000000
#define NANOS       100000
void sleeper( int timer )
{
    struct timespec duration = {0,timer*NANOS};
    nanosleep(&duration,NULL);
}
#endif

// -----------------------------------------------------------------------------
// vi: et ts=4 ai
