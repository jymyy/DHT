#include "cmdpacket.h"

int sendcmd(int socket, byte *buf, sha1_t key, uint16_t type,
            byte *payload, uint16_t pl_len) {
    if (socket != -1) {                                 
        int packetlen = pack_c(buf, key, type, payload, pl_len);
        return send_c(socket, buf, packetlen);
    } else {
        return 0;
    }
}

struct cmd* recvcmd(int socket, byte *buf, int bufsize) {
    int status = recv_c(socket, buf, bufsize);
    if (status == 0) {
        return NULL;
    } else {
        return unpack_c(buf);
    }
}

int send_c(int socket, byte *sendbuf, int cmdlen) {
    LOG_DEBUG(TAG_CMD, "Sending command");
    int bytes_sent = 0;
    while (bytes_sent < cmdlen) {
        bytes_sent += send(socket, sendbuf+bytes_sent, cmdlen-bytes_sent, 0);
    }
    LOG_DEBUG(TAG_CMD, "Sent %d bytes", bytes_sent);
    return bytes_sent;  
}

int recv_c(int socket, byte *recvbuf, int bufsize) {
    LOG_DEBUG(TAG_CMD, "Receiving command");
    int bytes_total = 0;
    int bytes_received = 0;
    int bytes_missing = CMD_HEADER_LEN;
    uint16_t pl_len = 0;

    // Receive header
    while (bytes_missing > 0) {
        bytes_received = recv(socket, recvbuf+bytes_total, bytes_missing, 0);
        if (bytes_received == 0) {
            return 0;
        } else if (bytes_total > bufsize) {
            DIE(TAG_CMD, "Recvbuf overflow");
        }
        bytes_total += bytes_received;
        bytes_missing -= bytes_received;
    }

    // Check length of the packet and receive more data if needed
    memcpy(&pl_len, recvbuf+CMD_PL_LEN_OFFSET, sizeof(uint16_t));
    pl_len = ntohs(pl_len);
    bytes_received = 0;
    bytes_missing = pl_len;
    while (bytes_missing > 0) {
        bytes_received = recv(socket, recvbuf+bytes_total, bytes_missing, 0);
        if (bytes_received == 0) {
            LOG_WARN(TAG_CMD, "GUI disconnected");
            return 0;
        } else if (bytes_total > bufsize) {
            DIE(TAG_CMD, "Recvbuf overflow");
        }
        bytes_total += bytes_received;
        bytes_missing -= bytes_received;
    }
    
    LOG_DEBUG(TAG_CMD, "Received %d bytes", bytes_total);
    return bytes_total;
}

int pack_c(byte *buf, sha1_t key, uint16_t type,
             byte *payload, uint16_t pl_len) {

    uint16_t type_htons = htons(type);
    uint16_t pl_len_htons = htons(pl_len);
    memcpy(buf+CMD_KEY_OFFSET, key, sizeof(sha1_t));
    memcpy(buf+CMD_TYPE_OFFSET, &type_htons, sizeof(uint16_t));
    memcpy(buf+CMD_PL_LEN_OFFSET, &pl_len_htons, sizeof(uint16_t));
    if (payload != NULL) {
        memcpy(buf+CMD_PAYLOAD_OFFSET, payload, pl_len);
    }
    
    LOG_INFO(TAG_CMD, "Packed command %s", cmdtostr(type));
    return CMD_HEADER_LEN + pl_len;
}

struct cmd* unpack_c(byte *buf) {
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
            return "CMD_DUMP_DATA";
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
