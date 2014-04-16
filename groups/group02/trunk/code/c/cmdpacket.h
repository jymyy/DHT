#define CMD_PUT 1
#define CMD_GET 2
#define CMD_DUMP 3
#define CMD_TERMINATE 4
#define CMD_GET_DIR 5
#define CMD_RELEASE_DIR 6

#define CMD_PUT_ACK 11
#define CMD_GET_ACK 12
#define CMD_GET_NO_DATA_ACK 13
#define CMD_DUMP_ACK 14
#define CMD_TERMINATE_ACK 15
#define CMD_TERMINATE_DENY 16
#define CMD_GET_DIR_ACK 17
#define CMD_RELEASE_DIR_ACK 18

#define CMD_REGISTER_ACK 21
#define CMD_DEREGISTER_ACK 22
#define CMD_DATA_TRANSFER 23

#define

int pack_cmd();

int unpack_cmd();