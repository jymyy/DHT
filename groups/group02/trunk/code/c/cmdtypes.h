// Commands from GUI
#define CMD_PUT_DATA 1
#define CMD_GET_DATA 2
#define CMD_DUMP_DATA 3
#define CMD_TERMINATE 4
#define CMD_ACQUIRE_REQUEST 5
#define CMD_RELEASE_REQUEST 6

// Responses to commands from GUI
#define CMD_PUT_DATA_ACK 11
#define CMD_GET_DATA_ACK 12
#define CMD_GET_NO_DATA_ACK 13
#define CMD_DUMP_DATA_ACK 14
#define CMD_TERMINATE_ACK 15
#define CMD_TERMINATE_DENY 16
#define CMD_ACQUIRE_ACK 17
#define CMD_RELEASE_ACK 18

// Information to GUI
#define CMD_REGISTER_DONE 21
#define CMD_DEREGISTER_DONE 22
#define CMD_BLOCKS_MAINTAINED 23

#define CMD_GUI_SHAKE    0x4721     // G!