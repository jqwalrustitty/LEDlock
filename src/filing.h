/* -----------------------------------------------------------------------------
 * File         : src/filing.h
 * Description  : File Operations for Daemon
 * Copyright    : (c) Rodger Allen 2019-2020
 * Licence      : BSD3
 * -------------------------------------------------------------------------- */

#ifndef __FILING_H_
#define __FILING_H_

#include "meta.h"
#include "global.h"

/* -------------------------------------
 *  FILE UTILITIES
 */
int writeContents( struct metadata md, uint8_t *buf, char *fname );
int check_dir( const char *path );
void showFileData( uint8_t *buf, int size );

/* -------------------------------------
 *  FILENAME GENERATION
 */
char * newFileName( const char *pathname, char *filename );
char * mkStamp( struct metadata md, char *extras, char *ext );
char * crcStamp( struct metadata md );
char * sizeStamp( struct metadata md );
char * epochStamp( );


#endif
// -----------------------------------------------------------------------------
// vi: et ts=4 ai
