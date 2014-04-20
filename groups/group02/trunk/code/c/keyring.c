#include "keyring.h"

int calc_mid(sha1_t a, sha1_t b, sha1_t mid, int dir) {
    int ord = hashcmp(a, b);
    int val = 0;
    int carry = 0;

    // First, calculate the midpoint for the sector that doesn't
    // contain the gap. Midpoint can be found by calculating average
    // of bits in same position.
    for (int i = 0; i < SHA1_KEY_LEN; i++) {
        val = (int) a[i] + (int) b[i] + 256*carry;
        carry = val % 2;
        val /= 2;
        if (val - 256 > 0 && i > 0) {
            mid[i-1]++;
            val -= 256;
        }
        mid[i] = (unsigned char) val;
    }

    // Check if the requested midpoint was actually on the sector
    // that has the gap. If so, then simply find the point that is on
    // the opposite side of the ring (i.e. add or substract 128 to/from
    // the most significant bit).
    if ((ord < 0 && dir < 0) || (ord > 0 && dir > 0)) {
        mid[0] = (mid[0] + 128) % 256;
    }

    return 0;
}

struct keyring* init_ring(sha1_t host_key) {
    struct keyring *new = malloc(sizeof(struct keyring));
    memcpy(new->key, host_key, SHA1_KEY_LEN);
    new->next = new;
    new->previous = new;
    return new;
}

int add_key(struct keyring *ring, sha1_t key) {
    if (ring == NULL) {
        LOG_WARN(TAG_KEYRING, "Tried to add key to null ring");
        return 1;
    }

    struct keyring *pos = find_pos(ring, key);
    if (hashcmp(key, pos->key) != 0) {
        struct keyring *new = malloc(sizeof(struct keyring));
        memcpy(new->key, key, SHA1_KEY_LEN);
        new->previous = pos;
        new->next = pos->next;

        if (pos == pos->next) { // If there is only host key
            pos->previous = new;
            pos->next = new;
        } else {
            (pos->next)->previous = new;
            pos->next = new;
        }
    }

    if (LOG_LEVEL >= DEBUG_LEVEL) {
        char key_str[SHA1_DEBUG_LEN];
        shatostr(key, key_str, SHA1_DEBUG_LEN);
        LOG_DEBUG(TAG_KEYRING, "Added key: %s", key_str);
    }

    return 0;
}

int del_key(struct keyring *ring, sha1_t key) {
    if (ring == NULL) {
        LOG_WARN(TAG_KEYRING, "Tried to delete key from null ring");
        return 1;
    } else if (hashcmp(key, ring->key) == 0) {
        LOG_WARN(TAG_KEYRING, "Can't delete host key");
        return 1;
    }

    struct keyring *pos = find_pos(ring, key);
    if (hashcmp(key, pos->key) == 0) {
        (pos->previous)->next = pos->next;
        (pos->next)->previous = pos->previous;
        LOG_DEBUG(TAG_KEYRING, "Deleted key: %.*s", SHA1_KEY_LEN, key);
        free(pos);
        return 0;
    } else {
        LOG_WARN(TAG_KEYRING, "Couldn't delete key: %.*s", SHA1_KEY_LEN, key);
        return 1;
    }
}

int has_key(struct keyring *ring, sha1_t key) {
    if (ring == NULL) {
        LOG_WARN(TAG_KEYRING, "Tried to delete key from null ring");
        return 0;
    }

    struct keyring *pos = find_pos(ring, key);
    return !(hashcmp(key, pos->key));
}

struct keyring* find_pos(struct keyring *ring, sha1_t key) {
    struct keyring *pos = NULL;
    if (ring == NULL) {
        LOG_WARN(TAG_KEYRING, "Tried to find position in null ring");
    } else if (ring == ring->next) {
        pos = ring;
    } else {
        // First the direction of traversal is determined by comparing
        // the new key to the host key. After that the ring is
        // traversed until a position is found where the new key is between
        // current and next/previous. Ord_diff is a sentinel for those cases
        // where the new key is lowest or highest in the whole ring.
        int ord = hashcmp(key, ring->key);
        if (ord == 0) {
            pos = ring;
        } else if (ord < 0) {
            struct keyring *cur = ring;
            struct keyring *prev = ring->previous;
            int ord_cur = ord;
            int ord_prev = hashcmp(key, prev->key);
            int ord_diff = hashcmp(prev->key, cur->key);
            while (ord_prev < 0 && ord_cur <= 0 && ord_diff < 0) {
                cur = prev;
                prev = prev->previous;
                ord_cur = hashcmp(key, cur->key);
                ord_prev = hashcmp(key, prev->key);
                ord_diff = hashcmp(prev->key, cur->key);
            }
            pos = prev;
        } else if (ord > 0) {
            struct keyring *cur = ring;
            struct keyring *next = ring->next;
            int ord_cur = ord;
            int ord_next = hashcmp(key, next->key);
            int ord_diff = hashcmp(next->key, cur->key);
            while (0 <= ord_next && 0 < ord_cur && 0 < ord_diff) {
                cur = next;
                next = next->next;
                ord_cur = hashcmp(key, cur->key);
                ord_next = hashcmp(key, next->key);
                ord_diff = hashcmp(next->key, cur->key);
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
    if (hashcmp(begin->key, range_begin) != 0) {
        begin = begin->next;
    }

    int ord_begin = hashcmp(begin->key, ring->key);
    int ord_end = hashcmp(end->key, ring->key);

    if (ord_begin == 0 && ord_end == 0) {
        // Only host key in ring
        return NULL;
    } else if (ord_begin < 0 && 0 < ord_end) {
        // Remove host key from slice
        (ring->previous)->next = ring->next;
        (ring->next)->previous = ring->previous;

        // Join non-sliced parts back to host key
        (begin->previous)->next = ring;
        (end->next)->previous = ring;

        // Join host key back to non-sliced parts
        ring->previous = begin->previous;
        ring->next = end->next;
    } else {
        // Remove host key from slice
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
        LOG_WARN(TAG_KEYRING, "Tried to iterate null ring");
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
    LOG_DEBUG(TAG_KEYRING, "Freeing ring");
    if (ring == NULL) {
        LOG_DEBUG(TAG_KEYRING, "Tried to free null ring");
        return 0;
    } else if (ring == ring->next) {
        free(ring);
    } else {
        if (ring->previous != NULL) {   // NULL check for slices
            (ring->previous)->next = NULL;
        }
        struct keyring *cur = ring;
        struct keyring *next = ring->next;
        for (; next != NULL; cur = next) {
            next = cur->next;
            free(cur);
        }
        ring = NULL;
    }
    return 0;
}