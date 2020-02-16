/* -----------------------------------------------------------------------------
 * File         : src/cq.h
 * Description  : Detect "CQ" from stream of Trybbles
 * Copyright    : (c) Rodger Allen 2019
 * Licence      : BSD3
 * -------------------------------------------------------------------------- */

#ifndef __CQ_H_
#define __CQ_H_

#include "global.h"
#include <stdio.h>      // FILE

/* -------------------------------------
 * 
 */
#define CQSIZE (MAGSZ * TYDS)

uint8_t decodeChunk( FILE *fh );

int waitForCQ( FILE *fh );
int eqCQ( uint8_t *buf );
void pushCQ( uint8_t *buf, uint8_t b );

void showCQbuf( uint8_t *buf );

#if DEBUG
int * cqSignature( const char *buf);
#endif

#endif
// -----------------------------------------------------------------------------
// vi: et ts=4 ai
