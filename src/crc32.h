/* -----------------------------------------------------------------------------
 *
 * File         : src/crc32.h
 * Description  : CRC32
 * Copyright    : https://rosettacode.org/wiki/CRC-32#C
 * Licence      : https://rosettacode.org/wiki/CRC-32#C
 *
 */
// -----------------------------------------------------------------------------

#ifndef __CRC32_H__
#define __CRC32_H__

#include <inttypes.h>
#include <stdlib.h>

uint32_t rc_crc32( uint32_t crc, const uint8_t *buf, size_t len );

#endif
// -----------------------------------------------------------------------------
// vi: et ts=4 ai
