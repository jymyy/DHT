#ifndef SOCKETIO_H
#define SOCKETIO_H

int sendall(int socket, void *sendbuf, int packetlen, int flags);

int recvall(int socket, void *recvbuf, int bufsize, int flags);

#endif