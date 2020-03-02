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
#include <ctype.h>          // isprint()
#include <sys/stat.h>       // stat()

#include "filing.h"
#include "meta.h"

/* -----------------------------------------------------------------------------
 *  FILENAME GENERATION
 */

/* -------------------------------------
 * Concatenates pathname + filename
 */
char * newFileName( const char *pathname, char *filename )
{
    char *filestamp;
    int sz = strlen(pathname)+1 + strlen(filename)+1;
    filestamp = malloc(sz);
    snprintf(filestamp, sz, "%s/%s", pathname, filename);
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
    snprintf(stamp, sz, "%ld", epoch);
    return stamp;
}

#define FMT32 "0x%08x"
#define FMT32old "%#010x"
char * crcStamp( struct metadata md )
{
    int sz = 2 + 2*sizeof(md.crc) + 1;
    char *stamp = malloc(sz+1);
    snprintf(stamp, sz, FMT32, md.crc);
    return stamp;
}

char * sizeStamp( struct metadata md )
{
    int sz = 5+1+1;       // 16-bit to decimal + 'b' (+1 to avoid compiler warnings)
    char *stamp = malloc(sz+1);
    snprintf(stamp,sz,"%ub", md.size);
    return stamp;
}

/* -------------------------------------
 *  Create filename from stamps
 *  'md'        : metadata
 *  'extras'    : extra string to go after main stamps
 *  'ext'       : file extension (dot '.' is inserted)
 */
char * mkStamp( struct metadata md, char *extras, char *ext )
{
    char *stamp;
    char *stampE = epochStamp();
    char *stampC = crcStamp(md);
    char *stampS = sizeStamp(md);
    int stamplen = strlen(stampE) + 1 + strlen(stampC) + 1 + strlen(stampS) + 1;
    stamplen += strlen(extras) + 1;
    stamplen += strlen(ext) + 1;
    stamp = malloc(stamplen);
    snprintf(stamp,stamplen,"%s-%s-%s-%s.%s",stampE,stampC,stampS,extras,ext);
    free(stampS);
    free(stampC);
    free(stampE);
    return stamp;
}

/* -----------------------------------------------------------------------------
 * FILE UTILITIES
 */

/* -------------------------------------
 * Write 'buffer' to file.
 *      'md'    : metadata
 *      'buf'   : data buffer (size derived from 'md'
 *      'newname'  : filename (incl path)
 */
int writeContents( struct metadata md, uint8_t *buf, char *newname )
{
    FILE *fh;
    int n = 0;
    int size = md.size;

    fh = fopen(newname, "wb");
    if (fh != NULL) {
        for (int i=0 ; i < size; i++) {
            // TODO: check if this fails
            n += fwrite(buf+i, 1, 1, fh);
        }
    } else {
        fprintf(stderr, "WARN: Failed write to %s\n\n", newname);
        syslog(LOG_LOCAL7|LOG_WARNING,"> Failed write to %s: %s", newname, strerror(errno));
        return 1;
    }
    fclose(fh);
    return 0;
}

/* -------------------------------------
 * Poor man's hexdump
 */
void showFileData( uint8_t *buf, int size )
{
    fprintf(stdout,">>>>\n");
    for (int j=0 ; j < size; j++) {
        //fprintf(stdout,"%c", buf[j]);
        //fprintf(stdout,"%c", isprint(buf[j])?buf[j]:'.');
        //fprintf(stdout,"%s", (j%16)?"":"\n");
        //fprintf(stdout,"%c%s", isprint(buf[j])?buf[j]:'.', (j%16)?"":"\n");
        fprintf(stdout,"0x%02x %s",buf[j],((j+1)%16)?"":"\n");
    }
    fprintf(stdout,"<<<<\n");
}

/* -------------------------------------
 * Check output directory exists
 *  - create if not?
 *  return: 0 on success
 */
#ifndef CREATEDIR
#define CREATEDIR 0
#endif
int check_dir( const char *path )
{
    struct stat sb;
    int rv;

    //return stat(path, &sb) == 0 && S_ISDIR(sb.st_mode);

    rv = stat(path, &sb);
    if (rv != 0) {
        syslog(LOG_LOCAL7|LOG_ERR,"Cannot stat directory %s : %s", path, strerror(errno));
        return rv;
    }
    if (S_ISDIR(sb.st_mode) != 1) {
        syslog(LOG_LOCAL7|LOG_ERR,"Invalid output directory %s", path);
        return 1;
    }

#if CREATEDIR
    // create directory (needs to go into the rv check from stat()
    rv = mkdir(path, 0x755);
    if (rv != 0) {
        syslog(LOG_LOCAL7|LOG_ERR,"Error creating output dir %s : %s", path, strerror(errno));
        syslog(LOG_LOCAL7|LOG_ERR,"    rv=%d, errno=%d", rv, errno);
        return errno;
    }
#endif
    //syslog(LOG_LOCAL7|LOG_INFO,"Using output dir %s", path);
    return 0;
}

// -----------------------------------------------------------------------------
// vi: et ts=4 ai
