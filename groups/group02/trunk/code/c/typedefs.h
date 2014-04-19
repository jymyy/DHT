#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <stdint.h>

#define MAX_CONNECTIONS 2
#define MAX_ADDR_LEN 64
#define MAX_PATH_LEN 128

#define SHA1_KEY_LEN 20
#define SHA1_STR_LEN 41

#define MAX_PACKET_SIZE 65579 // MAX_BLOCK SIZE + PACKET_HEADER_LEN
#define MAX_BLOCK_SIZE 65535

// Options
#define LOG_LEVEL DEBUG_LEVEL   // Levels: ZERO, ERROR, WARN, INFO, DEBUG
#define SHA1_DEBUG_LEN 8        // Length of printed SHA1 strings
#define CMD_USE_STDIN 1         // Read commands from stdin

typedef uint8_t byte;

typedef uint8_t sha1_t[SHA1_KEY_LEN];

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

struct keyring {
    sha1_t key;
    struct keyring *next;
    struct keyring *previous;
};

#endif
