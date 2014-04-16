#ifndef CMDPACKET_H
#define CMDPACKET_H

#include "typedefs.h"
#include "cmdtypes.h"
#include "log.h"

#define TAG_CMD "Command"

#define CMD_HEADER_LEN 24
#define KEY_OFFSET 0
#define TYPE_OFFSET 20
#define PL_LEN_OFFSET 22
#define PAYLOAD_OFFSET 24

/*
 * Create command with given parameters into buf.
 */
int pack_cmd(byte *buf, int buflen, sha1_t key, uint16_t type,
             byte *payload, uint16_t payload_len);

/*
 * Return packet constructed from data in buf.
 */
struct cmd* unpack_cmd(byte *buf, int buflen);

/*
 * Return name of command type.
 */
char* cmd_type(int type);

#endif