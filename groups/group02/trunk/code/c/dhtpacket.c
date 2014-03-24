#include "dhtpacket.h"
#include "typedefs.h"

int pack(byte **buf, int buflen, sha1_t target_key, sha1_t sender_key, uint16_t type, byte *payload, uint16_t pl_len) {
    int sha1_len = sizeof(sha1_t);
    memcpy(*buf+TARGET_OFFSET, target_key, sha1_len);
    memcpy(*buf+SENDER_OFFSET, sender_key, sizeof(sha1_t));
    uint16_t type_htons = htons(type);
    uint16_t pl_len_htons = htons(pl_len);
    memcpy(*buf+TYPE_OFFSET, &type_htons, sizeof(uint16_t));
    memcpy(*buf+PL_LEN_OFFSET, &pl_len_htons, sizeof(uint16_t));
    if (payload != NULL) {
        memcpy(*buf+PAYLOAD_OFFSET, payload, pl_len);
    }
    return PACKET_HEADER_LEN + pl_len;
}

struct packet* unpack(byte *buf, int packetlen) {
    struct packet *packet = malloc(packetlen);
    memcpy(packet->target, buf+TARGET_OFFSET, sizeof(sha1_t));
    memcpy(packet->sender, buf+SENDER_OFFSET, sizeof(sha1_t));
    memcpy(&(packet->type), buf+TYPE_OFFSET, sizeof(uint16_t));
    memcpy(&(packet->pl_len), buf+PL_LEN_OFFSET, sizeof(uint16_t));
    if (packet->pl_len > 0) {
        memcpy(packet->payload, buf+PAYLOAD_OFFSET, packet->pl_len);
    } else {
        packet->payload = NULL;
    }

    return packet;  
    
}

int build_tcp_addr(byte *payload, struct tcp_addr *left, struct tcp_addr *right) {
    memcpy(&(left->port), payload, sizeof(uint16_t));
    strcpy(left->addr, (char *) payload+sizeof(uint16_t));
    if (right != NULL) {
        memcpy(&(right->port), payload, sizeof(uint16_t));
        strcpy(left->addr, (char *) payload+sizeof(uint16_t));
    }
    return 0;
}
