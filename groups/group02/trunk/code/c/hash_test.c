#include "hash.h"
#include "typedefs.h"

int main(int argc, char **argv) {
    struct tcp_addr a_addr = {.port = "9999", .addr = "pii"};
    struct tcp_addr b_addr = {.port = "1111", .addr = "paa"};
    sha1_t a_sha;
    sha1_t b_sha;
    sha1_t mid_sha;
    char a_str[SHA1_STR_LEN];
    char b_str[SHA1_STR_LEN];
    char mid_str[SHA1_STR_LEN];

    hash_addr(&a_addr, a_sha);
    hash_addr(&b_addr, b_sha);
    calc_mid(a_sha, b_sha, mid_sha, -1);

    shatostr(a_sha, a_str);
    shatostr(b_sha, b_str);
    shatostr(mid_sha, mid_str);

    return 0;
}