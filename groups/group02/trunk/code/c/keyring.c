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

    if (ring->next == NULL) {
        new->next = ring;
        new->previous = ring;
        ring->next = new;
        ring->previous = new;
    } else {
        // First the direction of traversal is determined by comparing
        // the new key to the initialization key. After that the ring is
        // traversed until a position is found where the new key is between
        // current and next/previous. Ord_diff is a sentinel for those cases
        // where the new key is lowest or highest in the whole ring.
        int ord = strncmp(key, ring->key, KEY_LEN);
        if (ord == 0) {
            fprintf(stderr, "Tried to add key equal to initialization key.\n");
            free(new);
            return 1;
        } else if (ord < 0) {
            struct keyring *cur = ring;
            struct keyring *prev = ring->previous;
            int ord_cur = ord;
            int ord_prev = strncmp(key, prev->key, KEY_LEN);
            int ord_diff = strncmp(prev->key, cur->key, KEY_LEN);
            while (ord_prev < 0 && ord_cur < 0 && ord_diff < 0) {
                cur = prev;
                prev = prev->previous;
                ord_cur = strncmp(key, cur->key, KEY_LEN);
                ord_prev = strncmp(key, prev->key, KEY_LEN);
                ord_diff = strncmp(prev->key, cur->key, KEY_LEN);
            }
            if (ord_cur == 0 || ord_prev == 0) {
                fprintf(stderr, "Tried to add duplicate key %s\n", key);
                free(new);
                return 1;
            }
            new->next = cur;
            new->previous = prev;
            cur->previous = new;
            prev->next = new;

        } else if (ord > 0) {
            struct keyring *cur = ring;
            struct keyring *next = ring->next;
            int ord_cur = ord;
            int ord_next = strncmp(key, next->key, KEY_LEN);
            int ord_diff = strncmp(next->key, cur->key, KEY_LEN);
            while (0 < ord_next && 0 < ord_cur && 0 < ord_diff) {
                cur = next;
                next = next->next;
                ord_cur = strncmp(key, cur->key, KEY_LEN);
                ord_next = strncmp(key, next->key, KEY_LEN);
                ord_diff = strncmp(next->key, cur->key, KEY_LEN);
            }
            if (ord_cur == 0 || ord_next == 0) {
                fprintf(stderr, "Tried to add duplicate key %s\n", key);
                free(new);
                return 1;
            }
            new->next = next;
            new->previous = cur;
            cur->next = new;
            next->previous = new;
        }
    }
    // DEBUG("Added %.*s between %.*s and %.*s\n", KEY_LEN, new->key, KEY_LEN, (new->previous)->key, KEY_LEN, (new->next)->key);
    return 0;
}

int get_key(struct keyring *ring, sha1_t key);

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