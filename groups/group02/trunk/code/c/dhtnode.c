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
    if (argc != 5) {
        die("give host and server ports and addresses as argument");
    }

    char *host_address = argv[1];
    char *host_port = argv[2];
    char *server_address = argv[3];
    char *server_port = argv[4];
           
    // Status info     
    int status = 0;
    int running = 1;
    int acks_received = 0;
    
    // Address structs and keys
    struct tcp_addr host_addr;
    struct tcp_addr serv_addr;
    struct tcp_addr left_addr;
    struct tcp_addr right_addr;
    sha1_t host_key;
    sha1_t serv_key;

    // Sockets
    fd_set rfds;
    fd_set wfds;
    int cmdsock = fileno(stdin);    // TODO Connect this to GUI
    int listensock = create_listen_socket(host_port);
    int servsock = -1;
    int left = -1;  // left neighbour socket
    int right = -1; // right neighbour socket

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

    // Connect to server
	if ((status = getaddrinfo(host_address, host_port, &hosthints, &hostinfo)) != 0) {
        die(gai_strerror(status));
    }

    if ((status = getaddrinfo(server_address, server_port, &servhints, &servinfo)) != 0) {
        die(gai_strerror(status));
    }
	
	servsock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (connect(servsock, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        die("connection refused (is server running?)");
    }

    // Handshake with server
    sleep(1);	// Server doesn't accept handshake if sent immediately
    init_hs(servsock);

    // Create keys and address structs
    struct sockaddr_in *sa = (struct sockaddr_in *) hostinfo->ai_addr;
    char host_ip4[INET_ADDRSTRLEN]; 
    inet_ntop(AF_INET, &(sa->sin_addr), host_ip4, INET_ADDRSTRLEN);
    strcpy(host_addr.addr, host_ip4);
    strcpy(host_addr.port, host_port);

    struct sockaddr_in *sb = (struct sockaddr_in *) servinfo->ai_addr;
    char serv_ip4[INET_ADDRSTRLEN]; 
    inet_ntop(AF_INET, &(sb->sin_addr), serv_ip4, INET_ADDRSTRLEN);
    strcpy(serv_addr.addr, serv_ip4);
    strcpy(serv_addr.port, server_port);

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
    
    // Start main loop (connection sequence isn't actually
    // finished at this point)
    while(running) {
        // Add sockets to listening pool
		FD_ZERO(&rfds);
		FD_SET(listensock, &rfds);
        FD_SET(servsock, &rfds);
        FD_SET(cmdsock, &rfds);
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
        status = select(10, &rfds, NULL, NULL, NULL);

		if (status == -1) {
			die("select failed");
		} else if (FD_ISSET(cmdsock, &rfds)) {
			// Currently program terminates if it receives q from stdin.
			read(cmdsock, recvbuf, MAX_PACKET_SIZE);
            if (recvbuf[0] == 'q') {
                DEBUG("Disconnecting...\n");
                packetlen = pack(sendbuf, MAX_PACKET_SIZE, host_key, host_key,
                    DHT_DEREGISTER_BEGIN, NULL, 0);
                sendall(servsock, sendbuf, packetlen, 0);
            }
            /*
            recvcmd(cmdsock, recvbuf, MAX_PACKET_SIZE);
            cmd = unpack_cmd(recvbuf);
            switch (cmd->type) {
                case PUT:
                    // Get payload from cmd, create packet and send to server
                    break;
                case GET:
                    // Send request to server
                    break;
                case DUMP:
                    // Send request to server
                    break;
                case TERMINATE:
                    DEBUG("Disconnecting...\n");
                    packetlen = pack(sendbuf, MAX_PACKET_SIZE, host_key, host_key,
                        DHT_DEREGISTER_BEGIN, NULL, 0);
                    sendall(servsock, sendbuf, packetlen, 0);
                    break;
                default:
                    die("invalid command");
            }
            */
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
                case DHT_SEND_DATA:
                    // TODO: Send payload to Java
                    break;
                case DHT_NO_DATA:
                    // TODO: Inform Java that no data was found
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
                    build_tcp_addr(packet->payload, &nb_addr, NULL);
                    open_conn(&tempfd, &nb_addr);
                    
                    // Handshake with new node
                    sha1_t nb_key;
                    hash_addr(&nb_addr, nb_key);
                    init_hs(tempfd);

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
                    // Inform Java that disconnection sequence has begun.
                    build_tcp_addr(packet->payload, &left_addr, &right_addr);

                    // Connect to left neighbour
                    open_conn(&left, &left_addr);
                    init_hs(left);

                    // Connect to right neighbour
                    open_conn(&right, &right_addr);
                    init_hs(right);

                    running = 0;
                    break;
                case DHT_DEREGISTER_DENY:
                    // Disconnection attempt denied
                    // TODO: Inform Java
                    fprintf(stderr, "Disconnecting denied\n");
                    break;
                case DHT_GET_DATA:
                    // Some node requested data
                    // TODO: Open connection to requesting node and send data
                    break;
                case DHT_PUT_DATA_ACK:
                    // Data added succesfully
                    // TODO: Send OK to Java
                    break;
                case DHT_DUMP_DATA:
                    // Some node requested data removal
                    // TODO: Remove data (if exists) and send ACK to server
                    break;
                case DHT_DUMP_DATA_ACK:
                    // Our request to remove data was succesful
                    // TODO: Send OK to Java
                    break;
                default:
                    // Invalid packet type, dump packet if debugging and die
                    DEBUG("PACKET DUMP\n");
                    for (int i = 0; i < packetlen; ++i) {
                        DEBUG("%x ", recvbuf[i]);
                    }
                    DEBUG("\n");
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
            wait_hs(tempfd);
        }
    }

    // Server accepted our leaving attempt and connections
    // to both neighbours have been established. Send data to
    // neighbours and wait confirmation from server.
    int disconnecting = 1;
    int deregs_received = 0;
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
                    free(packet->payload);
                    free(packet);
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
