#ifndef SOCKETIO_H
#define SOCKETIO_H

/*
Return length of sent data
*/
int sendall(int socket, byte *sendbuf, int packetlen, int flags);


/*
Return length of received packet
*/
int recvall(int socket, byte *recvbuf, int bufsize, int flags);

#endif