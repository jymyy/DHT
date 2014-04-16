#include "socketio.h"

int sendall(int socket, byte *sendbuf, int packetlen, int flags) {
	LOG_INFO(TAG_SOCKET, "Sending to %d", socket);
	int bytes_sent = 0;
	while (bytes_sent < packetlen) {
		bytes_sent += send(socket, sendbuf+bytes_sent, packetlen-bytes_sent, flags);
	}
    LOG_DEBUG(TAG_SOCKET, "Packet sent");
	return bytes_sent;		
}

int recvall(int socket, byte *recvbuf, int bufsize, int flags) {
	LOG_INFO(TAG_SOCKET, "Receiving from %d", socket);
	int bytes_total = 0;
	int bytes_received = 0;
	int bytes_missing = PACKET_HEADER_LEN;
	int offset = 0;
	uint16_t pl_len = 0;

	// Receive header
	while (bytes_missing > 0) {
		bytes_received = recv(socket, recvbuf+bytes_total, bytes_missing, flags);
        if (bytes_received == 0) {
            DIE(TAG_SOCKET, "Sender disconnected");
        } else if (bytes_total > bufsize) {
            DIE(TAG_SOCKET, "Recvbuf overflow");
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
            DIE(TAG_SOCKET, "Sender disconnected");
        } else if (bytes_total > bufsize) {
            DIE(TAG_SOCKET, "Recvbuf overflow");
        }
		bytes_total += bytes_received;
		bytes_missing -= bytes_received;
	}
	
	LOG_DEBUG(TAG_SOCKET, "Packet received");
	return bytes_total;
}

int recvcmd(int socket, byte *recvbuf, int bufsize, int flags) {
    return 0;
}

int init_hs(int socket) {
	LOG_INFO(TAG_SOCKET, "Handshaking with %d", socket);
	uint16_t client_shake = htons(DHT_CLIENT_SHAKE);
    uint16_t server_shake = htons(DHT_SERVER_SHAKE);
    uint16_t buf = 0;
    send(socket, &client_shake, 2, 0);
    while (buf != server_shake) {
    	recv(socket, &buf, 2, 0);
    }
    LOG_DEBUG(TAG_SOCKET, "Handshake completed");
    return 0;

}

int wait_hs(int socket) {
	LOG_INFO(TAG_SOCKET, "Waiting for handshake from %d", socket);
	uint16_t client_shake = htons(DHT_CLIENT_SHAKE);
    uint16_t server_shake = htons(DHT_SERVER_SHAKE);
    uint16_t buf = 0;
    while (buf != client_shake) {
    	recv(socket, &buf, 2, 0);
    }
    send(socket, &server_shake, 2, 0);
    LOG_DEBUG(TAG_SOCKET, "Handshake completed");
    return 0;
}

int open_conn(int *sock, struct tcp_addr *addr) {
    LOG_INFO(TAG_SOCKET, "Opening connection");
    int status = 0;
    struct addrinfo hints, *info;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((status = getaddrinfo(addr->addr, addr->port, &hints, &info)) != 0) {
        DIE(TAG_SOCKET, "%s", gai_strerror(status));
    }
    if ((*sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == -1) {
        DIE(TAG_SOCKET, "Socket creation failed");
    }
    if ((status = connect(*sock, info->ai_addr, info->ai_addrlen)) == -1) {
        DIE(TAG_SOCKET, "Connecting socket failed");
        close(*sock);
    }
    freeaddrinfo(info);
    LOG_DEBUG(TAG_SOCKET, "Connection opened");
    return 0;
}

