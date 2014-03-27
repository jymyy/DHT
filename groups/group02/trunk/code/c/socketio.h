#ifndef SOCKETIO_H
#define SOCKETIO_H

/*
Send data until all data is sent and return length of sent data
*/
int sendall(int socket, byte *sendbuf, int packetlen, int flags);


/*
Receive data until a complete packet is received and return length of received packet
*/
int recvall(int socket, byte *recvbuf, int bufsize, int flags);

#endif