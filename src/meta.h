/* -----------------------------------------------------------------------------
 * File         : src/meta.h
 * Description  : Protocol Metadata
 * Copyright    : (c) Rodger Allen 2019
 * Licence      : BSD3
 * -------------------------------------------------------------------------- */

#ifndef __META_H_
#define __META_H_

#include "global.h"
#include "encoder.h"
#include <inttypes.h>

/* -------------------------------------
 * Metadata definitions
 */
typedef struct metadata {
    uint8_t mask;
    uint8_t delay;
    uint16_t size;
    uint32_t crc;
} metadata;

// Implicit in metadata.size - 255 or 65535
#define MAXFILESIZE (0xffff)

/* -------------------------------------
 * Construct metadata from file
 */
struct metadata getFileH( FILE *fh, uint8_t **buf, encoder enc, int timer );

/* -------------------------------------
 * Metadata Manipulators
 */
int                 metaToBytes    ( uint8_t **buf, struct metadata *md );
int                 metaSize       ( struct metadata *md );
struct metadata     bytesToMeta    ( uint8_t *buf );
char *              showMetaData   ( metadata *md );
int                 metaSanity     ( struct metadata *md, encoder *enc );

#endif
// -----------------------------------------------------------------------------
// vi: et ts=4 ai
