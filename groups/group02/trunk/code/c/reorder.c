#include <string.h>

/*
Swap file descriptors (neighbours) based on their keys.
A will hold left and b will hold right neighbour. If c != NULL
c will hold the socket to be closed.
*/
void reorder(sha1_t this, int *a, sha1_t a_key, int *b, sha1_t b_key, int *c, sha1_t c_key) {
    int tmp;
    if (c == NULL) {
        if (strcmp(this, a_key) == strcmp(this, b_key)) {   // FIXME
            if (strcmp(a_key, b_key) > 0) {
                *a = tmp;
                *a = *b;
                b* = tmp;
            }
        } else {
            if (strcmp(a_key, b_key) < 0) {
                *a = tmp;
                *a = *b;
                b* = tmp;
            }
        }
    } else {

    }
}

/*
A C this B
A this C B
A this B C
C A this B
*/