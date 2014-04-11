#include "keyring.h"
#include "typedefs.h"

int printfun(sha1_t key) {
    fprintf(stderr, "%s ", key);
    return 0;
}

int main(int argc, char **argv) {
    sha1_t a = "aaaaa";
    sha1_t b = "bbbbb";
    sha1_t c = "ccccc";
    sha1_t d = "ddddd";
    sha1_t e = "eeeee";
    sha1_t f = "fffff";
    sha1_t m = "mmmmm";
    sha1_t x = "xxxxx";

    struct keyring *ring = init_ring(b);
    add_key(ring, c);
    add_key(ring, d);
    add_key(ring, f);
    add_key(ring, a);
    add_key(ring, x);
    add_key(ring, m);
    add_key(ring, e);

    printf("Initial ring\n");
    iterate(ring, printfun); printf("\n");

    struct keyring *slice = slice_ring(ring, a, d);
    printf("Ring after slicing\n");
    iterate(ring, printfun); printf("\n");
    printf("Slice\n");
    iterate(slice, printfun); printf("\n");
    printf("Free slice\n");
    free_ring(slice);

    printf("Free ring\n");
    free_ring(ring);

    return 0;
}