#ifndef CMDPACKET_H
#define CMDPACKET_H

#include "typedefs.h"
#include "cmdtypes.h"
#include "log.h"

#define TAG_CMD "Command"

#define CMD_HEADER_LEN 24
#define CMD_KEY_OFFSET 0
#define CMD_TYPE_OFFSET 20
#define CMD_PL_LEN_OFFSET 22
#define CMD_PAYLOAD_OFFSET 24

/*
 * Create command with given parameters into buf.
 */
int pack_cmd(byte *buf, sha1_t key, uint16_t type,
             byte *payload, uint16_t pl_len);

/*
 * Return packet constructed from data in buf.
 */
struct cmd* unpack_cmd(byte *buf);

/*
 * Return name of command type.
 */
char* cmd_type(int type);

#endif