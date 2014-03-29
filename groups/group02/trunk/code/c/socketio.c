#include <sys/socket.h>
#include "typedefs.h"
#include "socketio.h"
#include "dhtpackettypes.h"

/*
Send data until all data is sent and return length of sent data
*/
int sendall(int socket, byte *sendbuf, int packetlen, int flags) {
	DEBUG("Sending to %d... ", socket);
	int bytes_sent = 0;
	while (bytes_sent < packetlen) {
		bytes_sent += send(socket, sendbuf+bytes_sent, packetlen-bytes_sent, flags);
	}
	DEBUG("ready\n");
	return bytes_sent;		
}

/*
Receive data until a complete packet is received and return length of received packet
*/
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
		bytes_total += bytes_received;
		bytes_missing -= bytes_received;
	}
	
	DEBUG("ready\n");
	return bytes_total;
}

/*
Initiate handshaking sequence i.e. send handshake and wait for response.
*/
int init_hs(int socket) {
	uint16_t client_shake = htons(DHT_CLIENT_SHAKE);
    uint16_t server_shake = htons(DHT_SERVER_SHAKE);
    uint16_t buf = 0;
    send(socket, &client_shake, 2, 0);
    while (buf != server_shake) {
    	recv(socket, &buf, 2, 0);
    }
    return 0;

}

/*
Wait for handshake and send response.
*/
int wait_hs(int socket) {
	uint16_t client_shake = htons(DHT_CLIENT_SHAKE);
    uint16_t server_shake = htons(DHT_SERVER_SHAKE);
    uint16_t buf = 0;
    while (buf != client_shake) {
    	recv(socket, &buf, 2, 0);
    }
    send(socket, &server_shake, 2, 0);
    return 0;
}

