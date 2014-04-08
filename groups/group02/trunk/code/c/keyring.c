#include "keyring.h"

struct keyring* init_ring(sha1_t init_key) {
    struct keyring *ring = malloc(sizeof(struct keyring));
    strncpy(ring->key, init_key, KEY_LEN);
    ring->next = NULL;
    ring->previous = NULL;
    return ring;
}

int add_key(struct keyring *ring, sha1_t key) {
    struct keyring *new = malloc(sizeof(struct keyring));
    strncpy(new->key, key, KEY_LEN);

    struct keyring *prec;
    struct keyring *succ;
    find_position(ring, key, &prec, &succ);
    if (strncmp(key, prec->key, KEY_LEN) == 0) {
        DEBUG("Tried to add duplicate key: %s\n", key);
        return 1;
    } else {
        new->previous = prec;
        new->next = succ;
        prec->next = new;
        succ->previous = new;
        return 0;
    }

}

int del_key(struct keyring *ring, sha1_t key);

int find_position(struct keyring *ring, sha1_t key, struct keyring **prec, struct keyring **succ) {
    if (ring->next == NULL) {
        *prec = ring;
        *succ = ring;
    } else {
        // First the direction of traversal is determined by comparing
        // the new key to the initialization key. After that the ring is
        // traversed until a position is found where the new key is between
        // current and next/previous. Ord_diff is a sentinel for those cases
        // where the new key is lowest or highest in the whole ring.
        int ord = strncmp(key, ring->key, KEY_LEN);
        if (ord == 0) {
            *prec = ring;
            *succ = ring->next;
        } else if (ord < 0) {
            struct keyring *cur = ring;
            struct keyring *prev = ring->previous;
            int ord_cur = ord;
            int ord_prev = strncmp(key, prev->key, KEY_LEN);
            int ord_diff = strncmp(prev->key, cur->key, KEY_LEN);
            while (ord_prev < 0 && ord_cur <= 0 && ord_diff < 0) {
                cur = prev;
                prev = prev->previous;
                ord_cur = strncmp(key, cur->key, KEY_LEN);
                ord_prev = strncmp(key, prev->key, KEY_LEN);
                ord_diff = strncmp(prev->key, cur->key, KEY_LEN);
            }
            *prec = prev;
            *succ = cur;
        } else if (ord > 0) {
            struct keyring *cur = ring;
            struct keyring *next = ring->next;
            int ord_cur = ord;
            int ord_next = strncmp(key, next->key, KEY_LEN);
            int ord_diff = strncmp(next->key, cur->key, KEY_LEN);
            while (0 <= ord_next && 0 < ord_cur && 0 < ord_diff) {
                cur = next;
                next = next->next;
                ord_cur = strncmp(key, cur->key, KEY_LEN);
                ord_next = strncmp(key, next->key, KEY_LEN);
                ord_diff = strncmp(next->key, cur->key, KEY_LEN);
            }
            *prec = cur;
            *succ = next;
        }
    }
    return 0;
}

struct keyring* slice(struct keyring *ring, sha1_t begin, sha1_t end);

void iterate(struct keyring *ring, int (*iterfun)(sha1_t key)) {
    struct keyring *cur = ring;
    int retval = 0;
    do {
        retval = (*iterfun)(cur->key);
        cur = cur->next;
    } while (!retval && (cur != ring));

}

int free_ring(struct keyring *ring) {
    if (ring->previous == NULL) {
        free(ring);
    } else {
        (ring->previous)->next = NULL;
        struct keyring *cur = ring;
        struct keyring *next = ring->next;
        for (; next != NULL; cur = next) {
            DEBUG("Freeing key %.*s\n", KEY_LEN, cur->key);
            next = cur->next;
            free(cur);
        }
        ring = NULL;
    }
    return 0;
}