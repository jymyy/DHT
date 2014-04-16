#ifndef FILEIO_H
#define FILEIO_H

#include "typedefs.h"
#include "hash.h"
#include "log.h"

#define TAG_FILE "File IO"

/*
 * Write block of length blocklen located in buf to path. The name of the
 * block will be key formatted as string. Return bytes written.
 */
int write_block(char *dir, sha1_t key, byte *buf, int blocklen);

/*
 * Read block named key from path to buf. Return bytes read.
 */
int read_block(char *dir, sha1_t key, byte *buf, int buflen);

/*
 * Remove a block.
 */
int rm_block(char *dir, sha1_t key);

#endif