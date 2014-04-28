#ifndef DHTPACKET_H
#define DHTPACKET_H

#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "typedefs.h"
#include "dhtpackettypes.h"
#include "dhtpacket.h"
#include "hash.h"
#include "log.h"

#define TAG_PACKET "Packet"

#define PACKET_HEADER_LEN 44
#define TARGET_OFFSET 0
#define SENDER_OFFSET 20
#define TYPE_OFFSET 40
#define PL_LEN_OFFSET 42
#define PAYLOAD_OFFSET 44

/**
 * Pack data and send to socket. Return number of sent bytes.
 */
int sendpacket(int socket, byte *buf, sha1_t target, sha1_t sender,
               uint16_t type, byte *payload, uint16_t pl_len);

/**
 * Receive and unpack from socket. Return NULL on error.
 */
struct packet* recvpacket(int socket, byte *buf, int bufsize);

/**
 * Send contents of sendbuf to socket.
 */
int send_p(int socket, byte *sendbuf, int packetlen);

/**
 * Receive from socket to recvbuf.
 */
int recv_p(int socket, byte *recvbuf, int bufsize);

/**
 * Create packet to buf.
 */
int pack_p(byte *buf, sha1_t target_key, sha1_t sender_key,
        uint16_t type, byte *payload, uint16_t pl_len);

/**
 * Unpack contents of buf to.
 */
struct packet* unpack_p(byte *buf);

/*
 * Build TCP address(es) from payload.
 */
int build_tcp_addr(byte *payload, struct tcp_addr *left, struct tcp_addr *right);

/*
 * Copy tcp_addr to pl in format suitable to sending as
 * payload. Pl should be freed after. Return length of data in pl.
 */
int addr_to_pl(byte **pl, struct tcp_addr *addr);

/*
 * Return name of packet type.
 */
const char* packettostr(int type);

#endif


