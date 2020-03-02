/* -----------------------------------------------------------------------------
 * File         : src/cq.h
 * Description  : Detect "CQ" from stream of Trybbles
 * Copyright    : (c) Rodger Allen 2019
 * Licence      : BSD3
 * -------------------------------------------------------------------------- */

#ifndef __CQ_H__
#define __CQ_H__

#include "global.h"
#include "encoder.h"    // TYDS, TRYBS
#include <stdio.h>      // FILE

/* -------------------------------------
 * DECODER
 */
uint8_t decodeChunkEnc( FILE *fh, encoder enc );
uint8_t * getPayload( FILE *fh, int fileSize, encoder enc );

/* -------------------------------------
 * DETECT ENCODING
 */
int waitForEncoding( FILE *fh, encoder *enc );
uint8_t detectEncoding( uint8_t *buf, int sz );
void pushBuf( uint8_t *buf, int sz, uint8_t b );
void showEncBuf( uint8_t *buf, int size );

/* -------------------------------------
 * CQ SANITY
 */
int cqSanity( FILE *hid, encoder enc );
int eqBuf( uint8_t *a, uint8_t *b, int sz );

/* -------------------------------------
 * LEGACY cq signature array generation
 */
uint8_t * cqSignature( const char *buf, encoder enc );


#endif
// -----------------------------------------------------------------------------
// vi: et ts=4 ai
