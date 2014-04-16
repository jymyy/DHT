#include "hash.h"

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

int hashcmp(sha1_t a, sha1_t b) {
    int diff = 0;
    for (int i = 0; i < SHA1_KEY_LEN && diff == 0; i++) {
        diff = a[i] - b[i];
    }
    return diff;
}

unsigned char hextodec(unsigned char hex) {
    // Conversion is done based on ASCII values
    if ('0' <= hex && hex <= '9') {
        return hex - '0';
    } else if ('A' <= hex && hex <= 'F') {
        return hex - 'A' + 10;
    } else if ('a' <= hex && hex <= 'f') {
        return hex - 'a' + 10;
    } else {
        return 255;
    }
}

int strtosha(char *str, sha1_t sha) {
    unsigned char val1 = 0;
    unsigned char val2 = 0;
    unsigned char val = 0;
    for (int i = 0; i < SHA1_KEY_LEN; i++) {
        val1 = hextodec(str[2*i]);
        val2 = hextodec(str[2*i+1]);
        val = 16*val1 + val2;
        if (val1 == 255 || val2 == 255) {
            return 1;
        }
        sha[i] = val;
    }
    return 0;
}

int shatostr(sha1_t sha, char* str, int len) {
    if (len <= 0 || SHA1_STR_LEN < len) {
        len = SHA1_STR_LEN;
    }
    for (int i = 0; i < len/2; i++) {
        snprintf(str+i*2, 3, "%02x", sha[i]);
    }
    str[len] = '\0';
    return 0;
}
