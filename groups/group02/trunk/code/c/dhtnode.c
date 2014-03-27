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

#include "dhtpackettypes.h"
#include "dhtpacket.h"
#include "socketio.h"
#include "typedefs.h"
#include "hash.h"

void die(const char *reason) {
    fprintf(stderr, "Fatal error: %s\n", reason);
    exit(1);
}

int create_listen_socket(char *port) {
    int fd;
    int t;

    struct sockaddr_in a;

    a.sin_addr.s_addr = INADDR_ANY;
    a.sin_family = AF_INET;
    a.sin_port = htons(atoi(port));

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

int main(int argc, char **argv) {
    if (argc != 2) {
        die("give host port as an argument");
    }
    char *host_port = argv[1];
    int status = 0;
    int running = 1;
    
    // Connection state
    int acks_received = 0;
    int left = -1;  // left neighbour socket
    int right = -1; // right neighbour socket
    struct tcp_addr left_addr;
    struct tcp_addr right_addr;
    sha1_t host_key;
    sha1_t serv_key;

    fd_set rfds;
    fd_set wfds;
    int cmdsock = -1;
	int servsock = -1;
    int listensock = create_listen_socket(host_port);

    // Buffers for sending and receiving
	byte *sendbuf = malloc(MAX_PACKET_SIZE);
	byte *recvbuf = malloc(MAX_PACKET_SIZE);
    memset(sendbuf, 0, MAX_PACKET_SIZE);
    memset(recvbuf, 0, MAX_PACKET_SIZE);
	int packetlen = 0;

    // Structs for connection info
	struct addrinfo servhints, hosthints, *servinfo, *hostinfo;
	memset(&servhints, 0, sizeof(struct addrinfo));
	memset(&hosthints, 0, sizeof(struct addrinfo));
	servhints.ai_family = AF_INET; // AF_UNSPEC, AF_INET or AF_INET6
	servhints.ai_socktype = SOCK_STREAM;
	hosthints.ai_family = AF_INET;
	hosthints.ai_socktype = SOCK_STREAM;
	hosthints.ai_flags = AI_PASSIVE;

    // Connect to server
	if ((status = getaddrinfo(HOST_ADDR, host_port, &hosthints, &hostinfo)) != 0) {
        die(gai_strerror(status));
    }

    if ((status = getaddrinfo(SERVER_ADDR, SERVER_PORT, &servhints, &servinfo)) != 0) {
        die(gai_strerror(status));
    }
	
	servsock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (connect(servsock, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        die("connection refused (is server running?)");
    }

    // Handshake with server
    int CLIENT_SHAKE = htons(DHT_CLIENT_SHAKE);
    int SERVER_SHAKE = htons(DHT_SERVER_SHAKE);
    DEBUG("Handshaking with server... ");
    send(servsock, &CLIENT_SHAKE, 2, 0);
    while (recv(servsock, recvbuf, 2, 0) != 2);
    DEBUG("ready\n");

    // Create keys and address structs
    struct sockaddr_in *sa = (struct sockaddr_in *) hostinfo->ai_addr;
    char host_ip4[INET_ADDRSTRLEN]; 
    inet_ntop(AF_INET, &(sa->sin_addr), host_ip4, INET_ADDRSTRLEN);
	struct tcp_addr host_addr;
    strcpy(host_addr.addr, host_ip4);
    strcpy(host_addr.port, host_port);

    struct sockaddr_in *sb = (struct sockaddr_in *) servinfo->ai_addr;
    char serv_ip4[INET_ADDRSTRLEN]; 
    inet_ntop(AF_INET, &(sb->sin_addr), serv_ip4, INET_ADDRSTRLEN);
    struct tcp_addr serv_addr = {.port = SERVER_PORT};
    strcpy(serv_addr.addr, serv_ip4);

    hash_addr(&host_addr, host_key);
    hash_addr(&serv_addr, serv_key);

    freeaddrinfo(hostinfo);
    freeaddrinfo(servinfo);

    // Send DHT_REGISTER_BEGIN to server
    uint16_t port = htons(atoi(host_addr.port));
	byte *pl = malloc(sizeof(uint16_t) + strlen(host_addr.addr) + 1);
	memcpy(pl, &port, sizeof(uint16_t));
	memcpy(pl+sizeof(uint16_t), host_addr.addr, strlen(host_addr.addr) + 1);
	packetlen = pack(sendbuf, MAX_PACKET_SIZE, serv_key, host_key,
	   DHT_REGISTER_BEGIN, pl, sizeof(uint16_t) + strlen(host_addr.addr) + 1);
	sendall(servsock, sendbuf, packetlen, 0);
	free(pl);
    
    int timer = 0;
    while(running) {
        timer++;
        if (timer == 30) {
            DEBUG("TIMEOUT\n");
            packetlen = pack(sendbuf, MAX_PACKET_SIZE, host_key, host_key,
                DHT_DEREGISTER_BEGIN, NULL, 0);
            sendall(servsock, sendbuf, packetlen, 0);
        }

        // Add sockets to listening pool
		FD_ZERO(&rfds);
		FD_SET(listensock, &rfds);
        FD_SET(servsock, &rfds);
        if (left != -1) {
            FD_SET(left, &rfds);
        } 
        if (right != -1) {
            FD_SET(right, &rfds);
        }

        // Reset buffers
        memset(sendbuf, 0, MAX_PACKET_SIZE);
        memset(recvbuf, 0, MAX_PACKET_SIZE);

        // Let's hope 10 is high enough for numfds...
        struct timeval timeout;
        timeout.tv_sec = 1;
		status = select(10, &rfds, NULL, NULL, &timeout);

		if (status == -1) {
			die("select failed");
		} else if (FD_ISSET(cmdsock, &rfds)) {
            // TODO Command socket for communicating with UI
		} else if (FD_ISSET(left, &rfds) || FD_ISSET(right, &rfds)) {
            int tempfd = -1;
            if (FD_ISSET(left, &rfds)) {
                tempfd = left;
                left = -1;
            } else if (FD_ISSET(right, &rfds)) {
                tempfd = right;
                right = -1;
            }
            packetlen = recvall(tempfd, recvbuf, MAX_PACKET_SIZE, 0);
            struct packet *packet = unpack(recvbuf, packetlen);
            switch (packet->type) {
                case DHT_TRANSFER_DATA:
                    // TODO: Implement data structure and insert data there
                    break;
                case DHT_REGISTER_ACK:
                    // Received all data from neighbour, send DONE to server if
                    // ACK received from both neighbours
                    acks_received++;
                    close(tempfd);

                    if (acks_received == 2) {
                        packetlen = pack(sendbuf, MAX_PACKET_SIZE, host_key, host_key,
                            DHT_REGISTER_DONE, NULL, 0);
                        sendall(servsock, sendbuf, packetlen, 0);
                    }
                    break;
                case DHT_DEREGISTER_ACK:
                    // Received all data from leaving neighbour, acknowledge this to server
                    close(tempfd);
                    packetlen = pack(sendbuf, MAX_PACKET_SIZE, packet->sender, host_key,
                        DHT_DEREGISTER_DONE, NULL, 0);
                    sendall(servsock, sendbuf, packetlen, 0);
                    break;
                default:
                    die("invalid header");
            }
            free(packet->payload);
            free(packet);

        } else if (FD_ISSET(servsock, &rfds)) {
            packetlen = recvall(servsock, recvbuf, MAX_PACKET_SIZE, 0);
            struct packet *packet = unpack(recvbuf, packetlen);
            switch (packet->type) {
                case DHT_REGISTER_FAKE_ACK:
                    // First node in network
                    packetlen = pack(sendbuf, MAX_PACKET_SIZE, host_key, host_key,
					   DHT_REGISTER_DONE, NULL, 0);
					sendall(servsock, sendbuf, packetlen, 0);
                    break;
                case DHT_REGISTER_BEGIN:
                    // New node is trying to join, connect and send data
                    ; // Complier throws error without this semicolon
                    int tempfd;
                    struct tcp_addr nb_addr;
                    struct addrinfo nb_hints, *nb_info;
                    build_tcp_addr(packet->payload, &nb_addr, NULL);
                    
                    memset(&nb_hints, 0, sizeof(struct addrinfo));
                    nb_hints.ai_family = AF_INET;
                    nb_hints.ai_socktype = SOCK_STREAM;
                    if ((status = getaddrinfo(nb_addr.addr, nb_addr.port, &nb_hints, &nb_info)) != 0) {
                        die(gai_strerror(status));
                    }
					if ((tempfd = socket(nb_info->ai_family, nb_info->ai_socktype, nb_info->ai_protocol)) == -1) {
						fprintf(stderr, "Socket creation failed\n");
						continue;
					}
					if ((status = connect(tempfd, nb_info->ai_addr, nb_info->ai_addrlen)) == -1) {
						fprintf(stderr, "Connecting failed\n");
						close(tempfd);
						continue;
					}
                    
                    // Handshake with new node
                    sha1_t nb_key;
                    hash_addr(&nb_addr, nb_key);
                    DEBUG("Handshaking with %d... ", tempfd);
                    send(tempfd, &CLIENT_SHAKE, 2, 0);
                    while (recv(tempfd, recvbuf, MAX_PACKET_SIZE, 0) != 2);
                    DEBUG("ready\n");

                    // TODO Send data
                    // Send ACK to new node to inform that all data is sent
                    packetlen = pack(sendbuf, MAX_PACKET_SIZE, nb_key, nb_key,
                        DHT_REGISTER_ACK, NULL, 0);
                    sendall(tempfd, sendbuf, packetlen, 0);
                    close(tempfd);
                    break;
                case DHT_REGISTER_DONE:
                    // TODO Forget data sent to new neighbour
                    break;
                case DHT_DEREGISTER_BEGIN:
                	// A node leaves abnormally
                	packetlen = pack(sendbuf, MAX_PACKET_SIZE, packet->sender, host_key,
						DHT_DEREGISTER_DONE, NULL, 0);
					sendall(servsock, sendbuf, packetlen, 0);
                	break;
                case DHT_DEREGISTER_ACK:
                    // Server has responded to our attempt to leave, create connections to both neighbours
                    build_tcp_addr(packet->payload, &left_addr, &right_addr);

                    // Connect to left neighbour
                    struct addrinfo left_hints, *left_info;
                    memset(&left_hints, 0, sizeof(struct addrinfo));
                    left_hints.ai_family = AF_INET;
                    left_hints.ai_socktype = SOCK_STREAM;
                    if ((status = getaddrinfo(left_addr.addr, left_addr.port, &left_hints, &left_info)) != 0) {
                        die(gai_strerror(status));
                    }
                    if ((left = socket(left_info->ai_family, left_info->ai_socktype, left_info->ai_protocol)) == -1) {
                        die(strerror(errno));
                    }
                    if ((status = connect(left, left_info->ai_addr, left_info->ai_addrlen)) == -1) {
                        die(strerror(errno));
                    }

                    // Handshake with left
                    DEBUG("Handshaking with %d... ", left);
                    send(left, &CLIENT_SHAKE, 2, 0);
                    while (recv(left, recvbuf, MAX_PACKET_SIZE, 0) != 2);
                    DEBUG("ready\n");

                    // Connect to right neighbour
                    struct addrinfo right_hints, *right_info;
                    memset(&right_hints, 0, sizeof(struct addrinfo));
                    right_hints.ai_family = AF_INET;
                    right_hints.ai_socktype = SOCK_STREAM;
                    if ((status = getaddrinfo(right_addr.addr, right_addr.port, &right_hints, &right_info)) != 0) {
                        die(gai_strerror(status));
                    }
                    if ((right = socket(right_info->ai_family, right_info->ai_socktype, right_info->ai_protocol)) == -1) {
                        die(strerror(errno));
                    }
                    if ((status = connect(right, right_info->ai_addr, right_info->ai_addrlen)) == -1) {
                        die(strerror(errno));
                    }

                    // Handshake with right
                    DEBUG("Handshaking with %d... ", right);
                    send(right, &CLIENT_SHAKE, 2, 0);
                    while (recv(right, recvbuf, MAX_PACKET_SIZE, 0) != 2);
                    DEBUG("ready\n");

                    // Cleanup
                    freeaddrinfo(left_info);
                    freeaddrinfo(right_info);
                    running = 0;
                    break;
                case DHT_DEREGISTER_DENY:
                    // Leaving attempt denied
                    fprintf(stderr, "Leaving denied\n");
                    break;
                default:
                    // Invalid packet type, dump packet and die
                    for (int i = 0; i < packetlen; ++i) {
                        fprintf(stderr, "%x ", recvbuf[i]);
                    }
                    fprintf(stderr, "\n");
                    die("invalid packet type");
            }
            free(packet->payload);
            free(packet);

        } else if (FD_ISSET(listensock, &rfds)) {
            // Another node tries to connect
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
            DEBUG("Handshaking with %d... ", tempfd);
            recv(tempfd, recvbuf, MAX_PACKET_SIZE, 0);
            if (recvbuf[0] == 'A' && recvbuf[1] == '!') {
                send(tempfd, &SERVER_SHAKE, 2, 0);
                DEBUG("ready\n");
            } else {
                die("invalid handshake");
            }
        }
    }

    // Server accepted our leaving attempt and connections
    // to both neighbours have been established. Send data to
    // neighbours and wait confirmation from server.
    int disconnecting = 1;
    int deregs_received = 0;
    DEBUG("DISCONNECTING\n");
    while (disconnecting) {
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_SET(servsock, &rfds);
        if (left != -1) {
            FD_SET(left, &wfds);
        } 
        if (right != -1) {
            FD_SET(right, &wfds);
        }
        memset(sendbuf, 0, MAX_PACKET_SIZE);
        memset(recvbuf, 0, MAX_PACKET_SIZE);

        status = select(10, &rfds, &wfds, NULL, NULL);

        if (status == -1) {
            die("select failed");
        } else if (FD_ISSET(servsock, &rfds)) {
            packetlen = recvall(servsock, recvbuf, MAX_PACKET_SIZE, 0);
            struct packet *packet = unpack(recvbuf, packetlen);
            switch (packet->type) {
                case DHT_DEREGISTER_DONE:
                    // Simply leave after receiving confirmations from
                    // both neighbours
                    deregs_received++;
                    if (deregs_received == 2) {
                        disconnecting = 0;
                    }
                    break;
                default:
                    die("invalid packet type");
            }
        } else if (FD_ISSET(left, &wfds) || FD_ISSET(right, &wfds)) {
            if (FD_ISSET(left, &wfds)) {
                // Send data to left neighbour
                sha1_t left_key;
                hash_addr(&left_addr, left_key);
                packetlen = pack(sendbuf, MAX_PACKET_SIZE, left_key, host_key,
                    DHT_DEREGISTER_ACK, NULL, 0);
                sendall(left, sendbuf, packetlen, 0);
                close(left);
                left = -1;
            } else if (FD_ISSET(right, &wfds)) {
                // Send data to right neighbour
                sha1_t right_key;
                hash_addr(&right_addr, right_key);
                packetlen = pack(sendbuf, MAX_PACKET_SIZE, right_key, host_key,
                    DHT_DEREGISTER_ACK, NULL, 0);
                sendall(right, sendbuf, packetlen, 0);
                close(right);
                right = -1;
            }
        }
    }

    // Cleanup
    close(listensock);
    close(servsock);
    free(sendbuf);
    free(recvbuf);
    
    return 0;
}
