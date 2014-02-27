/* You can use this file as a starting point for your C code. */

/* Reading the documentation for select and for TCP/IP is strongly advised;
   see e.g. man pages:
   select(2)
   select_tut(2)
   tcp(7)
   ip(7) */

#include "dhtpackettypes.h"
#include "create_packet.h"
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
#include <netdb.h>

#define HOST_PORT 9876
#define MAX_CONNECTIONS 5
#define MAX_PACKET_SIZE 1024
#define HOSTNAME_LEN 64
#define SERVER_ADDR "example.com"
#define SERVER_PORT 1234

struct TCP_addr {
	char *addr;
	uint16_t port;
};
typedef struct TCP_addr TCP_addr;

void die(char *reason) {
    fprintf(stderr, "Fatal error: %s\n", reason);
    exit(1);
}

int create_listen_socket() {
    int fd;
    int t;

    struct sockaddr_in a;

    a.sin_addr.s_addr = INADDR_ANY;
    a.sin_family = AF_INET;
    a.sin_port = htons(HOST_PORT);

    fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        die(strerror(errno));

    t = bind(fd, (struct sockaddr *)(&a), sizeof(struct sockaddr_in));
    if (t == -1)
        die(strerror(errno));

    t = listen(fd, MAX_CONNECTIONS);
    if (t == -1)
        die(strerror(errno));        

    return fd;
}

int main(void) {
    fd_set rfds;
    int retval;
    int running = 1;
	int left = 0;
	int right = 0;
    int listensock = create_listen_socket();

	int servsock;
	void *sendbuf;
	void *rcvbuf;
	void *payloadbuf;
	int packetlen;
	struct addrinfo servhints, hosthints, *servinfo, *hostinfo;
	memset(&servhints, 0, sizeof(servhints));
	memset(&hosthints, 0, sizeof(hosthints));
	servhints.ai_family = AF_UNSPEC;
	servhints.ai_socktype = SOCK_STREAM;
	hosthints.ai_family = AF_UNSPEC;
	hosthints.ai_socktype = SOCK_STREAM;
	hosthints.ai_flags = AI_PASSIVE;

	// char hostname[HOSTNAME_LEN];
	// gethostname(hostname, HOSTNAME_LEN);
	getaddrinfo(NULL, HOST_PORT, &hosthints, &hostinfo);
    
	getaddrinfo(SERVER_ADDR, SERVER_PORT, &servhints, &servinfo);
	listensock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	connect(servsock, servinfo->ai_addr, servinfo->ai_addrlen);
	sendall(servsock, DHT_CLIENT_SHAKE, 2, 0);	// Handshake
	rcvall(servsock, rcvbuf, MAX_PACKET_SIZE, 0);

	TCP_addr tcp_addr = {hostinfo->ai_addr, HOST_PORT};
	packetlen = create_packet(&sendbuf, MAX_PACKET_SIZE, servinfo->ai_addr, SERVER_PORT, hostinfo->ai_addr, HOST_PORT,
	DHT_REGISTER_BEGIN, &tcp_addr, sizeof(uint16_t) + strlen(tcp_addr.addr));
	sendall(servsock, sendbuf, packetlen, 0);	// DHT_REGISTER_BEGIN

    while(running) {
		FD_ZERO(&rfds);
		// FD_SET(0, &rfds); /* Standard input */
		FD_SET(listensock, &rfds);     

		retval = select(listensock + 1, &rfds, NULL, NULL, NULL);

		if (retval == -1) {
			die("select failed");
		} else if (retval) {
			if (FD_ISSET(listensock, &rfds)) {
				struct sockaddr_in tempaddr;
				unsigned int addrlen = 0;
				int tempfd = accept(listensock, (struct sockaddr *)&tempaddr,
							&addrlen);
				recvall(tempfd, recvbuf, MAX_PACKET_SIZE, 0);
				Header *header = read_header(recvbuf);
				if (header->target == this) {	// this == hosthash
					switch (header->type) {
						case DHT_CLIENT_SHAKE:
							break;	// From neighbour (connecting)
						case DHT_SERVER_SHAKE:
							break; // From new neighbour
						case DHT_REGISTER_BEGIN:
							break; // From server
						case DHT_REGISTER_ACK:
							break; // Received data from neighbour (connecting)
						case DHT_REGISTER_FAKE_ACK:
							break; // First node in network (connecting)
						case DHT_REGISTER_DONE:
							break; // Forget data sent to new neighbour
					}
				} else {
					forward(); // NOT IMPLEMENTED
				}
				close(tempfd);
				
			}
		}
    }

    close(listensock);

    return 0;
}
