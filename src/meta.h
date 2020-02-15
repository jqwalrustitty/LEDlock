/* -----------------------------------------------------------------------------
 *
 * File         : src/meta.h
 * Description  : Protocol Metadata
 * Copyright    : (c) Rodger Allen 2019
 * Licence      : BSD3
 *
 * -------------------------------------------------------------------------- */

#ifndef __META_H_
#define __META_H_

#include "global.h"
#include <inttypes.h>   // uint32_t

/* -------------------------------------
 * Metadata definitions
 */

typedef struct metadata {
    uint16_t size;
    uint32_t crc;
} metadata;

// Implicit in metadata.size
//#define MAXFILESIZE 255
//#define MAXFILESIZE 65535
#define MAXFILESIZE ((0x100 << (8*(sizeof(uint16_t)-1))) - 1)


// TODO: sort out the MAGIX
uint8_t magix[4];


/* -------------------------------------
 * Utilities
 */

#ifdef IMPLANT
struct metadata getFile( const char *filename, uint8_t **buf );
void test_getFile( char *filename );
int metaToBytes( uint8_t **buf, struct metadata *md );
void showMetadata( metadata *md );
#endif


#ifdef LISTENER
int metaSize( struct metadata *md );                    // kbdinfil
struct metadata bytesToMeta( uint8_t *buf );            // kbdinfil
#endif


#if DEBUG
//void showMetadata( metadata *md );                      // kbdexfil,kbdinfil
void showMetadataStr( char * str, int sz, metadata *md );   // kbdinfil
void showMetadataRaw( metadata *md );

int metaSanity( struct metadata *md );
#endif


#endif
// -----------------------------------------------------------------------------
// vi: et ts=4 ai
