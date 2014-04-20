#ifndef HASH_H
#define HASH_H

#include <openssl/evp.h>
#include <netinet/in.h>

#include "typedefs.h"
#include "log.h"

#define TAG_HASH "Hash"

/*
 * Hash TCP address and put result in key. Return length of the key.
 */
int hash_addr(struct tcp_addr *addr, sha1_t key);

/*
 * Compare two hashes. Return negative if a is smaller than b, positive
 * if a is larger than b and zero if they are equal.
 */
int hashcmp(sha1_t a, sha1_t b);

/*
 * Convert hex character to decimal value. Return 255 if
 * character is invalid.
 */
unsigned char hextodec(unsigned char hex);

/*
 * Format sha1_t to string. Return zero on success.
 */
int strtosha(char *str, sha1_t sha);

/*
 * Convert SHA1 hex string to sha1_t. Len specifies how many characters
 * (including the null byte) the resulting string should contain. If len
 * is not in range (0, SHA1_STR_LEN] then SHA1_STR_LEN is used.
 */
int shatostr(sha1_t sha, char* str, int len);

#endif