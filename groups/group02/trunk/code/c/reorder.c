#include <string.h>

/*
Swap file descriptors (neighbours) based on their keys.
*/
void reorder(sha1_t this, int *a, sha1_t a_key, int *b, sha1_t b_key) {
    int tmp;
    if (strcmp(this, a_key) == strcmp(this, b_key)) {
        if (strcmp(a_key, b_key) == 1) {
            *a = tmp;
            *a = *b;
            b* = tmp;
        }
    } else {
        if (strcmp(a_key, b_key) == -1) {
            *a = tmp;
            *a = *b;
            b* = tmp;
        }
    }
}