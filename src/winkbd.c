/* -----------------------------------------------------------------------------
 * File         : src/winkbd.c
 * Description  : Windows Keyboard Routines
 * Copyright    : (c) Rodger Allen 2019
 * Licence      : BSD3
 * -------------------------------------------------------------------------- */

//#include <windows.h>
#include <stdio.h>          // printf()
#include <unistd.h>         // usleep(), sleep()
#include <time.h>           // nanosleep()
#include <stdlib.h>         // rand()
#include <ctype.h>          // isprint()
#include <inttypes.h>

#include "winkbd.h"
#include "encoder.h"

/* -------------------------------------------------------------------------- */
/*
 *  The next two functions should be the only User32.dll call (maybe sleeper())
 */

/* -------------------------------------
 * Press and Release a key
 *    'key'         : a 'VK_*' code
 *
 * XXX: I can't remember what the second param in keydb_event means
 *      Possibly the usb scan code?
 */
void pressRelease( int key )
{
    keybd_event(key, key + 0x80, KEYEVENTF_EXTENDEDKEY | 0, 0);
    keybd_event(key, key + 0x80, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
}

/* -------------------------------------
 * Derive current ledstate
 */
ledstate getKeyState()
{
    ledstate t = 0;

    unsigned char keyState[256];
    GetKeyboardState(keyState);
    //FauxGetKeyboardState(keyState);
    t += (keyState[VK_SCROLL])?SCROLL:0;
    t += (keyState[VK_CAPITAL])?CAPITAL:0;
    t += (keyState[VK_NUMLOCK])?NUMLOCK:0;
    t += (keyState[VK_KANA])?KANA:0;

    return t;
}

/* -------------------------------------
 * Generate a fake keyboard state (for testing)
 */
#if DEBUG
void FauxGetKeyboardState( unsigned char *buf )
{
    (void)srand((unsigned int)time(NULL));
    for (int i=0; i<256; i++)
        buf[i]  = (unsigned char)(0x01 & rand());
}
#endif

/* -------------------------------------------------------------------------- */

/* -------------------------------------
 * Press the three LOCK keys to the trybble state
 *  'old'   : current state of keyboard (this is updated)
 *  'new'   : updated to this state
 *  returns : 'new'
 *
 * Implicit that the new ledstate has an updated hi-bit
 * This function does not flip the parity bit
 *
 * TODO:  check the actual keystate first; but then what?
 */

ledstate setKeys( ledstate old, ledstate new )
{
#if 0
    trybble origState = getKeyState();
    if (origState != old) {
        VERBOSE(2) fprintf(stderr,"\nsetKeys(1) %#x %#x\n",origState,old);
    }
#endif
    VERBOSE(2) fprintf(stdout,"| %s ", showLEDs(new));
    ledstate res = (old ^ new) & (KLOCKKEYS);
    if (res&KANA)     pressRelease(VK_KANA);
    if (res&NUMLOCK)  pressRelease(VK_NUMLOCK);
    if (res&CAPITAL)  pressRelease(VK_CAPITAL);
    if (res&SCROLL)   pressRelease(VK_SCROLL);
    //VERBOSE(3) fprintf(stdout,"{%s} ", showLEDs(res));
    return new;
}

/* -------------------------------------
 * Flash a single tydbit
 *  'st'    : current state (this state is updated)
 *  'timer' : milliseconds
 *  'new'   : a trybble
 */
void flashTrybble( ledstate *st, int timer, ledstate new )
{
    *st = setKeys(*st, new); sleeper(timer);
}

/* -------------------------------------
 * Flash a byte as sequence of trybbles
 *  'st'    : current state (this state is updated)
 *  'timer' : milliseconds
 *  'b'     : a byte
 */
void flashByte( ledstate *st, int timer, uint8_t b, encoder enc )
{
    uint8_t *q = (enc.byteTo)(b);
    if (verbose>=2) {
        fprintf(stdout,">  %c  (%#04x)\t", isprint(b)?b:'_', b);
        //(enc.showArr)(q); fprintf(stdout,"  ");
    }
    for (int j=0; j < (enc.frames) ; j++) {
        (void) flashTrybble(st, timer, (enc.stateFrom)(q[j], *st));
        //VERBOSE(3) { (enc.showArr)(st); fprintf(stdout,"  "); }
    }
    VERBOSE(2) fprintf(stdout,"\n");
}

/* -------------------------------------
 * Flash all the bytes
 *  'currState' : pointer to keyboard state as 'struct ledstate'
 *  'st'        : current state (this state is updated)
 *  'timer'     : milliseconds
 *  'bs'        : array of bytes
 *  'sz'        : number of elements in 'bs'
 */
void flashBytes( ledstate *st, int timer, unsigned char *bs, int sz, encoder enc )
{
    for (int i=0; i<sz; i++)
        flashByte(st, timer, bs[i], enc);
}

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

/* -------------------------------------
 * Initialise the LEDs for an encoding
 *  resets the state to zero
 *  repeats the mask and zero
 *  ends on the mask
 *
 *  effectively repeats (zero+mask) INIT_REPEATS times
 *  NB, two flashes each time
 */
ledstate sendInit( ledstate st, int in_timer, encoder enc )
{
    ledstate zero = 0;
    int timer = 2 * in_timer;
    //ledstate reset = (st&PARITY)?st:((st&PARITY)^PARITY);
    //st = setKeys(st, (enc.stateFrom)(zero,reset)); sleeper(timer);
    st = setKeys(st, zero);             sleeper(timer);
    st = setKeys(st, SCROLL);           sleeper(timer);
    st = setKeys(st, zero);             sleeper(timer);
    for(int i=0;i<INIT_REPEATS-1;i++) {
        st = setKeys(st, (enc.mask));   sleeper(timer);
        st = setKeys(st, zero );        sleeper(timer);
    }
    st = setKeys(st, (enc.mask));       sleeper(timer);
    VERBOSE(2) printf("| init=%s\n", showLEDs(st));
    return st;
}

// -----------------------------------------------------------------------------

/* -------------------------------------
 * Reset the LEDs - properly and fully
 * - set led state to zero
 *
 * TODO:
 * - get original state,
 *   do operations,
 *   end by setting to original
 */

void resetLEDs()
{
    encoder enc = trybbleEncoder;
    ledstate zero = 0;

    ledstate orig = getKeyState();
    VERBOSE(2) printf("| keystate=%s\n", showLEDs(orig));

    ledstate reset = (orig&PARITY)?orig:((orig&PARITY)^PARITY);
    VERBOSE(2) printf("| reset=%s\n", showLEDs(reset));

    ledstate st = setKeys(orig, (enc.stateFrom)(zero,reset)); sleeper(25);
    VERBOSE(2) printf("| state=%s\n", showLEDs(st));

    ledstate final = getKeyState();
    VERBOSE(2) printf("| final=%s (0x%02x)\n", showLEDs(final), (final&st));
}



// -----------------------------------------------------------------------------
// vi: et ts=4 ai
