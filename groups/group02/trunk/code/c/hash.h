#ifndef HASH_H
#define HASH_H

#include "typedefs.h"

/*
 * Hash TCP address and put result in key. Return length of the key.
 */
int hash_addr(struct tcp_addr *addr, sha1_t key);

/*
 * Format sha1_t to string. Return zero on success.
 */
int strtosha(char *hex, sha1_t sha);

/*
 * Convert SHA1 hex string to sha1_t. Return zero on success.
 */
int shatostr(sha1_t sha, char* hex);

/*
 * Convert hex character to decimal value. Return 255 if
 * character is invalid.
 */
unsigned char hextodec(unsigned char hex);

#endif