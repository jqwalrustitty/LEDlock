/* -----------------------------------------------------------------------------
 *
 * File         : src/meta.c
 * Description  : Protocol Metadata
 * Copyright    : (c) Rodger Allen 2019
 * Licence      : BSD3
 *
 * -------------------------------------------------------------------------- */

#include <stdio.h>          // FILE, fopen(), fclose(), fread(), ...
#include <stdlib.h>         // malloc(), exit()
#include <string.h>         // strlen(), memcpy()

#include "meta.h"
#include "crc32.h"

// -------------------------------------
// TODO: sort out the MAGIX
uint8_t magix[] = { 'C', 'Q', 'C', 'Q' };

/* -------------------------------------------------------------------------- */
/*
 * Converting bytes to/from metadata
 */

/* -------------------------------------
 *
 */
int metaSize( struct metadata *md )
{
    return (sizeof(md->size) + sizeof(md->crc));
}

/* -------------------------------------
 *
 */
int metaToBytes( uint8_t **buf, struct metadata *md )
{
    int sz = metaSize(md);
    *buf = malloc(sz);
    memcpy(*buf,                    (char *) &md->size, sizeof(md->size));
    memcpy(*buf + sizeof(md->size), (char *) &md->crc, sizeof(md->crc));
    return sz;
}

/* -------------------------------------
 *
 */
struct metadata bytesToMeta( uint8_t *buf )
{
    struct metadata md;
    memcpy(&(md.size), buf,                     sizeof(md.size));
    memcpy(&(md.crc), (buf + sizeof(md.size)),  sizeof(md.crc));
    return md;
}

/* -------------------------------------------------------------------------- */
/*
 * Create metadata for files
 */

/* -------------------------------------
 *
 */
struct metadata getFile( const char *filename, uint8_t **buf )
{
    struct metadata md = {0};
    size_t sz = 0;
    FILE *fh = fopen(filename,"rb");
    if (fh==NULL) return md;
    fseek(fh, 0, SEEK_END);
    sz = ftell(fh);
    rewind(fh);
    if (sz >= MAXFILESIZE) {
        fprintf(stderr,"ERR: max file size exceeded: %d > %d\n",(int)sz,MAXFILESIZE);
        return md;
    }
    *buf = malloc(sz);
    fread(*buf, sz, 1, fh);
    md.size = sz;
    md.crc = rc_crc32(0, *buf, sz);
    fclose(fh);
    return md;
}


/* -------------------------------------------------------------------------- */
#if DEBUG
/* -------------------------------------
 * not sure yet what sanity means
 */
int metaSanity( struct metadata *md )
{
    if (md->size == 0)
        return 0;
    return 1;
}
#endif

/* -------------------------------------------------------------------------- */
/*
 * Display some of the metadata
 */

/* -------------------------------------
 *
 */
#ifdef IMPLANT
void showMetadata( metadata *md )
{
    fprintf(stdout,"size: %u\ncrc %#x\n", md->size, md->crc);
    //fprintf(stdout,"size: %u\n", md->size);
    //fprintf(stdout,"crc: %#x\n", md->crc);
}
#endif

#if DEBUG
/* -------------------------------------
 *
 */
void showMetadataStr( char * str, int sz, metadata *md )
{
    snprintf(str,sz,"size:%u, crc:%#x",
            md->size, md->crc);
}

/* -------------------------------------
 *
 */
void showMetadataRaw( metadata *md )
{
    unsigned int i;
    char filesz[sizeof(md->size)];
    char crc32[sizeof(md->crc)];

    memcpy(filesz, (char *) & md->size, sizeof(md->size));
    memcpy(crc32,  (char *) & md->crc,  sizeof(md->crc));

    for (i=0; i<sizeof(md->size); i++)
        fprintf(stdout,"%c", filesz[i]);

    for (i=0; i<sizeof(md->crc); i++)
        fprintf(stdout,"%c", crc32[i]);
}
#endif

// -----------------------------------------------------------------------------
// vi: et ts=4 ai
