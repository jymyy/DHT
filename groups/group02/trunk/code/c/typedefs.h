#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#define HOST_ADDR "localhost"
#define HOST_PORT "9876"
#define MAX_CONNECTIONS 2
#define HOSTNAME_LEN 64
#define SERVER_ADDR "example.com"
#define SERVER_PORT "1234"

#define MAX_PACKET_SIZE 65581 // 65535 + 2 + 2 + 21 + 21
#define PACKET_HEADER_SIZE 46
#define TARGET_OFFSET 0
#define SENDER_OFFSET 21
#define TYPE_OFFSET 42
#define PL_LEN_OFFSET 44
#define PAYLOAD_OFFSET 46

typedef unsigned char byte;

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
    byte *payload; 
};

#endif