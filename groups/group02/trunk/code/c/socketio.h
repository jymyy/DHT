#ifndef SOCKETIO_H
#define SOCKETIO_H

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "typedefs.h"
#include "socketio.h"
#include "dhtpacket.h"
#include "log.h"

#define TAG_SOCKET "Socket IO"

/*
 * Send data until all data is sent and return length of sent data
 */
int sendall(int socket, byte *sendbuf, int packetlen, int flags);

/*
 * Receive data until a complete packet is received and return length of received packet
 */
int recvall(int socket, byte *recvbuf, int bufsize, int flags);

/*
 * Receive command from Java
 */
int recvcmd(int socket, byte *recvbuf, int bufsize, int flags);

/*
 * Send handshake and wait for response.
 */
int init_hs(int socket);

/*
 * Wait for handshake and send response.
 */
int wait_hs(int socket);

/*
 * Open connection to target specified by addr and set socket to point there.
 */
int open_conn(int *socket, struct tcp_addr *addr);

#endif