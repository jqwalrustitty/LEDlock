/* -----------------------------------------------------------------------------
 * File         : src/cq.c
 * Description  : Detect "CQ" from stream of Trybbles
 * Copyright    : (c) Rodger Allen 2019
 * Licence      : BSD3
 * -------------------------------------------------------------------------- */

#include "cq.h"
#include "encoder.h"        // toTydbits()

#include <inttypes.h>
#include <stdio.h>          // FILE, fread()
#include <stdlib.h>         // exit()
#include <string.h>         // strlen()

#include <ctype.h>          // isprint()

/* -------------------------------------------------------------------------- */

/* -------------------------------------
 * Decode a byte off the hid-stream
 *  'fh'    : file handle to hid-stream
 *
 * It requires that the lastTrybble is updated before 
 * processing the incoming tydbit.
 */
uint8_t decodeChunk( FILE *fh )
{
    uint8_t b = lastTrybble;
    tydbit buf[TYDS] = {0};
    size_t sz;

    VERBOSE(2) printf("[%d] ",lastTrybble);
    for (int i=0; i<TYDS; i++) {
        VERBOSE(2) printf("(");
        while (!((b&__SCROLL__)^lastTrybble)) {
            sz =fread(&b, 1, 1, fh);
            VERBOSE(2) printf("%d",b);
            if (sz == 0) exit(EXIT_FAILURE);    // UGLY!!!
        }
        buf[i] = trybbleToTydbit(b);
        lastTrybble = b & __SCROLL__;
        VERBOSE(2) printf(" %d)",buf[i]);
    }
    VERBOSE(2) printf("\t");
    return fromTydbits(buf);
}

/* -------------------------------------------------------------------------- */

/* -------------------------------------
 * Defines the Tydbits of "CQCQ" used to detect incoming message
 * TODO: Should be generated dynamically from MAGIX
*/
static
int cq0[] = { 0x01, 0x00, 0x00, 0x03,
              0x01, 0x01, 0x00, 0x01,
              0x01, 0x00, 0x00, 0x03,
              0x01, 0x01, 0x00, 0x01 };

/* -------------------------------------
 * Assuming input buf is CQSIZE in length
 * return 1 == true
 *        0 == false
 */
int eqCQ( uint8_t *buf )
{
    for (int i=0; i<CQSIZE; i++) {
        if (cq0[i] != (buf[i] & __TYDMASK__))
            return 0;
    }
    return 1;
}

/* -------------------------------------
 * Pushes a character onto the cq buffer.
 * Old characters are pushed off the bottom.
 * 'buf'    : cq buffer
 * 'b'      : new character to push onto cq buffer
*/
void pushCQ( uint8_t *buf, uint8_t b )
{
    for (int i=1; i<CQSIZE; i++) {
        buf[i-1] = buf[i];
    }
    buf[CQSIZE-1] = b;
}

/* -------------------------------------
 * Listens on a filehandle and waits for the CQ signature
 * 'fh'   : filehandle
 * return 1 == true (found CQ)
 *        0 == false (haven't found CQ yet)
 *
 * XXX: it is possible to get severely out of state
 *      with the lastTrybble.
 *      maybe add a counter to reset lastTrybble?
 */
int waitForCQ( FILE *fh )
{
    int n = 0;
    uint8_t b = lastTrybble;
    uint8_t cqbuf[CQSIZE] = { 0 };
    int counter = 0;

    for (int i=0; !feof(fh); i++) {
    VERBOSE(2) printf("[%d] (",lastTrybble);
        while (!((b&__SCROLL__)^lastTrybble)) {
            n = fread(&b, 1, 1, fh);
            VERBOSE(2) printf("%d",b);
        }
        VERBOSE(2) printf(" %d)\t",b);
        if (n!=0) {
            pushCQ(cqbuf, b);
            VERBOSE(3) showCQbuf(cqbuf);
            lastTrybble = b & __SCROLL__;
            if (eqCQ(cqbuf) == 1) {
                return 1;
            }
        }
        VERBOSE(2) printf("\n");

        // FIXME:
        if (!(++counter % 1009)) {
            lastTrybble ^= lastTrybble;
            printf("#");
        }
    }
    return 0;
}

/* -------------------------------------
 * Display the CQ buffer
 */
void showCQbuf( uint8_t *buf )
{
    fprintf(stdout,"cqbuf: ");
    for (int i=0; i<CQSIZE; i++) {
        fprintf(stdout,"%d ",buf[i]);
    }

    uint8_t bs[MAGSZ] = {0};
    uint8_t b;
    fprintf(stdout,"\t");
    for (int i=0; i<MAGSZ; i++) {
        for (int j=0; j<TYDS; j++)
            bs[j] = buf[i*TYDS + j] & __TYDMASK__;
        b = fromTydbits(bs);
        fprintf(stdout,"%c ",isprint(b)?b:'.');
    }
}

#if DEBUG
/* -------------------------------------
 * Create the CQ signature array.
 * > int *cq0;
 * > cq0 = cqSignature("CQCQ");
 */
int * cqSignature( const char *buf )
{
    int buflen = strlen(buf);
    uint8_t *tbs = {0};
    static int out[MAGSZ*TYDS] = {0};

    for (int i=0; i<buflen && i<(MAGSZ*TYDS); i++) {
        tbs = toTydbits(buf[i]);
        for (int j=0; j<MAGSZ; j++) {
            out[MAGSZ*i+j] = tbs[j];
        }
    }
    return(out);
}
#endif

// -----------------------------------------------------------------------------
// vi: et ts=4 ai
