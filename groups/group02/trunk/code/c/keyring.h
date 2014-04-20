#ifndef KEYRING_H
#define KEYRING_H

#include "typedefs.h"
#include "keyring.h"
#include "hash.h"
#include "log.h"

#define TAG_KEYRING "Keyring"

/*
 * Calculate the midpoint mid of a and b when traversing to direction
 * dir. Clockwise is positive and counter-clockwise is negative.
 * Return zero on success.
 */
int calc_mid(sha1_t a, sha1_t b, sha1_t mid, int dir);

/*
 * Return a pointer to keyring initialized with host_key.
 */
struct keyring* init_ring(sha1_t host_key);

/*
 * Add a key to the ring. Duplicate keys are allowed.
 * Return zero on success.
 */
int add_key(struct keyring *ring, sha1_t key);

/*
 * Delete a key from the ring. Return zero if key was found and
 * deleted, nonzero otherwise. Host key can't be deleted.
 */
int del_key(struct keyring *ring, sha1_t key);

/*
 * Return nonzero if ring contains key.
 */
int has_key(struct keyring *ring, sha1_t key);

/*
 * Find position of (arbitrary) key. The returned pointer points to the key
 * or, if not found, the preceding key (when traversing clockwise). Return NULL
 * on error.
 */
struct keyring* find_pos(struct keyring *ring, sha1_t key);

/*
 * Remove an outer slice between begin and end, inclusive. Return a pointer to the
 * beginning of the slice or NULL if no slice could be created. Host key
 * is removed from the slice.
 */
struct keyring* slice_ring(struct keyring *ring, sha1_t begin_key, sha1_t end_key);

/*
 * Iterate ring or slice clockwise starting from first element and calling
 * iterfun with each key as argument. Iterfun must return zero on success.
 */
int iterate(struct keyring *ring, int (*iterfun)(sha1_t key));

/*
 * Free ring or slice. Return zero on success.
 */
int free_ring(struct keyring *ring);

#endif