#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <stdint.h>

#define MAX_CONNECTIONS 2
#define MAX_ADDR_LEN 64
#define MAX_PATH_LEN 128

#define SHA1_KEY_LEN 20
#define SHA1_STR_LEN 41

#define MAX_PACKET_SIZE 65579   // MAX_BLOCK SIZE + PACKET_HEADER_LEN
#define MAX_BLOCK_SIZE 65535

// Options
#define LOG_LEVEL DEBUG_LEVEL   // Levels: ZERO, ERROR, WARN, INFO, DEBUG
#define SHA1_DEBUG_LEN 8        // Length of printed SHA1 strings
#define CMD_USE_STDIN 0         // Read commands from stdin

// Type of buffer arrays
typedef uint8_t byte;

// SHA1 hash (not printable)
typedef uint8_t sha1_t[SHA1_KEY_LEN];

// TCP address struct for internal handling
struct tcp_addr {
    char port[6];
    char addr[MAX_ADDR_LEN];
};

// DHT packet
struct packet {
    sha1_t target;
    sha1_t sender;
    uint16_t type;
    uint16_t pl_len;
    byte *payload; 
};

// Command packet for communicating between node and GUI
struct cmd {
    sha1_t key;
    uint16_t type;
    uint16_t pl_len;
    byte *payload;
};

// An item of keyring data structure
struct keyring {
    sha1_t key;
    struct keyring *next;
    struct keyring *previous;
};

#endif
