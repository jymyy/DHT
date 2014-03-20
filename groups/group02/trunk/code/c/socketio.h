#ifndef SOCKETIO_H
#define SOCKETIO_H

/*
Return length of sent data
*/
int sendall(int socket, void *sendbuf, int packetlen, int flags);


/*
Return length of received packet
*/
int recvall(int socket, void *recvbuf, int bufsize, int flags);

#endif