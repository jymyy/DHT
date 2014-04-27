#include "dhtpacket.h"

int pack(byte *buf, sha1_t target_key, sha1_t sender_key,
         uint16_t type, byte *payload, uint16_t pl_len) {

    uint16_t type_htons = htons(type);
    uint16_t pl_len_htons = htons(pl_len);
    memcpy(buf+TARGET_OFFSET, target_key, sizeof(sha1_t));
    memcpy(buf+SENDER_OFFSET, sender_key, sizeof(sha1_t));
    memcpy(buf+TYPE_OFFSET, &type_htons, sizeof(uint16_t));
    memcpy(buf+PL_LEN_OFFSET, &pl_len_htons, sizeof(uint16_t));
    if (payload != NULL) {
        memcpy(buf+PAYLOAD_OFFSET, payload, pl_len);
    }

    LOG_INFO(TAG_PACKET, "Packed %s", packettostr(type));
    if (LOG_LEVEL >= INFO_LEVEL) {
        char target_str[SHA1_DEBUG_LEN];
        char sender_str[SHA1_DEBUG_LEN];
        shatostr(target_key, target_str, SHA1_DEBUG_LEN);
        shatostr(sender_key, sender_str, SHA1_DEBUG_LEN);
    
        LOG_INFO(TAG_PACKET, "Target: %s, sender: %s, length: %d",
                  target_str, sender_str, pl_len);
    }
    
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
    //if (buf[0] == '?') {
    //    offset = 1;
    //}
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

    LOG_INFO(TAG_PACKET, "Unpacked %s", packettostr(packet->type));
    if (LOG_LEVEL >= INFO_LEVEL) {
        char target_str[SHA1_DEBUG_LEN];
        char sender_str[SHA1_DEBUG_LEN];
        shatostr(packet->target, target_str, SHA1_DEBUG_LEN);
        shatostr(packet->sender, sender_str, SHA1_DEBUG_LEN);
    
        LOG_INFO(TAG_PACKET, "Target: %s, sender: %s, length: %d",
                  target_str, sender_str, packet->pl_len);
    }
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

const char* packettostr(int type) {
    
    switch (type) {
        case DHT_REGISTER_BEGIN:
            return "DHT_REGISTER_BEGIN";
        case DHT_REGISTER_ACK:
            return "DHT_REGISTER_ACK";
        case DHT_REGISTER_FAKE_ACK:
            return "DHT_REGISTER_FAKE_ACK";
        case DHT_REGISTER_DONE:
            return "DHT_REGISTER_DONE";
            
        case DHT_DEREGISTER_BEGIN:
            return "DHT_DEREGISTER_BEGIN";
        case DHT_DEREGISTER_ACK:
            return "DHT_DEREGISTER_ACK";
        case DHT_DEREGISTER_DONE:
            return "DHT_DEREGISTER_DONE";
        case DHT_DEREGISTER_DENY:
            return "DHT_DEREGISTER_DENY";
            
            
        case DHT_GET_DATA:
            return "DHT_GET_DATA";
        case DHT_PUT_DATA:
            return "DHT_PUT_DATA";
        case DHT_DUMP_DATA:
            return "DHT_DUMP_DATA";
        case DHT_PUT_DATA_ACK:
            return "DHT_PUT_DATA_ACK";
        case DHT_DUMP_DATA_ACK:
            return "DHT_DUMP_DATA_ACK";
        case DHT_SEND_DATA:
            return "DHT_SEND_DATA";
        case DHT_TRANSFER_DATA:
            return "DHT_TRANSFER_DATA";
        case DHT_NO_DATA:
            return "DHT_NO_DATA";
            
            
        case DHT_ACQUIRE_REQUEST:
            return "DHT_ACQUIRE_REQUEST";
        case DHT_ACQUIRE_ACK:
            return "DHT_ACQUIRE_ACK";
        case DHT_RELEASE_REQUEST:
            return "DHT_RELEASE_REQUEST";
        case DHT_RELEASE_ACK:
            return "DHT_RELEASE_ACK";
            
        default:
            return "UNKNOWN_PACKET_TYPE";
    }
}
