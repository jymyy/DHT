#include "keyring.h"

struct keyring* init_ring(sha1_t init_key) {
    struct keyring *new = malloc(sizeof(struct keyring));
    strncpy(new->key, init_key, SHA1_KEY_LEN);
    new->next = new;
    new->previous = new;
    return new;
}

int add_key(struct keyring *ring, sha1_t key) {
    if (ring == NULL) {
        DEBUG("Tried to add key to null ring\n");
        return 1;
    }    

    struct keyring *pos = find_pos(ring, key);
    if (strncmp(key, pos->key, SHA1_KEY_LEN) == 0) {
        DEBUG("Tried to add duplicate key: %.*s\n", SHA1_KEY_LEN, key);
        return 1;
    } else {
        struct keyring *new = malloc(sizeof(struct keyring));
        strncpy(new->key, key, SHA1_KEY_LEN);
        new->previous = pos;
        new->next = pos->next;

        if (pos == pos->next) { // If there is only initial key
            pos->previous = new;
            pos->next = new;
        } else {
            (pos->next)->previous = new;
            pos->next = new;
        }

        return 0;
    }

}

int del_key(struct keyring *ring, sha1_t key) {
    if (ring == NULL) {
        DEBUG("Tried to delete key from null ring\n");
        return 1;
    } else if (strncmp(key, ring->key, SHA1_KEY_LEN) == 0) {
        DEBUG("Can't delete initialization key\n");
        return 1;
    }

    struct keyring *pos = find_pos(ring, key);
    if (strncmp(key, pos->key, SHA1_KEY_LEN) == 0) {
        (pos->previous)->next = pos->next;
        (pos->next)->previous = pos->previous;
        DEBUG("Deleted key: %.*s\n", SHA1_KEY_LEN, key);
        free(pos);
        return 0;
    } else {
        DEBUG("Couldn't delete key: %.*s\n", SHA1_KEY_LEN, key);
        return 1;
    }
    
}

struct keyring* find_pos(struct keyring *ring, sha1_t key) {
    struct keyring *pos = NULL;
    if (ring == NULL) {
        DEBUG("Tried to find position in null ring\n");
    } else if (ring == ring->next) {
        pos = ring;
    } else {
        // First the direction of traversal is determined by comparing
        // the new key to the initialization key. After that the ring is
        // traversed until a position is found where the new key is between
        // current and next/previous. Ord_diff is a sentinel for those cases
        // where the new key is lowest or highest in the whole ring.
        int ord = strncmp(key, ring->key, SHA1_KEY_LEN);
        if (ord == 0) {
            pos = ring;
        } else if (ord < 0) {
            struct keyring *cur = ring;
            struct keyring *prev = ring->previous;
            int ord_cur = ord;
            int ord_prev = strncmp(key, prev->key, SHA1_KEY_LEN);
            int ord_diff = strncmp(prev->key, cur->key, SHA1_KEY_LEN);
            while (ord_prev < 0 && ord_cur <= 0 && ord_diff < 0) {
                cur = prev;
                prev = prev->previous;
                ord_cur = strncmp(key, cur->key, SHA1_KEY_LEN);
                ord_prev = strncmp(key, prev->key, SHA1_KEY_LEN);
                ord_diff = strncmp(prev->key, cur->key, SHA1_KEY_LEN);
            }
            pos = prev;
        } else if (ord > 0) {
            struct keyring *cur = ring;
            struct keyring *next = ring->next;
            int ord_cur = ord;
            int ord_next = strncmp(key, next->key, SHA1_KEY_LEN);
            int ord_diff = strncmp(next->key, cur->key, SHA1_KEY_LEN);
            while (0 <= ord_next && 0 < ord_cur && 0 < ord_diff) {
                cur = next;
                next = next->next;
                ord_cur = strncmp(key, cur->key, SHA1_KEY_LEN);
                ord_next = strncmp(key, next->key, SHA1_KEY_LEN);
                ord_diff = strncmp(next->key, cur->key, SHA1_KEY_LEN);
            }
            pos = cur;
        }
    }
    return pos;
}


struct keyring* slice_ring(struct keyring *ring, sha1_t range_begin, sha1_t range_end) {
    struct keyring *begin = find_pos(ring, range_begin);
    struct keyring *end = find_pos(ring, range_end);

    // Select the key that is inside the range
    if (strncmp(begin->key, range_begin, SHA1_KEY_LEN) != 0) {
        begin = begin->next;
    }

    int ord_begin = strncmp(begin->key, ring->key, SHA1_KEY_LEN);
    int ord_end = strncmp(end->key, ring->key, SHA1_KEY_LEN);

    if (ord_begin < 0 && 0 < ord_end) {
        // Join sliced parts
        (ring->previous)->next = ring->next;
        (ring->next)->previous = ring->previous;

        // Join non-sliced to ring
        (begin->previous)->next = ring;
        (end->next)->previous = ring;

        // Join ring to non-sliced
        ring->previous = begin->previous;
        ring->next = end->next;
    } else {
        // Remove initialization key from slice
        if (ord_begin == 0) {
            begin = ring->next;
        }
        if (ord_end == 0) {
            end = ring->previous;
        }
        (begin->previous)->next = end->next;
        (end->next)->previous = begin->previous;
    }

    // Terminate slice
    begin->previous = NULL;
    end->next = NULL;

    return begin;
}

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
        } while (!retval && cur != ring && cur != NULL);  // NULL check for slices
        return 0;
    }
}

int free_ring(struct keyring *ring) {
    if (ring == NULL) {
        DEBUG("Tried to free null ring\n");
        return 1;
    } else if (ring == ring->next) {
        DEBUG("Freeing key %.*s\n", SHA1_KEY_LEN, ring->key);
        free(ring);
    } else {
        if (ring->previous != NULL) {   // NULL check for slices
            (ring->previous)->next = NULL;
        }
        struct keyring *cur = ring;
        struct keyring *next = ring->next;
        for (; next != NULL; cur = next) {
            DEBUG("Freeing key %.*s\n", SHA1_KEY_LEN, cur->key);
            next = cur->next;
            free(cur);
        }
        ring = NULL;
    }
    return 0;
}