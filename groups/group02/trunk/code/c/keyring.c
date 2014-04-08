#include "keyring.h"

struct keyring* init_ring(sha1_t init_key) {
    struct keyring *new = malloc(sizeof(struct keyring));
    strncpy(new->key, init_key, KEY_LEN);
    new->next = new;
    new->previous = new;
    return new;
}

int add_key(struct keyring *ring, sha1_t key) {
    if (ring == NULL) {
        DEBUG("Tried to add key to null ring\n");
        return 1;
    }

    struct keyring *new = malloc(sizeof(struct keyring));
    strncpy(new->key, key, KEY_LEN);

    struct keyring *prec;
    struct keyring *succ;
    find_position(ring, key, &prec, &succ);
    if (strncmp(key, prec->key, KEY_LEN) == 0) {
        DEBUG("Tried to add duplicate key: %.*s\n", KEY_LEN, key);
        free(new);
        return 1;
    } else {
        new->previous = prec;
        new->next = succ;
        prec->next = new;
        succ->previous = new;
        return 0;
    }

}

int del_key(struct keyring *ring, sha1_t key) {
    if (ring == NULL) {
        DEBUG("Tried to delete key from null ring\n");
        return 1;
    } else if (strncmp(key, ring->key, KEY_LEN) == 0) {
        DEBUG("Can't delete initialization key\n");
        return 1;
    }

    struct keyring *prec;
    struct keyring *succ;
    find_position(ring, key, &prec, &succ);
    if (strncmp(key, prec->key, KEY_LEN) == 0) {
        (prec->previous)->next = succ;
        succ->previous = prec->previous;
        DEBUG("Deleted key: %.*s\n", KEY_LEN, key);
        free(prec);
        return 0;
    } else {
        DEBUG("Couldn't delete key: %.*s\n", KEY_LEN, key);
        return 1;
    }
    
}

int find_position(struct keyring *ring, sha1_t key, struct keyring **prec, struct keyring **succ) {
    if (ring == NULL) {
        DEBUG("Tried to find position in null ring\n");
        return 1;
    } else if (ring == ring->next) {
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

int iterate(struct keyring *ring, int (*iterfun)(sha1_t key)) {
    if (ring == NULL) {
        DEBUG("Tried to iterate null ring\n");
        return 1;
    } else {
        struct keyring *cur = ring;
        int retval = 0;
        do {
            retval = (*iterfun)(cur->key);
            cur = cur->next;
        } while (!retval && (cur != ring));
        return 0;
    }
}

int free_ring(struct keyring *ring) {
    if (ring == NULL) {
        DEBUG("Tried to free null ring\n");
        return 1;
    } else if (ring == ring->next) {
        DEBUG("Freeing key %.*s\n", KEY_LEN, ring->key);
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