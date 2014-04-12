#include <openssl/evp.h>

#include "typedefs.h"

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
    DEBUG("Converting str to sha1_t\n");
    for (int i = 0; i < SHA1_KEY_LEN; i++) {
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
    for (int i = 0; i < SHA1_KEY_LEN; i++) {
        snprintf(str+i*2, 3, "%02x", sha[i]);
    }
    str[SHA1_STR_LEN] = '\0';
    DEBUG("Result: %s\n", str);
    return 0;
}

int calc_mid(sha1_t a, sha1_t b, sha1_t mid, int dir) {
    int ord = hashcmp(a, b);
    int val = 0;
    int carry = 0;
    if ((ord < 0 && dir < 0) || (ord > 0 && dir > 0)) {
        for (int i = 0; i < SHA1_KEY_LEN; i++) {
            val = ((int) a[i] + (int) b[i]) % 256 + carry;
            carry = val % 2;
            val /= 2;
            val += carry;
            mid[i] = (unsigned char) val;
        }
    } else if ((ord < 0 && dir > 0) || (ord > 0 && dir < 0)) {
        for (int i = 0; i < SHA1_KEY_LEN; i++) {
            val = (int) a[i] + (int) b[i] + 256*carry;
            carry = val % 2;
            val /= 2;
            if (val - 256 > 0 && i > 0) {
                mid[i-1]++;
                val -= 256;
            }
            mid[i] = (unsigned char) val;
        }
    }

    char a_str[SHA1_STR_LEN];
    char b_str[SHA1_STR_LEN];
    char mid_str[SHA1_STR_LEN];
    shatostr(a, a_str);
    shatostr(b, b_str);
    shatostr(mid, mid_str);
    //DEBUG("Mid of %s and %s (direction %d) is %s\n", a_str, b_str, dir, mid_str);
    return 0;
}
