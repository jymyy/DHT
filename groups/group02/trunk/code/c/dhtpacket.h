#ifndef DHTPACKET_H
#define DHTPACKET_H

#include "typedefs.h"
#include "dhtpackettypes.h"
#include "dhtpacket.h"
#include "socketio.h"
#include "hash.h"
#include "log.h"

#define TAG_PACKET "Packet"

#define PACKET_HEADER_LEN 44
#define TARGET_OFFSET 0
#define SENDER_OFFSET 20
#define TYPE_OFFSET 40
#define PL_LEN_OFFSET 42
#define PAYLOAD_OFFSET 44

/*
 * Create packet with given parameters into buf.
 */
int pack(byte *buf, sha1_t target_key, sha1_t sender_key,
         uint16_t type, byte *payload, uint16_t payload_len);

/*
 * Return packet constructed from data in buffer buf.
 */
struct packet* unpack(byte *buf);

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


