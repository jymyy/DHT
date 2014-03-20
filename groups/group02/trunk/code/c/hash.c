#include "typedefs.h"
#include <openssl/sha.h>

int hash_addr(struct tcp_addr *tcp_addr, sha1_t key) {
    const unsigned char *temp = (unsigned char *) tcp_addr->addr;
    SHA1(temp, strlen(tcp_addr->addr), key);
    return 0;
}