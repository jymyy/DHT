#ifndef FILEIO_H
#define FILEIO_H

#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>

#include "typedefs.h"
#include "hash.h"
#include "log.h"

#define TAG_FILE "File IO"

/*
 * Write block of length blocklen located in buf to path. The name of the
 * block will be key formatted as string. Return bytes written or -1 on error.
 */
int write_block(char *dir, sha1_t key, byte *buf, int blocklen);

/*
 * Read block named key from path to buf. Return bytes read or -1 on error.
 */
int read_block(char *dir, sha1_t key, byte *buf, int buflen);

/*
 * Remove a block.
 */
int rm_block(char *dir, sha1_t key);

/**
 * Create directory if it doesn't exist. Return nonzero on error.
 */
int create_dir(char *dir);

#endif