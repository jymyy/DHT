#include "typedefs.h"
#include <openssl/evp.h>

/*
* Hash TCP address and put result in key. Return length of the key.
*/
int hash_addr(struct tcp_addr *tcp_addr, sha1_t key) {
    // For some reason hashing port first and then address (using
    // DigestUpdate twice) doesn't work. Therefore port and address have to
    // be copied to a single buffer and hashed together.
    byte *pl = malloc(sizeof(uint16_t) + strlen(tcp_addr->addr) + 1);
    uint16_t port = htons(atoi(tcp_addr->port));
    memcpy(pl, &port, sizeof(uint16_t));
    memcpy(pl+sizeof(uint16_t), tcp_addr->addr, strlen(tcp_addr->addr) + 1);
    
    const EVP_MD *md = EVP_sha1();
    unsigned int md_len = -1;
    EVP_MD_CTX mdctx;
    EVP_MD_CTX_init(&mdctx);
    EVP_DigestInit_ex(&mdctx, md, NULL);
    EVP_DigestUpdate(&mdctx, pl, sizeof(uint16_t) + strlen(tcp_addr->addr) + 1);
    EVP_DigestFinal_ex(&mdctx, key, &md_len);
    EVP_MD_CTX_cleanup(&mdctx);
    
    free(pl);
    return md_len;
}
