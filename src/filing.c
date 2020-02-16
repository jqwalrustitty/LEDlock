/* -----------------------------------------------------------------------------
 * File         : src/filing.c
 * Description  : File Operations for Daemon
 * Copyright    : (c) Rodger Allen 2019
 * Licence      : BSD3
 * -------------------------------------------------------------------------- */

#include <stdio.h>          // FILE, fread()
#include <string.h>         // strlen()
#include <stdlib.h>         // malloc(),free()
#include <syslog.h>         // syslog()
#include <errno.h>          // errno
#include <time.h>           // time()

#include "filing.h"
#include "meta.h"

/* -------------------------------------------------------------------------- */
/*  File Utilities
 */

/* -------------------------------------
 * Write 'buffer' to file.
 *      'md'    : metadata
 *      'buf'   : data buffer (size derived from 'md'
 *      'path'  : path to outdir
 *      'flags' : alter behavior of 'namestamp'
 */
int writeContents( struct metadata md, unsigned char *buf, char *path, int flags )
{
    FILE *fh;
    int n = 0;
    int size = md.size;
    char *stamp = mkStamp(md);
    char *newname = newFileName(path, (flags & FLAGCRC)?"CRC.bin":"OK.bin", stamp);

    fh = fopen(newname, "wb");
    if (fh != NULL) {
        for (int i=0 ; i < size; i++) {
            // TODO: check if this fails
            n += fwrite(buf+i, 1, 1, fh);
        }
    } else {
        fprintf(stderr, "WARN: Failed write to %s\n\n", newname);
        syslog(LOG_LOCAL7|LOG_WARNING,"  > Failed write to %s: %s", newname, strerror(errno));
    }
    fprintf(stdout,"NOTICE: Wrote %d bytes to \"%s\"\n", n, newname);
    syslog(LOG_LOCAL7|LOG_NOTICE,"Wrote %s", newname);
    fclose(fh);

    free(stamp);
    free(newname);
    return 1;
}

/* -------------------------------------------------------------------------- */

/* -------------------------------------
 * Filename for output of received file
 * NB, essentially just adds a timestamp
 */
char * newFileName( char *pathname, char *filename, char *stamps )
{
    char *filestamp;
    int sz = strlen(pathname)+1 + strlen(stamps)+1 + strlen(filename) + 1;

    filestamp = malloc(sz);
    snprintf(filestamp, sz, "%s/%s-%s", pathname, stamps, filename);
    return filestamp;
}

/* -------------------------------------
 *  Filename tags
 */
char * epochStamp( )
{
    int sz = 10+1;        // 32-bit time to decimal
    char *stamp = malloc(sz+1);
    time_t epoch = time(NULL);
    snprintf(stamp,sz,"%ld",epoch);
    return stamp;
}

char * crcStamp( struct metadata md )
{
    char *fmt32 = "%#010x";
    char *fmt16 = "%#06x";
    int len = 2 + 2*sizeof(md.crc) + 1;
    char *stamp = malloc(len+1);

    if (sizeof(md.crc)==sizeof(uint16_t))
        snprintf(stamp, len, fmt16, md.crc);
    else snprintf(stamp, len, fmt32, md.crc);
    return stamp;
}

char * sizeStamp( struct metadata md )
{
    int sz = 5+1+1;       // 16-bit to decimal + 'b' (+1 to avoid compiler warnings)
    char *stamp = malloc(sz+1);
    snprintf(stamp,sz,"%ub", md.size);
    return stamp;
}

char * mkStamp( struct metadata md )
{
    char *stamp;
    char *stampE = epochStamp();
    char *stampC = crcStamp(md);
    char *stampS = sizeStamp(md);
    int stamplen = strlen(stampE) + 1 + strlen(stampC) + 1 + strlen(stampS) + 1;
    stamp = malloc(stamplen);
    snprintf(stamp,stamplen,"%s-%s-%s",stampE,stampC,stampS);
    free(stampS);
    free(stampC);
    free(stampE);
    return stamp;
}

/* -------------------------------------
 * Utility
 */
void showFileData( unsigned char *buf, int size )
{
    fprintf(stdout,">>>>\n");
    for (int j=0 ; j < size; j++) {
        fprintf(stdout,"%c", buf[j]);
    }
    fprintf(stdout,"<<<<\n");
}

// -----------------------------------------------------------------------------
// vi: et ts=4 ai
