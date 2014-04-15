#ifndef DHTPACKET_H
#define DHTPACKET_H

#include "typedefs.h"
#include "dhtpackettypes.h"

/*
* Create packet with given parameters into buffer buf.
*/
int pack(byte *buf, int buflen, sha1_t target_key, sha1_t sender_key,
         uint16_t type, byte *payload, uint16_t payload_len);

/*
* Return packet constructed from data in buffer buf.
*/
struct packet* unpack(byte *buf, int buflen);

/*
* Build TCP address(es) from payload.
*/
int build_tcp_addr(byte *payload, struct tcp_addr *left, struct tcp_addr *right);

/*
 * Acquire lock. Doesn't return until lock is acquired.
 */
int acquire(int socket, sha1_t key, sha1_t host_key);

/*
 * Release lock.
 */
int release(int socket, sha1_t key, sha1_t host_key);

/*
 * Function for turning the packettypes to
 * plain text.
 */
char* packet_type(int type);

#endif


