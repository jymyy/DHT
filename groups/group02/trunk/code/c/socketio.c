#include "typedefs.h"
#include "socketio.h"
#include <sys/socket.h>

/*
Send data until all data is sent and return length of sent data
*/
int sendall(int socket, byte *sendbuf, int packetlen, int flags) {

	int bytes_sent = 0;
	while (bytes_sent < packetlen) {
		bytes_sent += send(socket, (void *)sendbuf, packetlen, flags);
	}
	return bytes_sent;		
}


/*
Receive data until all data is received and return length of received packet
*/

int recvall(int socket, byte *recvbuf, int bufsize, int flags) {

	int bytes_received = 0;
	while (bytes_received < bufsize) {
		bytes_received += recv(socket, (void *)recvbuf, bufsize, flags);
	}
	return bytes_received;
}


