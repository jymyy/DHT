#include "typedefs.h"
#include <openssl/evp.h>

int hash_addr(struct tcp_addr *tcp_addr, sha1_t key) {
    byte *pl = malloc(sizeof(uint16_t) + strlen(tcp_addr->addr) + 1);
    uint16_t port = htons(atoi(tcp_addr->port));
    memcpy(pl, &port, sizeof(uint16_t));
    memcpy(pl+sizeof(uint16_t), tcp_addr->addr, strlen(tcp_addr->addr) + 1);
    
    OpenSSL_add_all_algorithms();
    const EVP_MD *md = EVP_get_digestbyname("SHA1");
    unsigned int md_len = -1;
    EVP_MD_CTX mdctx;
    EVP_MD_CTX_init(&mdctx);
    EVP_DigestInit_ex(&mdctx, md, NULL);
    EVP_DigestUpdate(&mdctx, pl, sizeof(uint16_t) + strlen(tcp_addr->addr) + 1);
    // EVP_DigestUpdate(&mdctx, tcp_addr->addr, strlen(tcp_addr->addr)+1);
    EVP_DigestFinal_ex(&mdctx, key, &md_len);
    EVP_MD_CTX_cleanup(&mdctx);
    /*
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, &port_int, sizeof(uint16_t));
    SHA1_Update(&ctx, &(tcp_addr->addr), strlen(tcp_addr->addr));
    SHA1_Final(key, &ctx);
    */
    free(pl);
    return md_len;
}
