#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "typedefs.h"
#include "socketio.h"
#include "dhtpackettypes.h"

int sendall(int socket, byte *sendbuf, int packetlen, int flags) {
	DEBUG("Sending to %d... ", socket);
	int bytes_sent = 0;
	while (bytes_sent < packetlen) {
		bytes_sent += send(socket, sendbuf+bytes_sent, packetlen-bytes_sent, flags);
	}
	DEBUG("ready\n");
	return bytes_sent;		
}

int recvall(int socket, byte *recvbuf, int bufsize, int flags) {
	DEBUG("Receiving from %d... ", socket);
	int bytes_total = 0;
	int bytes_received = 0;
	int bytes_missing = PACKET_HEADER_LEN;
	int offset = 0;
	uint16_t pl_len = 0;

	// Receive header
	while (bytes_missing > 0) {
		bytes_received = recv(socket, recvbuf+bytes_total, bytes_missing, flags);
        if (bytes_received == 0) {
            DIE("sender disconnected");
        } else if (bytes_total > bufsize) {
            DIE("recvbuf overflow");
        }
		bytes_total += bytes_received;
		bytes_missing -= bytes_received;
	}

	// Adjust offset, see comments of "unpack" for more info
	if (recvbuf[0] == '?') {
			offset = 1;
	}

	// Check length of the packet and receive more data if needed
	memcpy(&pl_len, recvbuf+PL_LEN_OFFSET+offset, sizeof(uint16_t));
	pl_len = ntohs(pl_len);
	bytes_received = 0;
	bytes_missing = pl_len + offset;
	while (bytes_missing > 0) {
		bytes_received = recv(socket, recvbuf+bytes_total, bytes_missing, flags);
        if (bytes_received == 0) {
            DIE("sender disconnected");
        } else if (bytes_total > bufsize) {
            DIE("recvbuf overflow");
        }
		bytes_total += bytes_received;
		bytes_missing -= bytes_received;
	}
	
	DEBUG("ready\n");
	return bytes_total;
}

int recvcmd(int socket, byte *recvbuf, int bufsize, int flags) {
    return 0;
}

int init_hs(int socket) {
	DEBUG("Handshaking with %d... ", socket);
	uint16_t client_shake = htons(DHT_CLIENT_SHAKE);
    uint16_t server_shake = htons(DHT_SERVER_SHAKE);
    uint16_t buf = 0;
    send(socket, &client_shake, 2, 0);
    while (buf != server_shake) {
    	recv(socket, &buf, 2, 0);
    }
    DEBUG("ready\n");
    return 0;

}

int wait_hs(int socket) {
	DEBUG("Handshaking with %d... ", socket);
	uint16_t client_shake = htons(DHT_CLIENT_SHAKE);
    uint16_t server_shake = htons(DHT_SERVER_SHAKE);
    uint16_t buf = 0;
    while (buf != client_shake) {
    	recv(socket, &buf, 2, 0);
    }
    send(socket, &server_shake, 2, 0);
    DEBUG("ready\n");
    return 0;
}

int open_conn(int *sock, struct tcp_addr *addr) {
    int status = 0;
    struct addrinfo hints, *info;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((status = getaddrinfo(addr->addr, addr->port, &hints, &info)) != 0) {
        DIE(gai_strerror(status));
    }
    if ((*sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == -1) {
        DIE("Socket creation failed");
    }
    if ((status = connect(*sock, info->ai_addr, info->ai_addrlen)) == -1) {
        DIE("Connecting socket failed");
        close(*sock);
    }
    freeaddrinfo(info);
    return 0;
}

