/* You can use this file as a starting point for your C code. */

/* Reading the documentation for select and for TCP/IP is strongly advised;
   see e.g. man pages:
   select(2)
   select_tut(2)
   tcp(7)
   ip(7) */

#include "dhtpackettypes.h"
#include "dhtpacket.h"
#include "socketio.h"
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
#define MAX_CONNECTIONS 2
#define MAX_PACKET_SIZE 1024
#define HOSTNAME_LEN 64
#define SERVER_ADDR "example.com"
#define SERVER_PORT 1234

typedef unsigned char[21] sha1_t;

struct tcp_addr {
    uint16_t port;
	char *addr;
};

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
    // Program state
    int status = 0;
    int running = 1;
    
    // Connection state
    int ACKS_RECEIVED = 0;
    int left = -1;  // left neighbour socket
    int right = -1; // right neighbour socket
    int third = -1; // third, connecting node
    int lonely = 0;
    sha1_t this = hash(self);

    fd_set rfds;
	int servsock;
    int listensock = create_listen_socket();
	unsigned char *sendbuf = malloc(MAX_PACKET_SIZE);   // packet to be sent
	unsigned char *recvbuf = malloc(MAX_PACKET_SIZE);    // packet received
	int packetlen;

    // Structs for connection info
	struct addrinfo servhints, hosthints, *servinfo, *hostinfo;
	memset(&servhints, 0, sizeof(struct addrinfo));
	memset(&hosthints, 0, sizeof(struct addrinfo));
	servhints.ai_family = AF_UNSPEC;
	servhints.ai_socktype = SOCK_STREAM;
	hosthints.ai_family = AF_UNSPEC;
	hosthints.ai_socktype = SOCK_STREAM;
	hosthints.ai_flags = AI_PASSIVE;

	if ((status = getaddrinfo(NULL, HOST_PORT, &hosthints, &hostinfo)) != 0) {
        die(gai_strerror(status));
    }

    if ((status = getaddrinfo(SERVER_ADDR, SERVER_PORT, &servhints, &servinfo)) != 0) {
        die(gai_strerror(status));
    }
	
	listensock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (connect(servsock, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        die(strerror(errno));
    }

    // Handshake with server
	sendall(servsock, DHT_CLIENT_SHAKE, 2, 0);
	rcvall(servsock, recvbuf, MAX_PACKET_SIZE, 0);

    // Send DHT_REGISTER_BEGIN to server
	struct tcp_addr host_addr = {hostinfo->ai_addr, HOST_PORT};
	packetlen = pack(&sendbuf, MAX_PACKET_SIZE, servinfo->ai_addr, SERVER_PORT, hostinfo->ai_addr, HOST_PORT,
	DHT_REGISTER_BEGIN, &host_addr, sizeof(uint16_t) + strlen(host_addr.addr));
	sendall(servsock, sendbuf, packetlen, 0);

    while(running) {
        // Add appropriate sockets to listening pool
		FD_ZERO(&rfds);
		FD_SET(listensock, &rfds);
        FD_SET(servsock, &rfds);
        if (left != -1) {
             FD_SET(left, &rfds);
        } 
        if (right != -1) {
             FD_SET(left, &rfds);
        }
        if (third != -1) {
            FD_SET(third, &rfds);
        }

        // Reset buffers
        memset(&sendbuf, 0, MAX_PACKET_SIZE);
        memset(&recvbuf, 0, MAX_PACKET_SIZE);

		status = select(listensock + 1, &rfds, NULL, NULL, NULL);

		if (status == -1) {
			die("select failed");
		} else if (status) {
            if (FD_ISSET(stdin, &rfds)) { // cmdsock
                /*
                cmd = read_cmd_from_cmdsock()
                switch (cmd) {
                    case PUT_DATA:
                        break;
                    case GET_DATA:
                        break;
                    case TERMINATE:
                        packetlen = pack(&sendbuf, MAX_PACKET_SIZE, target_key, sender_key,
                        DHT_DEREGISTER_BEGIN, NULL, 0);
                        sendall(servsock, sendbuf, packetlen, 0);
                        break;
                    default:
                        die();
                }
                */
            } else if (FD_ISSET(listensock, &rfds)) {
				struct sockaddr_in tempaddr;
                int tempfd;
				unsigned int addrlen = 0;
                
                if ((tempfd = accept(listensock, (struct sockaddr *)&tempaddr,
                            &addrlen)) == -1) {
                    die(strerror(errno));
                } else if (left == -1) {
                    left = tempfd;
                } else if (left != -1 && right == -1) {
                    right = tempfd;
                } else {
                    die("error accepting new connection");
                }
                FD_SET(tempfd, &rfds);
				recvall(tempfd, recvbuf, MAX_PACKET_SIZE, 0);
				struct packet *packet = unpack(recvbuf);
                switch (packet->type) {
                    case DHT_CLIENT_SHAKE:
                        sendall(tempfd, DHT_SERVER_SHAKE, 2, 0);
                        break;
                    default:
                        die("invalid handshake");
                }

			} else if (FD_ISSET(left, &rfds) || FD_ISSET(right, &rfds)) {
                int *tempfd;
                if (FD_ISSET(left, &rfds)) {
                    *tempfd = left;
                } else if (FD_ISSET(right, &rfds)) {
                    *tempfd = right;
                } else {
                    die();
                }
                recvall(*tempfd, recvbuf, MAX_PACKET_SIZE, 0);
                struct packet *packet = unpack(recvbuf);
                switch (packet->type) {
                    case DHT_TRANSFER_DATA:
                        // TODO: Implement hash ring
                        // insert_data(ring, packet->payload, packet->pl_len);
                        break;
                    case DHT_REGISTER_ACK:
                        // Received data from neighbour (connecting)
                        ACKS_RECEIVED++;
                        shutdown(*tempfd, 2);
                        *tempfd = -1;

                        if (ACKS_RECEIVED == 2) {
                            packetlen = pack(&sendbuf, MAX_PACKET_SIZE, target_key, sender_key,
                            DHT_REGISTER_DONE, &host_addr, sizeof(uint16_t) + strlen(host_addr.addr));
                            sendall(servsock, sendbuf, packetlen, 0);
                        }
                        break;
                    case DHT_DEREGISTER_ACK:
                        // Received all data from leaving neighbour
                        shutdown(*tempfd, 2);
                        packetlen = pack(&sendbuf, MAX_PACKET_SIZE, packet->sender, sender_key,
                        DHT_DEREGISTER_DONE, NULL, 0);
                        sendall(servsock, sendbuf, packetlen, 0);
                        *tempfd = -1;
                        break;
                    default:
                        die("invalid header");
                }

            } else if (FD_ISSET(servsock, &rfds)) {
                recvall(servsock, recvbuf, MAX_PACKET_SIZE, 0);
                struct packet *packet = unpack(recvbuf);
                switch (packet->type) {
                    case DHT_REGISTER_FAKE_ACK:
                        // First node in network (connecting), do nothing
                        lonely = 1;
                        break;
                    case DHT_REGISTER_BEGIN:
                        struct tcp_addr *nb_addr = build_tcp_addr(packet->pl_len, packet->payload);
                        struct addrinfo nb_hints, *nb_info;
                        int tempfd;
                        memset(&nb_hints, 0, sizeof(struct addrinfo));
                        nb_hints.ai_family = AF_UNSPEC;
                        nb_hints.ai_socktype = SOCK_STREAM;
                        if ((status = getaddrinfo(NULL, NULL, &nb_hints, &nb_info)) != 0) {
                            die(gai_strerror(status));
                        }
                        if ((tempfd = socket(nb_info->ai_family, nb_info->ai_socktype, nb_info->ai_protocol)) == -1) {
                            die(strerror(errno));
                        }
                        if ((status = connect(tempfd, nb_info->ai_addr, nb_info->ai_addrlen)) == -1) {
                            die(strerror(errno));
                        }

                        // TODO Send data
                        packetlen = pack(&sendbuf, MAX_PACKET_SIZE, target_key, sender_key,
                        DHT_REGISTER_ACK, NULL, 0);
                        sendall(servsock, sendbuf, packetlen, 0);
                        close(tempfd);

                        if (lonely) {
                            lonely = 0;
                            if ((status = getaddrinfo(NULL, NULL, &nb_hints, &nb_info)) != 0) {
                            die(gai_strerror(status));
                            }
                            if ((tempfd = socket(nb_info->ai_family, nb_info->ai_socktype, nb_info->ai_protocol)) == -1) {
                                die(strerror(errno));
                            }
                            if ((status = connect(tempfd, nb_info->ai_addr, nb_info->ai_addrlen)) == -1) {
                                die(strerror(errno));
                            }

                            packetlen = pack(&sendbuf, MAX_PACKET_SIZE, target_key, sender_key,
                            DHT_REGISTER_ACK, NULL, 0);
                            sendall(servsock, sendbuf, packetlen, 0);
                            close(tempfd);
                        }
                        break;
                    case DHT_REGISTER_DONE:
                        // Forget data sent to new neighbour
                        break;
                    case DHT_DEREGISTER_ACK:
                        // TODO Read neighbour addresses from payload
                        running = 0;
                        break;
                    case DHT_DEREGISTER_DENY:
                        // TODO Inform user
                        break;
                    default:
                        die("invalid header");
                }
            }
		}
    }

    
    while () {
        FD_ZERO(&rfds);
        FD_SET(left, &rfds);
        FD_SET(right, &rfds);
        memset(&sendbuf, 0, MAX_PACKET_SIZE);
        memset(&recvbuf, 0, MAX_PACKET_SIZE);

        status = select(listensock + 1, &rfds, NULL, NULL, NULL);

        if (status == -1) {
            die("select failed");
        } else if (FD_ISSET(servsock, &rfds)) {
            recvall(servsock, recvbuf, MAX_PACKET_SIZE, 0);
            struct packet *packet = unpack(recvbuf, MAX_PACKET_SIZE);
            switch ()
        }
    }

    close(listensock);
    close(servsock);

    return 0;
}
