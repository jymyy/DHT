#ifndef TYPEDEFS_H
#define TYPEDEFS_H


#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <openssl/sha.h>

#define MAX_CONNECTIONS 2
#define MAX_ADDR_LEN 64

// Offset values for packing and unpacking
#define MAX_PACKET_SIZE 65579 // 65535 + 2 + 2 + 20 + 20
#define PACKET_HEADER_LEN 44
#define TARGET_OFFSET 0
#define SENDER_OFFSET 20
#define TYPE_OFFSET 40
#define PL_LEN_OFFSET 42
#define PAYLOAD_OFFSET 44


#ifndef NDEBUG
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG(...) 
#endif

typedef unsigned char byte;

typedef unsigned char sha1_t[SHA_DIGEST_LENGTH];

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

#endif
