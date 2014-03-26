#include "typedefs.h"
#include "socketio.h"
#include <sys/socket.h>

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


