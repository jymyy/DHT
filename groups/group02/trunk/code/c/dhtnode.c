#include "dhtnode.h"

int loglevel = DEBUG_LEVEL; // This sets default log level

/**
 * Option:              Short:  Long:           Default:
 * Host address         -A      --hostaddr      localhost
 * Host port            -P      --hostport      2000
 * Server address       -a      --servaddr      localhost
 * Server port          -p      --servport      1234
 * Block directory      -b      --blockdir      ./blocks/
 * Logging level        -l      --loglevel      4 (debug)
 */
int main(int argc, char **argv) {
    char *host_address = NULL;
    char *host_port = NULL;
    char *server_address = NULL;
    char *server_port = NULL;
    char *blockdir = NULL;

    int opt_index;
    static struct option long_opts[] = {
          {"hostaddr", required_argument, NULL, 'A'},
          {"hostport", required_argument, NULL, 'P'},
          {"servaddr", required_argument, NULL, 'a'},
          {"servport", required_argument, NULL, 'p'},
          {"blockdir", required_argument, NULL, 'b'},
          {"loglevel", required_argument, NULL, 'l'},
          {0, 0, 0, 0}
    };

    int c;
    while ((c = getopt_long(argc, argv, "A:P:a:p:b:l:", long_opts, &opt_index)) != -1) {
        switch (c) {
            case 'A':
                host_address = optarg;
                break;
            case 'P':
                host_port = optarg;
                break;
            case 'a':
                server_address = optarg;
                break;
            case 'p':
                server_port = optarg;
                break;
            case 'b':
                blockdir = optarg;
                break;
            case 'l':
                if (*optarg == '0') {
                    loglevel = 0;
                } else if (atoi(optarg)) {
                    loglevel =  atoi(optarg);
                }
                break;
            case '?':
                if (optopt == 'A' || optopt == 'P' || optopt == 'a' ||
                    optopt == 'p' || optopt == 'b' || optopt == 'l') {
                    LOG_ERROR(TAG_NODE, "Option -%c requires argument, reverting to default", optopt);
                } else {
                    LOG_ERROR(TAG_NODE, "Unknown option -%c", optopt);
                }
        }
    }

    // Use defaults if options were not specified
    if (host_address == NULL) { host_address = "localhost"; }
    if (host_port == NULL) { host_port = "2000"; }
    if (server_address == NULL) { server_address = "localhost"; }
    if (server_port == NULL) { server_port = "1234"; }
    if (blockdir == NULL) { blockdir = "blocks"; }

    LOG_INFO(TAG_NODE, "Host address: %s", host_address);
    LOG_INFO(TAG_NODE, "Host port: %s", host_port);
    LOG_INFO(TAG_NODE, "Server address: %s", server_address);
    LOG_INFO(TAG_NODE, "Server port: %s", server_port);
    LOG_INFO(TAG_NODE, "Block directory: %s", blockdir);
  
    int status = 0;     // Return value for some functions
    int running = 1;    // Flag for main loop (changed to 0 when disconnection sequence starts)
    int regs_rcvd = 0;  // Number of received DHT_REGISTER_ACKs
    int blocks_no = 0;  // Number of maintained blocks
    
    // Address structs and keys
    struct tcp_addr host_addr;
    struct tcp_addr serv_addr;
    struct tcp_addr left_addr;
    struct tcp_addr right_addr;
    sha1_t host_key;
    sha1_t serv_key;

    strcpy(host_addr.port, host_port);
    strcpy(host_addr.addr, host_address);
    strcpy(serv_addr.port, server_port);
    strcpy(serv_addr.addr, server_address);

    // Sockets
    fd_set rfds;
    fd_set wfds;
    int listensock = create_listen_socket(host_port);
    int cmdsock = -1;               // GUI (set to -1) or stdin (set to 0)
    int servsock = -1;              // Server
    int leftsock = -1;              // Other node
    int rightsock = -1;             // Other node

    // Data buffers
    byte *sendbuf = malloc(MAX_PACKET_SIZE);
    byte *recvbuf = malloc(MAX_PACKET_SIZE);
    byte *blockbuf = malloc(MAX_BLOCK_SIZE);
    memset(sendbuf, 0, MAX_PACKET_SIZE);
    memset(recvbuf, 0, MAX_PACKET_SIZE);
    memset(blockbuf, 0, MAX_BLOCK_SIZE);

    // Connect to server
    open_conn(&servsock, &serv_addr);
    sleep(1);   // Server doesn't accept handshake if sent immediately
    init_hs(servsock);

    hash_addr(&host_addr, host_key);
    hash_addr(&serv_addr, serv_key);
    if (LOG_LEVEL >= INFO_LEVEL) {
        char host_key_str[SHA1_STR_LEN];
        shatostr(host_key, host_key_str, SHA1_STR_LEN);
        LOG_INFO(TAG_NODE, "Host key: %s", host_key_str);
    }

    struct keyring *ring = init_ring(host_key);

    // Send DHT_REGISTER_BEGIN
    byte *host_addr_pl = NULL;
    int host_addr_pl_len = addr_to_pl(&host_addr_pl, &host_addr);
    sendpacket(servsock, sendbuf, serv_key, host_key,
               DHT_REGISTER_BEGIN, host_addr_pl, host_addr_pl_len);
    
    // Start main loop (connection sequence not finished at this point)
    while(running) {
        // Add sockets to listening pool
        FD_ZERO(&rfds);
        FD_SET(listensock, &rfds);
        FD_SET(servsock, &rfds);
        FD_SET(cmdsock, &rfds);
        if (leftsock != -1) {
            FD_SET(leftsock, &rfds);
        } 
        if (rightsock != -1) {
            FD_SET(rightsock, &rfds);
        }

        // Reset buffers
        memset(sendbuf, 0, MAX_PACKET_SIZE);
        memset(recvbuf, 0, MAX_PACKET_SIZE);
        memset(blockbuf, 0, MAX_BLOCK_SIZE);

        // Value for numfds is currently arbitrary
        status = select(10, &rfds, NULL, NULL, NULL);

        if (status == -1) {
            LOG_ERROR(TAG_NODE, "Select failed");
        } else if (FD_ISSET(cmdsock, &rfds)) {
            if (CMD_USE_STDIN) {
                read(cmdsock, recvbuf, MAX_PACKET_SIZE);
                if (recvbuf[0] == 'q') {
                    LOG_INFO(TAG_NODE, "Requesting permission to leave");
                    sendpacket(servsock, sendbuf, host_key, host_key,
                               DHT_DEREGISTER_BEGIN, NULL, 0);
                }
            } else {
                status = recvcmd(cmdsock, recvbuf, MAX_PACKET_SIZE);
                if (status == 0) {
                    close(cmdsock);
                    cmdsock = -1;
                } else {
                    struct cmd *cmd = unpack_cmd(recvbuf);
                    switch (cmd->type) {
                        case CMD_PUT_DATA:
                            // Add data
                            sendpacket(servsock, sendbuf, cmd->key, host_key,
                                       DHT_PUT_DATA, cmd->payload, cmd->pl_len);
                            break;
                        case CMD_GET_DATA:
                            // Request data
                            sendpacket(servsock, sendbuf, cmd->key, host_key,
                                   DHT_GET_DATA, host_addr_pl, host_addr_pl_len);
                            break;
                        case CMD_DUMP_DATA:
                            // Delete data
                            sendpacket(servsock, sendbuf, cmd->key, host_key,
                                   DHT_DUMP_DATA, cmd->payload, cmd->pl_len);
                            break;
                        case CMD_TERMINATE:
                            // Ask permission to leave
                            LOG_INFO(TAG_NODE, "Requesting permission to leave");
                            sendpacket(servsock, sendbuf, host_key, host_key,
                                       DHT_DEREGISTER_BEGIN, NULL, 0);
                            break;
                        case CMD_ACQUIRE_REQUEST:
                            // Request lock
                            sendpacket(servsock, sendbuf, cmd->key, host_key,
                                       DHT_ACQUIRE_REQUEST, NULL, 0);
                            break;
                        case CMD_RELEASE_REQUEST:
                            // Release lock
                            sendpacket(servsock, sendbuf, cmd->key, host_key,
                                       DHT_RELEASE_REQUEST, NULL, 0);
                            break;
                        default:
                            LOG_WARN(TAG_NODE, "Invalid command type %d", cmd->type);
                    }
                    free(cmd->payload);
                    free(cmd);
                }
            }
            
        } else if (FD_ISSET(leftsock, &rfds) || FD_ISSET(rightsock, &rfds)) {
            int *tempsock;
            if (FD_ISSET(leftsock, &rfds)) {
                tempsock = &leftsock;
            } else if (FD_ISSET(rightsock, &rfds)) {
                tempsock = &rightsock;
            }

            status = recvpacket(*tempsock, recvbuf, MAX_PACKET_SIZE);
            if (status == 0) {
                close(*tempsock);
                *tempsock = -1;
            } else {
                struct packet *packet = unpack(recvbuf);
                switch (packet->type) {
                    case DHT_TRANSFER_DATA:
                        // Store data received from neighbour
                        add_key(ring, packet->target);
                        write_block(blockdir, packet->target, packet->payload, packet->pl_len);
                        blocks_no++;
                        break;
                    case DHT_SEND_DATA:
                        // Send received data to GUI
                        sendcmd(cmdsock, sendbuf, packet->target,
                                CMD_GET_DATA_ACK, packet->payload, packet->pl_len);
                        break;
                    case DHT_NO_DATA:
                        // Inform GUI that no data was found
                        sendcmd(cmdsock, sendbuf, packet->target,
                                CMD_GET_NO_DATA_ACK, NULL, 0);
                        break;
                    case DHT_REGISTER_ACK:
                        // Received all data from neighbour. Send DONE to server if
                        // ACKs received from both neighbours.
                        regs_rcvd++;
                        if (regs_rcvd == 2) {
                            sendpacket(servsock, sendbuf, host_key, host_key,
                                       DHT_REGISTER_DONE, NULL, 0);
                            sendcmd(cmdsock, sendbuf, host_key,
                                    CMD_REGISTER_DONE, NULL, 0);
                        }

                        close(*tempsock);
                        *tempsock = -1;
                        break;
                    case DHT_DEREGISTER_ACK:
                        // Received all data from leaving neighbour
                        sendpacket(servsock, sendbuf, packet->sender, host_key,
                                   DHT_DEREGISTER_DONE, NULL, 0);
                        close(*tempsock);
                        *tempsock = -1;
                        break;
                    default:
                        LOG_WARN(TAG_NODE, "Invalid packet type %d", packet->type);
                }
                free(packet->payload);
                free(packet);
            }

        } else if (FD_ISSET(servsock, &rfds)) {
            int tempsock;
            status = recvpacket(servsock, recvbuf, MAX_PACKET_SIZE);
            if (status == 0) {
                DIE(TAG_NODE, "Server disconnected");
            } else {
                struct packet *packet = unpack(recvbuf);
                struct tcp_addr temp_addr;
                switch (packet->type) {
                    case DHT_REGISTER_FAKE_ACK:
                        // First node in network
                        sendpacket(servsock, sendbuf, host_key, host_key,
                                   DHT_REGISTER_DONE, NULL, 0);

                        sendcmd(cmdsock, sendbuf, host_key,
                                CMD_REGISTER_DONE, NULL, 0);
                        break;
                    case DHT_REGISTER_BEGIN:
                        // New node is trying to join; connect and send data
                        build_tcp_addr(packet->payload, &temp_addr, NULL);
                        open_conn(&tempsock, &temp_addr);
                        init_hs(tempsock);
                        
                        // Hash neighbour address
                        sha1_t nb_key;
                        hash_addr(&temp_addr, nb_key);
                        sha1_t mid_clock;
                        sha1_t mid_counter;
                        calc_mid(host_key, nb_key, mid_clock, 1);
                        calc_mid(host_key, nb_key, mid_counter, -1);
                        struct keyring *slice = slice_ring(ring, mid_clock, mid_counter);
                        struct keyring *slice_n = slice;
                        while (slice_n != NULL) {
                            int blocklen = read_block(blockdir, slice_n->key, blockbuf, MAX_BLOCK_SIZE);
                            if (blocklen > 0) {
                                sendpacket(tempsock, sendbuf, slice_n->key, host_key,
                                           DHT_TRANSFER_DATA, blockbuf, blocklen);
                            }
                            rm_block(blockdir, slice_n->key);
                            blocks_no--;
                            slice_n = slice_n->next;
                        }
                        free_ring(slice);

                        // Send ACK to new node to inform that all data is sent
                        sendpacket(tempsock, sendbuf, nb_key, nb_key,
                                DHT_REGISTER_ACK, NULL, 0);
                        close(tempsock);
                        break;
                    case DHT_REGISTER_DONE:
                        // TODO: Forget data sent to new neighbour (currently blocks are actually
                        // dumped immediately after they are sent).
                        break;
                    case DHT_DEREGISTER_BEGIN:
                        // Other node leaves abnormally
                        sendpacket(servsock, sendbuf, packet->sender, host_key,
                                   DHT_DEREGISTER_DONE, NULL, 0);
                        break;
                    case DHT_DEREGISTER_ACK:
                        // Server has responded to our attempt to leave, create connections
                        // to both neighbours. Inform GUI that disconnection sequence has begun.
                        build_tcp_addr(packet->payload, &left_addr, &right_addr);

                        // Connect to left neighbour
                        open_conn(&leftsock, &left_addr);
                        init_hs(leftsock);

                        // Connect to right neighbour
                        open_conn(&rightsock, &right_addr);
                        init_hs(rightsock);

                        // Inform GUI
                        sendcmd(cmdsock, sendbuf, packet->target,
                                CMD_TERMINATE_ACK, NULL, 0);

                        running = 0;
                        break;
                    case DHT_DEREGISTER_DENY:
                        // Disconnection attempt denied
                        sendcmd(cmdsock, sendbuf, packet->target,
                                CMD_TERMINATE_DENY, NULL, 0);
                        LOG_WARN(TAG_NODE, "Request to leave denied");
                        break;
                    case DHT_GET_DATA:
                        // Other node requested data
                        ; int isnotself = hashcmp(host_key, packet->sender);

                        if (isnotself) {
                            build_tcp_addr(packet->payload, &temp_addr, NULL);
                            open_conn(&tempsock, &temp_addr);
                            init_hs(tempsock);
                        }

                        // Check if we have requested data
                        if (has_key(ring, packet->target)) {
                            int blocklen = read_block(blockdir, packet->target,
                                                      blockbuf, MAX_BLOCK_SIZE);
                            if (blocklen > 0) {
                                if (isnotself) {
                                    sendpacket(tempsock, sendbuf, packet->target, host_key,
                                           DHT_SEND_DATA, blockbuf, blocklen);
                                    close(tempsock);
                                } else {
                                    sendcmd(cmdsock, sendbuf, packet->target,
                                            CMD_GET_DATA_ACK, blockbuf, blocklen);
                                }
                            } else {
                                LOG_ERROR(TAG_NODE ,"Error reading block");
                            }

                        } else {
                            if (isnotself) {
                                sendpacket(tempsock, sendbuf, packet->target, host_key,
                                       DHT_NO_DATA, NULL, 0);
                                close(tempsock);
                            } else {
                                sendcmd(cmdsock, sendbuf, packet->target,
                                        CMD_GET_NO_DATA_ACK, NULL, 0);
                            }
                            
                        }
        
                        break;
                    case DHT_PUT_DATA:
                        // Other node added data
                        add_key(ring, packet->target);
                        write_block(blockdir, packet->target, packet->payload, packet->pl_len);
                        blocks_no++;
                        sendpacket(servsock, sendbuf, packet->target, packet->sender,
                                   DHT_PUT_DATA_ACK, NULL, 0);
                        break;
                    case DHT_PUT_DATA_ACK:
                        // Data added
                        sendcmd(cmdsock, sendbuf, packet->target,
                                CMD_PUT_DATA_ACK, NULL, 0);
                        break;
                    case DHT_DUMP_DATA:
                        // Other node requested data removal
                        if (!(del_key(ring, packet->target))) {
                            rm_block(blockdir, packet->target);
                            blocks_no--;
                        }
                        sendpacket(servsock, sendbuf, packet->target, packet->sender,
                                DHT_DUMP_DATA_ACK, NULL, 0);
                        break;
                    case DHT_DUMP_DATA_ACK:
                        // Data removed
                        sendcmd(cmdsock, sendbuf, packet->target,
                                CMD_DUMP_DATA_ACK, NULL, 0);
                        break;
                    case DHT_ACQUIRE_ACK:
                        // Lock acquired
                        sendcmd(cmdsock, sendbuf, packet->target,
                                CMD_ACQUIRE_ACK, NULL, 0);
                        break;
                    case DHT_RELEASE_ACK:
                        // Lock released
                        sendcmd(cmdsock, sendbuf, packet->target,
                                CMD_RELEASE_ACK, NULL, 0);
                        break;
                    default:
                        LOG_WARN(TAG_NODE, "Invalid packet type %d", packet->type);
                }
                free(packet->payload);
                free(packet);
            }

        } else if (FD_ISSET(listensock, &rfds)) {
            // Other node tries to connect
            struct sockaddr_in tempaddr;
            int tempsock;
            unsigned int addrlen = sizeof(struct sockaddr_storage);

            if ((tempsock = accept(listensock, (struct sockaddr*)&tempaddr, &addrlen)) == -1) {
                DIE(TAG_NODE, "%s", strerror(errno));
            } else if (wait_hs(tempsock) == 0) {
                cmdsock = tempsock;
            } else if (leftsock == -1) {
                leftsock = tempsock;
            } else if (leftsock != -1 && rightsock == -1) {
                rightsock = tempsock;
            } else {
                LOG_WARN(TAG_NODE, "Max connections reached");
                close(tempsock);
            }
            
        }
    }

    // Server accepted our leaving attempt and connections
    // to both neighbours have been established. Send data to
    // neighbours and wait confirmation from server.
    int disconnecting = 1;
    int deregs_rcvd = 0;

    sha1_t left_key;
    hash_addr(&left_addr, left_key);
    sha1_t right_key;
    hash_addr(&right_addr, right_key);

    sha1_t key_min;
    sha1_t key_max;
    memset(&key_min, 0, SHA1_KEY_LEN);
    memset(&key_max, 255, SHA1_KEY_LEN);
    struct keyring *slice_left = slice_ring(ring, host_key, key_max);
    struct keyring *slice_right = slice_ring(ring, host_key, key_min);
    struct keyring *slice_left_n = slice_left; 
    struct keyring *slice_right_n = slice_right;

    struct keyring *slice;
    struct keyring **slice_n;
    int *tempsock;
    sha1_t *temp_key;
    
    while (disconnecting) {
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_SET(servsock, &rfds);
        if (leftsock != -1) {
            FD_SET(leftsock, &wfds);
        } 
        if (rightsock != -1) {
            FD_SET(rightsock, &wfds);
        }
        memset(sendbuf, 0, MAX_PACKET_SIZE);
        memset(recvbuf, 0, MAX_PACKET_SIZE);
        memset(blockbuf, 0, MAX_BLOCK_SIZE);

        status = select(10, &rfds, &wfds, NULL, NULL);

        if (status == -1) {
            LOG_ERROR(TAG_NODE, "Select failed");
        } else if (FD_ISSET(servsock, &rfds)) {
            recvpacket(servsock, recvbuf, MAX_PACKET_SIZE);
            struct packet *packet = unpack(recvbuf);
            if (packet->type == DHT_DEREGISTER_DONE) {
                    // Simply leave after receiving confirmations from
                    // both neighbours
                    deregs_rcvd++;
                    if (deregs_rcvd == 2) {
                        sendcmd(cmdsock, sendbuf, packet->target,
                                CMD_DEREGISTER_DONE, NULL, 0);
                        disconnecting = 0;
                    }
                    free(packet->payload);
                    free(packet);
            } else {
                LOG_WARN(TAG_NODE, "Unexpected packet type %s", packettostr(packet->type));
            }
        } else {
            if (FD_ISSET(leftsock, &wfds)) {
                slice = slice_left;
                slice_n = &slice_left_n;
                tempsock = &leftsock;
                temp_key = &left_key;
            } else if (FD_ISSET(rightsock, &wfds)) {
                slice = slice_right;
                slice_n = &slice_right_n;
                tempsock = &rightsock;
                temp_key = &right_key;
            } else {
                LOG_WARN(TAG_NODE, "Invalid socket selected");
            }

            if (*slice_n != NULL) {
                int blocklen = read_block(blockdir, (*slice_n)->key, blockbuf, MAX_BLOCK_SIZE);
                if (blocklen > 0) {
                    sendpacket(*tempsock, sendbuf, (*slice_n)->key, host_key,
                               DHT_TRANSFER_DATA, blockbuf, blocklen);
                }
                rm_block(blockdir, (*slice_n)->key);
                *slice_n = (*slice_n)->next;
                blocks_no--;
            } else {
                sendpacket(*tempsock, sendbuf, *temp_key, host_key,
                           DHT_DEREGISTER_ACK, NULL, 0);
                close(*tempsock);
                *tempsock = -1;
                free_ring(slice);
            }

            // Inform GUI about number of blocks still maintained
            sendcmd(cmdsock, sendbuf, host_key,
                    CMD_BLOCKS_MAINTAINED, (byte *)&blocks_no, sizeof(int));
            LOG_DEBUG(TAG_NODE, "Blocks maintained %d", blocks_no);
        }
    }

    // Cleanup
    close(listensock);
    close(servsock);
    free_ring(ring);
    free(host_addr_pl);
    free(sendbuf);
    free(recvbuf);
    free(blockbuf);
    
    return 0;
}
