/* -----------------------------------------------------------------------------
 *
 * File         : src/filing.h
 * Description  : File Operations for Daemon
 * Copyright    : (c) Rodger Allen 2019-2020
 * Licence      : BSD3
 *
 * -------------------------------------------------------------------------- */

#ifndef __FILING_H_
#define __FILING_H_

#include "meta.h"
#include "global.h"

/* -------------------------------------
 *  Flags for 'writeContents()'
 */

#define FLAGNONE     0x00
#define FLAGCRC      0x01
#define FLAGMETA     0x02

/* -------------------------------------
 *
 */

int writeContents( struct metadata md, unsigned char *buf, char *path, int flags );

char * newFileName( char *pathname, char *filename, char *stamps );
char * mkStamp( struct metadata md );
char * crcStamp( struct metadata md );
char * sizeStamp( struct metadata md );
char * epochStamp( );

void showFileData( unsigned char *buf, int size );

#endif
// -----------------------------------------------------------------------------
// vi: et ts=4 ai
