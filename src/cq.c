/* -----------------------------------------------------------------------------
 * File         : src/cq.c
 * Description  : Detect "CQ" from stream of Trybbles
 * Copyright    : (c) Rodger Allen 2019
 * Licence      : BSD3
 * -------------------------------------------------------------------------- */

#include "cq.h"
#include "encoder.h"

#include <inttypes.h>
#include <stdio.h>          // FILE, fread()
#include <stdlib.h>         // exit()
#include <string.h>         // strlen()

#include <ctype.h>          // isprint()

/* -----------------------------------------------------------------------------
 * DECODER
 */

/* -------------------------------------
 * Decode a byte off the hid-stream
 *  'fh'    : file handle to hid-stream
 *
 * It requires that the parity is updated before 
 * processing the incoming frame.
 *
 */
uint8_t decodeChunkEnc( FILE *fh, encoder enc )
{
    uint8_t b = parity;
    uint8_t buf[8] = {0};
    size_t sz;

    VERBOSE(2) printf("[%d] ",parity);
    for (int i=0; i<(enc.frames); i++) {
        VERBOSE(2) printf("(");
        while (!((b&PARITY)^parity)) {
            if(feof(fh))exit(EXIT_FAILURE);
            sz =fread(&b, 1, 1, fh);
            VERBOSE(2) printf("%02x:",b);
            if (sz == 0) exit(EXIT_FAILURE);    // UGLY!!!
        }
        buf[i] = (enc.stateTo)(b);
        parity = b & PARITY;
        VERBOSE(2) printf(" %02x)",buf[i]);
    }
    VERBOSE(2) printf("\t");
    return (enc.byteFrom)(buf);
}

/* -------------------------------------
 * Receive and decode the file contents
 */
uint8_t * getPayload( FILE *fh, int fileSize, encoder enc )
{
    uint8_t *buf;
    buf = malloc(fileSize);
    for (int j=0 ; j < fileSize ; j++) {
        buf[j] = decodeChunkEnc(fh,enc);
        if (verbose>=2) printf(" %#04x %c\n",buf[j],isprint(buf[j])?buf[j]:'_');
        if ((verbose==1)&&(!(j%16))) printf(".");
    }
    return buf;
}

/* -----------------------------------------------------------------------------
 * DETECT ENCODING
 */

/* -------------------------------------
 * Pushes a character onto the buffer.
 * Old characters are pushed off the bottom.
 * 'buf'    : buffer
 * 'sz'     : size of buf
 * 'b'      : new character to push onto buffer
*/
void pushBuf( uint8_t *buf, int sz, uint8_t b )
{
    for (int i=1; i<sz; i++)
        buf[i-1] = buf[i];
    buf[sz-1] = b;
}

/* -------------------------------------
 * matches on {0,mask,0,mask}
 * return 1 == true
 *        0 == false
 */
uint8_t detectEncoding0( uint8_t *buf, int sz )
{
    uint8_t rv = 0;
    uint8_t pat0 = buf[0];
    uint8_t pat1 = buf[1];
    if ((pat0!=0) || (pat0==pat1)) return rv;
    for(int i=2;i<sz-1;i+=2){
        if ((buf[i]==pat0) && (buf[i+1]==pat1))
            continue;
        else
            return rv;
    }
    switch (pat1){
        case LOCKKEYS:
        case KLOCKKEYS:
        case ULOCKKEYS:
            rv = pat1;
            break;
        default:
            break;
    }
    return rv;
}
uint8_t detectEncoding( uint8_t *buf, int sz )
{
    uint8_t rv = 0;

    VERBOSE(2) printf("\tdetect: sz=%d buf[0]=0x%02x ",sz,buf[0]);
    if (buf[0]==0) return rv;
    for (int i=1;i<sz-2;i+=2) {
        if(buf[i]!=0) return rv;
        VERBOSE(2) printf("0");
    }
    for (int i=2;i<sz-1;i+=2) {
        if(buf[i]!=buf[0]) return rv;
        VERBOSE(2) printf("%d ",i);
    }
    switch (buf[0]){
        case LOCKKEYS:
        case KLOCKKEYS:
        case ULOCKKEYS:
            rv = buf[0];
            break;
        default:
            break;
    }
    return rv;
}

/* -------------------------------------
 *  Waits for a repetition of (zero+mask)
 */
//#define ENCBUF (2*INIT_REPEATS)
#define ENCBUF ((2*INIT_REPEATS)-1)
int waitForEncoding( FILE *fh, encoder *enc )
{
    int n = 0;
    uint8_t b = parity;
    uint8_t buf[ENCBUF] = { 0 };

    while(!feof(fh)) {
        VERBOSE(2) printf("[%d] (",parity);
        // *** wait for parity to change
        while (!((b&PARITY)^parity)) {
            n = fread(&b, 1, 1, fh);
            VERBOSE(2) printf("%2x:",b);
            if(n==0) return 0;
        }
        VERBOSE(2) printf(" %2x) ",b);
        // *** push onto buffer
        pushBuf(buf, ENCBUF, b);
        parity = b & PARITY;
        VERBOSE(3) showEncBuf(buf,ENCBUF);
        // *** check for signal mask
        switch(detectEncoding(buf, ENCBUF)){
            case LOCKKEYS:  *enc = tydbitEncoder;   return 1;
            case KLOCKKEYS: *enc = trybbleEncoder;  return 1;
            case ULOCKKEYS: *enc = unibitEncoder;   return 1;
            default:        break; 
        }
        VERBOSE(2) printf("\n");
    }
    // *** should never get here
    return 0;
}

/* -------------------------------------
 * Display the Enc buffer
 */
void showEncBuf( uint8_t *buf, int sz )
{
    fprintf(stdout,"\tcqbuf: ");
    for (int i=0; i<sz; i++)
        fprintf(stdout,"%02x ",buf[i]);
}

/* -----------------------------------------------------------------------------
 * CQ SANITY
 */

/* -------------------------------------
 * Test decode
 *  XXX: discards frames if no match
 */
int cqSanity( FILE *hid, encoder enc )
{
    int cqsz = MAGSZ;
    uint8_t cqBuf[MAGSZ] = MAGIX;
    uint8_t *inBuf = malloc(cqsz);
    inBuf = getPayload(hid, cqsz, enc);
    if (verbose) {
        fprintf(stdout,"cqBuf[%d]= ",cqsz);
        for(int j=0;j<cqsz;j++)
            printf("%#04x ",inBuf[j]);
        for(int j=0;j<cqsz;j++)
            printf("%c",isprint(inBuf[j])?inBuf[j]:'_');
        printf("\n");
    }
    int rv = eqBuf(inBuf,cqBuf,cqsz);
    free(inBuf);
    return rv;
}

/* -------------------------------------
 * equality between two buffers
 */
int eqBuf( uint8_t *a, uint8_t *b, int sz )
{
    for(int i=0;i<sz;i++)
        if(a[i]!=b[i]) return 0;
    return 1;
}

/* -----------------------------------------------------------------------------
 * LEGACY cq signature array generation
 */
/* -------------------------------------
 * Create the CQ signature array.
 * > int *cq0;
 * > cq0 = cqSignature("CQCQ",trybbleEncoder);
 */
uint8_t * cqSignature( const char *buf, encoder enc )
{
    int buflen = strlen(buf);
    uint8_t *tds = {0};
    uint8_t sz = buflen*(enc.frames);
    uint8_t *out = malloc(sz);
    bzero(out,sz);
    for (int i=0; i<buflen && i<sz; i++) {
        tds = (enc.byteTo)(buf[i]);
        for (int j=0; j<(enc.frames); j++)
            out[(enc.frames)*i+j] = tds[j];
    }
    return(out);
}

// -----------------------------------------------------------------------------
// vi: et ts=4 ai
