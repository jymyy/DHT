#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <stdint.h>

#define MAX_CONNECTIONS 2
#define MAX_ADDR_LEN 64
#define MAX_PATH_LEN 128

#define SHA1_KEY_LEN 20
#define SHA1_STR_LEN 41

// Offset values for packing and unpacking
#define MAX_PACKET_SIZE 65579 // 65535 + 2 + 2 + 20 + 20
#define MAX_BLOCK_SIZE 65535

typedef unsigned char byte;

typedef unsigned char sha1_t[SHA1_KEY_LEN];

struct tcp_addr {
    char port[6];
    char addr[MAX_ADDR_LEN];
};

struct packet {
    sha1_t target;
    sha1_t sender;
    uint16_t type;
    uint16_t pl_len;
    byte *payload; 
};

struct cmd {
    sha1_t key;
    uint16_t type;
    uint16_t pl_len;
    byte *payload;
};

#endif
