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

struct header {
	sha1_t target;
	sha1_t sender;
	uint16_t type;
	uint16_t pl_len;
};

int create_packet(void **buf, int buflen, char *target_addr, uint16_t target_port, char *sender_addr, uint16_t sender_port, uint16_t type, void *payload, uint16_t payload_len);

int read_packet(void **buf, void *packet, uint16_t pl_len);

struct header* read_header(void *packet);

struct tcp_addr* build_tcp_addr(void *payload);




