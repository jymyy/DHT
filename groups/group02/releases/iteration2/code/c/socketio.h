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
#include "dhtpackettypes.h"
#include "cmdtypes.h"
#include "log.h"

#define TAG_SOCKET "Socket IO"

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

/*
 * Return listening socket opened on port.
 */
int create_listen_socket(char *port);

#endif