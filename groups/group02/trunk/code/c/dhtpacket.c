#include "dhtpacket.h"

int pack(byte *buf, sha1_t target_key, sha1_t sender_key,
         uint16_t type, byte *payload, uint16_t pl_len) {

    memcpy(buf+TARGET_OFFSET, target_key, sizeof(sha1_t));
    memcpy(buf+SENDER_OFFSET, sender_key, sizeof(sha1_t));
    uint16_t type_htons = htons(type);
    uint16_t pl_len_htons = htons(pl_len);
    memcpy(buf+TYPE_OFFSET, &type_htons, sizeof(uint16_t));
    memcpy(buf+PL_LEN_OFFSET, &pl_len_htons, sizeof(uint16_t));
    if (payload != NULL) {
        memcpy(buf+PAYLOAD_OFFSET, payload, pl_len);
    }
    
    LOG_INFO(TAG_PACKET, "Packed %s", packet_type(type));
    return PACKET_HEADER_LEN + pl_len;
}

struct packet* unpack(byte *buf) {
    // There is a bug/undocumented behaviour in the server.
    // Sometimes when the server sends a packet (usually if it is a
    // DHT_REGISTER_FAKE_ACK) it starts with a single ? character, otherwise
    // the packet seems to be valid. Therefore reading has to be sometimes
    // offset by one. This same behaviour must be handled in recvall too.
    struct packet *packet = malloc(sizeof(struct packet));
    int offset = 0;
    if (buf[0] == '?') {
        offset = 1;
    }
    memcpy(packet->target, buf+TARGET_OFFSET+offset, sizeof(sha1_t));
    memcpy(packet->sender, buf+SENDER_OFFSET+offset, sizeof(sha1_t));
    memcpy(&(packet->type), buf+TYPE_OFFSET+offset, sizeof(uint16_t));
    memcpy(&(packet->pl_len), buf+PL_LEN_OFFSET+offset, sizeof(uint16_t));
    packet->type = ntohs(packet->type);
    packet->pl_len = ntohs(packet->pl_len);
    if (packet->pl_len > 0) {
    	packet->payload = malloc(packet->pl_len);
        memcpy(packet->payload, buf+PAYLOAD_OFFSET+offset, packet->pl_len);
    } else {
        packet->payload = NULL;
    }

    LOG_INFO(TAG_PACKET, "Unpacked %s", packet_type(packet->type));
    return packet;  
}

int build_tcp_addr(byte *payload, struct tcp_addr *left, struct tcp_addr *right) {
    uint16_t port = 0;
    uint16_t offset = 0;
    memcpy(&port, payload, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    port = ntohs(port);
    snprintf(left->port, 6, "%d", port);
    strcpy(left->addr, (char *) payload+offset);
    
    if (right != NULL) {
        offset += strlen(left->addr)+1;
        memcpy(&port, payload+offset, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        port = ntohs(port);
        snprintf(right->port, 6, "%d", port);
        strcpy(right->addr, (char *) payload+offset);
    }
    return 0;
}

int addr_to_pl(byte **pl, struct tcp_addr *addr) {
    (*pl) = malloc(sizeof(uint16_t) + strlen(addr->addr) + 1);
    uint16_t port = htons(atoi(addr->port));
    memcpy((*pl), &port, sizeof(uint16_t));
    memcpy((*pl)+sizeof(uint16_t), addr->addr, strlen(addr->addr) + 1);
    return sizeof(uint16_t) + strlen(addr->addr) + 1;
}

int acquire(int socket, sha1_t key, sha1_t host_key) {
    char str[SHA1_STR_LEN];
    shatostr(key, str, SHA1_STR_LEN);
    LOG_INFO(TAG_PACKET, "Requesting lock %s", str);
    byte *buf = malloc(PACKET_HEADER_LEN);
    int packetlen = pack(buf, key, host_key, DHT_ACQUIRE_REQUEST, NULL, 0);
    sendall(socket, buf, packetlen);
    recvall(socket, buf, PACKET_HEADER_LEN);

    struct packet *packet = unpack(buf);
    if (packet->type != DHT_ACQUIRE_ACK) {
        DIE(TAG_PACKET, "Invalid acquire lock response");
    }
    free(packet);
    free(buf);

    return 0;
}

int release(int socket, sha1_t key, sha1_t host_key) {
    char str[SHA1_STR_LEN];
    shatostr(key, str, SHA1_STR_LEN);
    LOG_INFO(TAG_PACKET, "Releasing lock %s", str);
    byte *buf = malloc(PACKET_HEADER_LEN);
    int packetlen = pack(buf, key, host_key, DHT_RELEASE_REQUEST, NULL, 0);
    sendall(socket, buf, packetlen);
    free(buf);

    return 0;
}


char* packet_type(int type) {
    
    switch (type) {
        case 1:
            return "DHT_REGISTER_BEGIN";
        case 2:
            return "DHT_REGISTER_ACK";
        case 3:
            return "DHT_REGISTER_FAKE_ACK";
        case 4:
            return "DHT_REGISTER_DONE";
            
            
        case 0x4121:
            return "DHT_CLIENT_SHAKE";
        case 0x413f:
            return "DHT_SERVER_SHAKE";
            
            
        case 11:
            return "DHT_DEREGISTER_BEGIN";
        case 12:
            return "DHT_DEREGISTER_ACK";
        case 13:
            return "DHT_DEREGISTER_DONE";
        case 14:
            return "DHT_DEREGISTER_DENY";
            
            
        case 21:
            return "DHT_GET_DATA";
        case 22:
            return "DHT_PUT_DATA";
        case 23:
            return "DHT_DUMP_DATA";
        case 24:
            return "DHT_PUT_DATA_ACK";
        case 25:
            return "DHT_DUMP_DATA_ACK";
        case 26:
            return "DHT_SEND_DATA";
        case 27:
            return "DHT_TRANSFER_DATA";
        case 28:
            return "DHT_NO_DATA";
            
            
        case 31:
            return "DHT_ACQUIRE_REQUEST";
        case 32:
            return "DHT_ACQUIRE_ACK";
        case 33:
            return "DHT_RELEASE_REQUEST";
        case 34:
            return "DHT_RELEASE_ACK";
            
        default:
            return "UNKNOWN_PACKET_TYPE";
    }
}
