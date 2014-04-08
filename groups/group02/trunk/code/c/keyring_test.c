#include "keyring.h"
#include "typedefs.h"

int printfun(sha1_t key) {
    fprintf("%s ", key);
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

    struct keyring *ring = init_ring(x);
    add_key(ring, c);
    add_key(ring, d);
    add_key(ring, f);
    add_key(ring, a);
    add_key(ring, b);
    add_key(ring, m);
    add_key(ring, e);

    iterate(ring, printfun); printf("\n");
    del_key(ring, f);
    iterate(ring, printfun); printf("\n");
    del_key(ring, a);
    iterate(ring, printfun); printf("\n");
    del_key(ring, d);
    iterate(ring, printfun); printf("\n");
    del_key(ring, "ggggg");
    iterate(ring, printfun); printf("\n");
    del_key(ring, b);
    iterate(ring, printfun); printf("\n");
    del_key(ring, c);
    iterate(ring, printfun); printf("\n");
    del_key(ring, m);
    iterate(ring, printfun); printf("\n");
    del_key(ring, e);
    iterate(ring, printfun); printf("\n");
    del_key(ring, x);
    iterate(ring, printfun); printf("\n");

    free_ring(ring);

    return 0;
}