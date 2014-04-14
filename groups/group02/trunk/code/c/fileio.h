#ifndef FILEIO_H
#define FILEIO_H

/*
 * Write block of length blocklen located in buf to path. The name of the
 * block will be key formatted as string. Return bytes written.
 */
int write_block(char *path, sha1_t key, byte *buf, int blocklen);

/*
 * Read block named key from path to buf. Return bytes read.
 */
int read_block(char *path, sha1_t key, byte *buf, int buflen);

#endif