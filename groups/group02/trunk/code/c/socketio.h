#ifndef SOCKETIO_H
#define SOCKETIO_H

#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "typedefs.h"
#include "dhtpacket.h"
#include "cmdpacket.h"
#include "log.h"

#define TAG_SOCKET "Socket IO"

#ifndef CMD_USE_STDIN
#define CMD_USE_STDIN 1
#endif

#define sendpacket(socket, buf, target, sender, type, payload, pl_len)  \
do {                                                                    \
    int packetlen = pack(buf, target, sender,                           \
        type, payload, pl_len);                                         \
    _sendpacket(socket, buf, packetlen);                                \
} while (0)

#if !CMD_USE_STDIN
#define sendcmd(socket, buf, key, type, payload, pl_len)    \
do {                                                        \
    int packetlen = pack_cmd(buf, key,                      \
        type, payload, pl_len);                             \
    _sendcmd(socket, buf, packetlen);                       \
} while (0)
#else
#define sendcmd(socket, buf, key, type, payload, pl_len)
#endif



/*
 * Send data until all data is sent and return length of sent data
 */
int _sendpacket(int socket, byte *sendbuf, int packetlen);

/*
 * Receive data until a complete packet is received and return length of received packet
 */
int recvall(int socket, byte *recvbuf, int bufsize);

/*
 * Send command to Java
 */
int _sendcmd(int socket, byte *sendbuf, int cmdlen);

/*
 * Receive command from Java
 */
int recvcmd(int socket, byte *recvbuf, int bufsize);

/*
 * Send handshake and wait for response.
 */
int init_hs(int socket);

/*
 * Wait for handshake and send response. If received handshake indicates
 * that socket is connected to GUI return zero, otherwise return socket.
 */
int wait_hs(int socket);

/*
 * Open connection to target specified by addr and set socket to point there.
 */
int open_conn(int *socket, struct tcp_addr *addr);

int create_listen_socket(char *port);

#endif