#include "dhtpacket.h"
#include "typedefs.h"

int pack(byte *buf, int buflen, sha1_t target_key, sha1_t sender_key, uint16_t type, byte *payload, uint16_t pl_len) {
    memcpy(buf+TARGET_OFFSET, target_key, sizeof(sha1_t));
    memcpy(buf+SENDER_OFFSET, sender_key, sizeof(sha1_t));
    uint16_t type_htons = htons(type);
    uint16_t pl_len_htons = htons(pl_len);
    memcpy(buf+TYPE_OFFSET, &type_htons, sizeof(uint16_t));
    memcpy(buf+PL_LEN_OFFSET, &pl_len_htons, sizeof(uint16_t));
    if (payload != NULL) {
        memcpy(buf+PAYLOAD_OFFSET, payload, pl_len);
    }
    
    DEBUG("Packing type %d\n", type);
    return PACKET_HEADER_LEN + pl_len;
}

struct packet* unpack(byte *buf, int packetlen) {
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

    DEBUG("Unpacking type %d with offset of %d\n", packet->type, offset);
    return packet;  
    
}

/*
 * Build TCP address(es) from packet payload.
 */
int build_tcp_addr(byte *payload, struct tcp_addr *left, struct tcp_addr *right) {
    uint16_t port = 0;
    uint16_t offset = 0;
    memcpy(&port, payload, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    port = ntohs(port);
    snprintf(left->port, 5, "%d", port);
    strcpy(left->addr, (char *) payload+offset);
    
    if (right != NULL) {
        offset += strlen(left->addr)+1;
        memcpy(&port, payload+offset, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        port = ntohs(port);
        snprintf(right->port, 5, "%d", port);
        strcpy(right->addr, (char *) payload+sizeof(uint16_t));
    }
    return 0;
}
