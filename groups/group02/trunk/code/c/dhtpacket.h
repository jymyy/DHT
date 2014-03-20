#ifndef DHTPACKET_H
#define DHTPACKET_H

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

#include "typedefs.h"
#include "dhtpackettypes.h"

int pack(byte **buf, int buflen, sha1_t target_key, sha1_t sender_key, uint16_t type, void *payload, uint16_t payload_len);

struct packet* unpack(byte *buf, int buflen);

int build_tcp_addr(byte *payload, struct tcp_addr *left, struct tcp_addr *right);

#endif


