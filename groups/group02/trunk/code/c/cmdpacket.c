#include "cmdpacket.h"

int pack_cmd(byte *buf, sha1_t key, uint16_t type,
             byte *payload, uint16_t pl_len) {

    memcpy(buf+CMD_KEY_OFFSET, key, sizeof(sha1_t));
    memcpy(buf+CMD_TYPE_OFFSET, &type, sizeof(uint16_t));
    memcpy(buf+CMD_PL_LEN_OFFSET, &pl_len, sizeof(uint16_t));
    if (payload != NULL) {
        memcpy(buf+CMD_PAYLOAD_OFFSET, payload, pl_len);
    }
    
    LOG_INFO(TAG_CMD, "Packed command %s", cmd_type(type));
    return CMD_HEADER_LEN + pl_len;
}

struct cmd* unpack_cmd(byte *buf) {
    struct cmd *cmd = malloc(sizeof(struct cmd));

    memcpy(cmd->key, buf+CMD_KEY_OFFSET, sizeof(sha1_t));
    memcpy(&(cmd->type), buf+CMD_TYPE_OFFSET, sizeof(uint16_t));
    memcpy(&(cmd->pl_len), buf+CMD_PL_LEN_OFFSET, sizeof(uint16_t));
    if (cmd->pl_len > 0) {
        cmd->payload = malloc(cmd->pl_len);
        memcpy(cmd->payload, buf+CMD_PAYLOAD_OFFSET, cmd->pl_len);
    } else {
        cmd->payload = NULL;
    }

    LOG_INFO(TAG_CMD, "Unpacked command %s", cmd_type(cmd->type));
    return cmd;  
}

char* cmd_type(int type) {
    switch (type) {
        case CMD_PUT_DATA:
            return "CMD_PUT_DATA";
        default:
            return "CMD_UNKNOWN";
    }
}