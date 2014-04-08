#ifndef KEYRING_H
#define KEYRING_H

#include "typedefs.h"

#define KEY_LEN 5

struct keyring {
    sha1_t key;
    struct keyring *next;
    struct keyring *previous;
};

/*
 * Return a pointer to keyring initialized with key.
 */
struct keyring* init_ring(sha1_t init_key);

/*
 * Add a key to the ring. Duplicate keys are not allowed.
 * Return zero on success.
 */
int add_key(struct keyring *ring, sha1_t key);

/*
 * Delete a key from the ring. Return zero if key was found and
 * deleted, nonzero otherwise.
 */
int del_key(struct keyring *ring, sha1_t key);

/*
 * Find position of (arbitrary) key. Prec and succ are set to point preceding and
 * succeeding items in the ring, respectively, when the ring is traversed
 * clockwise.
 */
int find_position(struct keyring *ring, sha1_t key, struct keyring **prec, struct keyring **succ);

/*
 * Remove an outer slice between begin and end. Return a pointer to the
 * beginning of the slice or NULL if no slice could be created.
 */
struct keyring* slice(struct keyring *ring, sha1_t begin, sha1_t end);

/*
 * Iterate the ring clockwise starting from first element and calling
 * iterfun with each key as argument. Iterfun must return zero on success.
 */
int iterate(struct keyring *ring, int (*iterfun)(sha1_t key));

/*
 * Free ring or slice. Return zero on success.
 */
int free_ring(struct keyring *ring);

#endif