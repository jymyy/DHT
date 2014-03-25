#include "typedefs.h"
#include "socketio.h"
#include <sys/socket.h>

/*
Send data until all data is sent and return length of sent data
*/
int sendall(int socket, byte *sendbuf, int packetlen, int flags) {
	int bytes_sent = 0;
	while (bytes_sent < packetlen) {
		bytes_sent += send(socket, sendbuf+bytes_sent, packetlen-bytes_sent, flags);
	}
	return bytes_sent;		
}


/*
Receive data until a complete packet is received and return length of received packet
*/
int recvall(int socket, byte *recvbuf, int bufsize, int flags) {
	int bytes_received = 0;
	int offset = 0;
	uint16_t pl_len = 0;

	while (bytes_received < PACKET_HEADER_LEN) {
		bytes_received += recv(socket, recvbuf+bytes_received, PACKET_HEADER_LEN, flags);
	}
	if (recvbuf[0] == '?') {
			offset = 1;
	}

	memcpy(&pl_len, recvbuf+PL_LEN_OFFSET+offset, sizeof(uint16_t));
	pl_len = ntohs(pl_len);
	while (bytes_received < PACKET_HEADER_LEN + pl_len + offset) {
		bytes_received += recv(socket, recvbuf+bytes_received, pl_len+offset, flags);
	}
	
	return bytes_received;
}


