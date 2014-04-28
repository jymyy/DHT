#ifndef CMDPACKET_H
#define CMDPACKET_H

#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "typedefs.h"
#include "cmdtypes.h"
#include "log.h"

#define TAG_CMD "Command"

#define CMD_HEADER_LEN 24
#define CMD_KEY_OFFSET 0
#define CMD_TYPE_OFFSET 20
#define CMD_PL_LEN_OFFSET 22
#define CMD_PAYLOAD_OFFSET 24

/**
 * Pack and send command to socket. Return number of sent bytes.
 *
 * socket:  Where to send
 * buf:     Buffer used in packing and sending, should have length of
 *          at least CMD_HEADER_LEN+pl_len  
 * key:     Command key
 * type:    Command type
 * payload: Payload for the command
 * pl_len:  Payload length
 */
int sendcmd(int socket, byte *buf, sha1_t key, uint16_t type,
            byte *payload, uint16_t pl_len);

/**
 * Receive and unpack command from socket. Return NULL on error.
 */
struct cmd* recvcmd(int socket, byte *buf, int bufsize);

/**
 * Send contents of sendbuf to socket.
 */
int send_c(int socket, byte *sendbuf, int cmdlen);

/**
 * Receive from socket to recvbuf.
 */
int recv_c(int socket, byte *recvbuf, int bufsize);

/**
 * Create command to buf.
 */
int pack_c(byte *buf, sha1_t key, uint16_t type,
             byte *payload, uint16_t pl_len);

/**
 * Unpack contents of buf.
 */
struct cmd* unpack_c(byte *buf);


/*
 * Return name of command type.
 */
const char* cmdtostr(int type);

#endif