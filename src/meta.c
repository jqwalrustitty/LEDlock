/* -----------------------------------------------------------------------------
 * File         : src/meta.c
 * Description  : Protocol Metadata
 * Copyright    : (c) Rodger Allen 2019
 * Licence      : BSD3
 * -------------------------------------------------------------------------- */

#include <stdio.h>          // FILE, fopen(), fclose(), fread(), ...
#include <stdlib.h>         // malloc(), exit()
#include <string.h>         // strlen(), memcpy()
#include <inttypes.h>

#include "meta.h"
#include "crc32.h"

/* -----------------------------------------------------------------------------
 * Metadata Manipulators
 */

/* -------------------------------------
 *  Size of the metadata header
 */
int metaSize( struct metadata *md )
{
    return (  sizeof(md->mask) + sizeof(md->delay)
            + sizeof(md->size) + sizeof(md->crc));
}

/* -------------------------------------
 * Convert metadata struct to string of bytes
 * returns number of bytes
 */
int metaToBytes( uint8_t **buf, struct metadata *md )
{
    int sz = metaSize(md);
    int inc = 0;
    *buf = malloc(sz);
    memcpy(*buf + inc, (uint8_t *) &md->crc,   sizeof(md->crc));    inc += sizeof(md->crc);
    memcpy(*buf + inc, (uint8_t *) &md->size,  sizeof(md->size));   inc += sizeof(md->size);
    memcpy(*buf + inc, (uint8_t *) &md->delay, sizeof(md->delay));  inc += sizeof(md->delay);
    memcpy(*buf + inc, (uint8_t *) &md->mask,  sizeof(md->mask));   inc += sizeof(md->mask);
    return sz;
}

/* -------------------------------------
 * Convert string of bytes to a metadata struct
 */
struct metadata bytesToMeta( uint8_t *buf )
{
    struct metadata md;
    int inc = 0;
    memcpy(&(md.crc),   buf + inc,  sizeof(md.crc));    inc += sizeof(md.crc);
    memcpy(&(md.size),  buf + inc,  sizeof(md.size));   inc += sizeof(md.size);
    memcpy(&(md.delay), buf + inc,  sizeof(md.delay));  inc += sizeof(md.delay);
    memcpy(&(md.mask),  buf + inc,  sizeof(md.mask));   inc += sizeof(md.mask);
    return md;
}

/* -------------------------------------
 * Metadata to string
 */
char * showMetaData( metadata *md )
{
    static char str[128] = {0};
    snprintf(str,128,"mask=%s delay=%d size=%u crc=0x%#08x",
                  showLEDs(md->mask), md->delay, md->size, md->crc);
    return str;
}

/* -------------------------------------
 *  Check validity of metadata
 *  - returns 0 if false
 *  - crc can only be zero if filesize is zero
 *  - reject if md mask doesn't match enc mask
 */
int metaSanity ( struct metadata *md, encoder *enc )
{
    if (md->crc==0 && md->size!=0)
        return 0;
    if (md->crc!=0 && md->size==0)
        return 0;
    if (md->mask != enc->mask)
        return 0;
    //if (md->size == 0) return 0;
    return 1;
}
/* -----------------------------------------------------------------------------
 * Construct metadata from file
 */

/* -------------------------------------
 * Read from a file-handle and create metadata
 *  'fh'        : file handle (can be stdin)
 *  'buf'       : ptr to buffer of file payload 
 *  returns     : metadata of file (size,crc)
 */
struct metadata getFileH( FILE *fh, uint8_t **buf, encoder enc, int timer )
{
    struct metadata md = {0};
    size_t sz = 0;
    uint8_t inbuf[MAXFILESIZE] = {0};
    size_t n = 0;

    fseek(fh, 0, SEEK_END);
    sz = ftell(fh);
    rewind(fh);
    if ((int)sz == -1) {
        VERBOSE(1)fprintf(stderr,"using stdin\n");
        for(sz=0;sz<MAXFILESIZE&&(!feof(stdin)); sz++) {
            n = fread(&(inbuf[(int)sz]),1,1,stdin);
            if(n==0) break;
        }
        *buf = malloc(sz);
        memcpy(*buf,inbuf,sz);
    } else {
        if (sz > MAXFILESIZE) {
            fprintf(stderr,"ERR: max file size exceeded: %d > %d\n",(int)sz,MAXFILESIZE);
            return md;
        }
        *buf = malloc(sz);
        fread(*buf, sz, 1, fh);
    }
    md.mask = enc.mask;
    md.delay = timer;
    md.size = sz;
    md.crc = rc_crc32(0, *buf, sz);
    return md;
}


// -----------------------------------------------------------------------------
// vi: et ts=4 ai
