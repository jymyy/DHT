#include "cmdpacket.h"

int pack_cmd(byte *buf, sha1_t key, uint16_t type,
             byte *payload, uint16_t pl_len) {

    type = ntohs(type);
    pl_len = ntohs(pl_len);
    memcpy(buf+CMD_KEY_OFFSET, key, sizeof(sha1_t));
    memcpy(buf+CMD_TYPE_OFFSET, &type, sizeof(uint16_t));
    memcpy(buf+CMD_PL_LEN_OFFSET, &pl_len, sizeof(uint16_t));
    if (payload != NULL) {
        memcpy(buf+CMD_PAYLOAD_OFFSET, payload, pl_len);
    }
    
    LOG_INFO(TAG_CMD, "Packed command %s", cmdtostr(type));
    return CMD_HEADER_LEN + pl_len;
}

struct cmd* unpack_cmd(byte *buf) {
    struct cmd *cmd = malloc(sizeof(struct cmd));

    memcpy(cmd->key, buf+CMD_KEY_OFFSET, sizeof(sha1_t));
    memcpy(&(cmd->type), buf+CMD_TYPE_OFFSET, sizeof(uint16_t));
    memcpy(&(cmd->pl_len), buf+CMD_PL_LEN_OFFSET, sizeof(uint16_t));
    cmd->type = ntohs(cmd->type);
    cmd->pl_len = ntohs(cmd->pl_len);
    if (cmd->pl_len > 0) {
        cmd->payload = malloc(cmd->pl_len);
        memcpy(cmd->payload, buf+CMD_PAYLOAD_OFFSET, cmd->pl_len);
    } else {
        cmd->payload = NULL;
    }

    LOG_INFO(TAG_CMD, "Unpacked command %s", cmdtostr(cmd->type));
    return cmd;  
}

const char* cmdtostr(int type) {
    switch (type) {

        case CMD_PUT_DATA:
            return "CMD_PUT_DATA";
        case CMD_GET_DATA:
            return "CMD_GET_DATA";
        case CMD_DUMP_DATA:
            return "CMD_DUMP_DAT";
        case CMD_TERMINATE:
            return "CMD_TERMINATE";
        case CMD_ACQUIRE_REQUEST:
            return "CMD_ACQUIRE_REQUEST";
        case CMD_RELEASE_REQUEST:
            return "CMD_RELEASE_REQUEST";

        case CMD_PUT_DATA_ACK:
            return "CMD_PUT_DATA_ACK";
        case CMD_GET_DATA_ACK:
            return "CMD_GET_DATA_ACK";
        case CMD_GET_NO_DATA_ACK:
            return "CMD_GET_NO_DATA_ACK";
        case CMD_DUMP_DATA_ACK:
            return "CMD_DUMP_DATA_ACK";
        case CMD_TERMINATE_ACK:
            return "CMD_TERMINATE_ACK";
        case CMD_TERMINATE_DENY:
            return "CMD_TERMINATE_DENY";
        case CMD_ACQUIRE_ACK:
            return "CMD_ACQUIRE_ACK";
        case CMD_RELEASE_ACK:
            return "CMD_RELEASE_ACK";

        case CMD_REGISTER_DONE:
            return "CMD_REGISTER_DONE";
        case CMD_DEREGISTER_DONE:
            return "CMD_DEREGISTER_DONE";
        case CMD_BLOCKS_MAINTAINED:
            return "CMD_BLOCKS_MAINTAINED";

        default:
            return "CMD_UNKNOWN";
    }
}
