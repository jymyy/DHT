#ifndef TYPEDEFS_H
#define TYPEDEFS_H

typedef unsigned char sha1_t[21];

struct tcp_addr {
    char port[5];
    char *addr;
};

struct packet {
    sha1_t target;
    sha1_t sender;
    uint16_t type;
    uint16_t pl_len;
    void *payload; 
};

#endif