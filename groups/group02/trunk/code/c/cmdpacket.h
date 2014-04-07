#define CMD_PUT 1
#define CMD_GET 2
#define CMD_DUMP 3
#define CMD_TERMINATE 4

#define RESPONSE_OK 11
#define RESPONSE_ERROR 12

int pack_cmd();

int unpack_cmd();