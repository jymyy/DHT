#include "typedefs.h"
#include <openssl/evp.h>

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
    DEBUG("Converting str to sha1_t\n");
    for (int i = 0; i < SHA1_KEY_LEN; ++i) {
        /*
        val1 = (str[2*i] >= 'a')? (str[2*i] - 'a' + 10): (str[2*i] - '0');
        val2 = (str[2*i+1] >= 'a')? (str[2*i+1] - 'a' + 10): (str[2*i+1] - '0');
        val = 16*val1 + val2;
        DEBUG("Byte %02d: %03d -> %03d\n", i, sha[i], val);
        sha[i] = val;
        */
        val1 = hextodec(str[2*i]);
        val2 = hextodec(str[2*i+1]);
        val = 16*val1 + val2;
        if (val1 == 255 || val2 == 255) {
            return 1;
        }
        DEBUG("Byte %02d: %03d -> %03d\n", i, sha[i], val);
        sha[i] = val;
    }
    return 0;
}

int shatostr(sha1_t sha, char* str) {
    DEBUG("Converting sha1_t to str\n");
    for (int i = 0; i < SHA1_KEY_LEN; ++i) {
        snprintf(str+i*2, 3, "%02x", sha[i]);
    }
    str[41] = '\0';
    DEBUG("Result: %s\n", str);
    return 0;
}
