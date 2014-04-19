#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "typedefs.h"
#include "dhtpackettypes.h"
#include "dhtpacket.h"
#include "socketio.h"
#include "fileio.h"
#include "hash.h"
#include "keyring.h"
#include "log.h"

#define TAG_NODE "Node"

int main(int argc, char **argv) {
    if (argc != 5) {
        DIE(TAG_NODE, "Give host and server ports and addresses as argument");
    }

    char *host_address = argv[1];
    char *host_port = argv[2];
    char *server_address = argv[3];
    char *server_port = argv[4];
    char *blockdir = "blocks";    // TODO: Ask user for path
           
    // Status info     
    int status = 0;
    int running = 1;
    int regs_no = 0;
    int blocks_no = 0;
    
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
    int cmdsock = fileno(stdin);
    int listensock = create_listen_socket(host_port);
    int servsock = -1;
    int leftsock = -1;  // left neighbour socket
    int rightsock = -1; // right neighbour socket

    // Data buffers
    byte *sendbuf = malloc(MAX_PACKET_SIZE);
    byte *recvbuf = malloc(MAX_PACKET_SIZE);
    byte *blockbuf = malloc(MAX_BLOCK_SIZE);
    memset(sendbuf, 0, MAX_PACKET_SIZE);
    memset(recvbuf, 0, MAX_PACKET_SIZE);
    memset(blockbuf, 0, MAX_BLOCK_SIZE);
    int blocklen = 0;

    // Connect to server
    open_conn(&servsock, &serv_addr);
    sleep(1);   // Server doesn't accept handshake if sent immediately
    init_hs(servsock);

    hash_addr(&host_addr, host_key);
    hash_addr(&serv_addr, serv_key);

    struct keyring *ring = init_ring(host_key);

    // Send DHT_REGISTER_BEGIN
    byte *host_addr_pl = NULL;
    int host_addr_pl_len = addr_to_pl(&host_addr_pl, &host_addr);
    sendpacket(servsock, sendbuf, serv_key, host_key,
               DHT_REGISTER_BEGIN, host_addr_pl, host_addr_pl_len);
    
    // Start main loop (connection sequence isn't actually
    // finished at this point)
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

        // Let's hope 10 is high enough for numfds...
        status = select(10, &rfds, NULL, NULL, NULL);

        if (status == -1) {
            LOG_WARN(TAG_NODE, "Select failed");
        } else if (FD_ISSET(cmdsock, &rfds)) {
            if (CMD_USE_STDIN) {
                read(cmdsock, recvbuf, MAX_PACKET_SIZE);
                if (recvbuf[0] == 'q') {
                    LOG_INFO(TAG_NODE, "Requesting permission to leave");
                    sendpacket(servsock, sendbuf, host_key, host_key,
                               DHT_DEREGISTER_BEGIN, NULL, 0);
                }
            } else {
                recvcmd(cmdsock, recvbuf, MAX_PACKET_SIZE);
                struct cmd *cmd = unpack_cmd(recvbuf);
                switch (cmd->type) {
                    case CMD_PUT_DATA:
                        sendpacket(servsock, sendbuf, cmd->key, host_key,
                                   DHT_PUT_DATA, cmd->payload, cmd->pl_len);
                        break;
                    case CMD_GET_DATA:
                        sendpacket(servsock, sendbuf, cmd->key, host_key,
                                   DHT_GET_DATA, host_addr_pl, host_addr_pl_len);
                        break;
                    case CMD_DUMP_DATA:
                        sendpacket(servsock, sendbuf, cmd->key, host_key,
                                   DHT_DUMP_DATA, cmd->payload, cmd->pl_len);
                        break;
                    case CMD_TERMINATE:
                        LOG_INFO(TAG_NODE, "Requesting permission to leave");
                        sendpacket(servsock, sendbuf, host_key, host_key,
                                   DHT_DEREGISTER_BEGIN, NULL, 0);
                        break;
                    case CMD_ACQUIRE_REQUEST:
                        sendpacket(servsock, sendbuf, cmd->key, host_key,
                                   DHT_ACQUIRE_REQUEST, NULL, 0);
                        break;
                    case CMD_RELEASE_REQUEST:
                        sendpacket(servsock, sendbuf, cmd->key, host_key,
                                   DHT_RELEASE_REQUEST, NULL, 0);
                        break;
                    default:
                        LOG_WARN(TAG_NODE, "Invalid command type %d", cmd->type);
                }
                free(cmd->payload);
                free(cmd);
            }
            
        } else if (FD_ISSET(leftsock, &rfds) || FD_ISSET(rightsock, &rfds)) {
            int *tempsock;
            if (FD_ISSET(leftsock, &rfds)) {
                tempsock = &leftsock;
            } else if (FD_ISSET(rightsock, &rfds)) {
                tempsock = &rightsock;
            }
            recvall(*tempsock, recvbuf, MAX_PACKET_SIZE);
            struct packet *packet = unpack(recvbuf);
            switch (packet->type) {
                case DHT_TRANSFER_DATA:
                    // Store data received from neighbour
                    add_key(ring, packet->target);
                    write_block(blockdir, packet->target, packet->payload, packet->pl_len);
                    blocks_no++;
                    break;
                case DHT_SEND_DATA:
                    // Send payload to Java
                    sendcmd(cmdsock, sendbuf, packet->target,
                            CMD_GET_DATA_ACK, packet->payload, packet->pl_len);
                    break;
                case DHT_NO_DATA:
                    // Inform Java that no data was found
                    sendcmd(cmdsock, sendbuf, packet->target,
                            CMD_GET_NO_DATA_ACK, NULL, 0);
                    break;
                case DHT_REGISTER_ACK:
                    // Received all data from neighbour. Send DONE to server if
                    // ACK received from both neighbours and inform Java.
                    regs_no++;
                    if (regs_no == 2) {
                        sendpacket(servsock, sendbuf, host_key, host_key,
                                   DHT_REGISTER_DONE, NULL, 0);
                        sendcmd(cmdsock, sendbuf, host_key,
                                CMD_REGISTER_DONE, NULL, 0);
                    }

                    close(*tempsock);
                    *tempsock = -1;
                    break;
                case DHT_DEREGISTER_ACK:
                    // Received all data from leaving neighbour, acknowledge this to server
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

        } else if (FD_ISSET(servsock, &rfds)) {
            int tempsock;
            recvall(servsock, recvbuf, MAX_PACKET_SIZE);
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
                    // New node is trying to join, connect and send data
                    ; // Compiler throws error without this semicolon
                    
                    build_tcp_addr(packet->payload, &temp_addr, NULL);
                    open_conn(&tempsock, &temp_addr);
                    
                    // Handshake with new node
                    sha1_t nb_key;
                    hash_addr(&temp_addr, nb_key);
                    init_hs(tempsock);

                    // Send data
                    sha1_t mid_clock;
                    sha1_t mid_counter;
                    calc_mid(host_key, nb_key, mid_clock, 1);
                    calc_mid(host_key, nb_key, mid_counter, -1);
                    struct keyring *slice = slice_ring(ring, mid_clock, mid_counter);
                    struct keyring *slice_n = slice;
                    while (slice_n != NULL) {
                        blocklen = read_block(blockdir, slice_n->key, blockbuf, MAX_BLOCK_SIZE);
                        if (blocklen > 0) {
                            sendpacket(tempsock, sendbuf, slice_n->key, host_key,
                                       DHT_TRANSFER_DATA, blockbuf, blocklen);
                        }
                        rm_block(blockdir, slice_n->key);
                        blocks_no--;
                    }
                    free_ring(slice);

                    // Send ACK to new node to inform that all data is sent
                    sendpacket(tempsock, sendbuf, nb_key, nb_key,
                            DHT_REGISTER_ACK, NULL, 0);
                    close(tempsock);
                    break;
                case DHT_REGISTER_DONE:
                    // TODO Forget data sent to new neighbour (currently blocks are actually
                    // dumped immediately after they are sent).
                    break;
                case DHT_DEREGISTER_BEGIN:
                    // A node leaves abnormally
                    sendpacket(servsock, sendbuf, packet->sender, host_key,
                            DHT_DEREGISTER_DONE, NULL, 0);
                    break;
                case DHT_DEREGISTER_ACK:
                    // Server has responded to our attempt to leave, create connections to both neighbours
                    // Inform Java that disconnection sequence has begun.
                    build_tcp_addr(packet->payload, &left_addr, &right_addr);

                    // Connect to left neighbour
                    open_conn(&leftsock, &left_addr);
                    init_hs(leftsock);

                    // Connect to right neighbour
                    open_conn(&rightsock, &right_addr);
                    init_hs(rightsock);

                    // Inform Java
                    sendcmd(cmdsock, sendbuf, packet->target,
                            CMD_TERMINATE_ACK, NULL, 0);

                    running = 0;
                    break;
                case DHT_DEREGISTER_DENY:
                    // Disconnection attempt denied, inform Java
                    sendcmd(cmdsock, sendbuf, packet->target,
                            CMD_TERMINATE_DENY, NULL, 0);
                    LOG_WARN(TAG_NODE, "Request to leave denied");
                    break;
                case DHT_GET_DATA:
                    ;// Some node requested data. Open connection to requesting node and send data
                    
                    build_tcp_addr(packet->payload, &temp_addr, NULL);
                    open_conn(&tempsock, &temp_addr);
                    
                    // Handshake with new node
                    init_hs(tempsock);

                    if (has_key(ring, packet->target)) {
                        blocklen = read_block(blockdir, packet->target, blockbuf, MAX_BLOCK_SIZE);
                        if (blocklen > 0) {
                            sendpacket(tempsock, sendbuf, packet->target, host_key,
                                       DHT_SEND_DATA, blockbuf, blocklen);
                        }
                    } else {
                        sendpacket(tempsock, sendbuf, packet->target, host_key,
                                   DHT_NO_DATA, NULL, 0);
                    }
                    close(tempsock);
    
                    break;
                case DHT_PUT_DATA:
                    // Some node added data
                    add_key(ring, packet->target);
                    write_block(blockdir, packet->target, packet->payload, packet->pl_len);
                    blocks_no++;
                    sendpacket(servsock, sendbuf, packet->target, packet->sender,
                               DHT_PUT_DATA_ACK, NULL, 0);
                    break;
                case DHT_PUT_DATA_ACK:
                    // Data added successfully
                    sendcmd(cmdsock, sendbuf, packet->target,
                            CMD_PUT_DATA_ACK, NULL, 0);
                    break;
                case DHT_DUMP_DATA:
                    // Some node requested data removal
                    // Remove data (if exists) and send ACK to server
                    if (!(del_key(ring, packet->target))) {
                        rm_block(blockdir, packet->target);
                        blocks_no--;
                    }
                    sendpacket(servsock, sendbuf, packet->target, packet->sender,
                            DHT_DUMP_DATA_ACK, NULL, 0);
                    break;
                case DHT_DUMP_DATA_ACK:
                    // Data was removed
                    sendcmd(cmdsock, sendbuf, packet->target,
                            CMD_DUMP_DATA_ACK, NULL, 0);
                    break;
                case DHT_ACQUIRE_ACK:
                    // A lock was acquired
                    sendcmd(cmdsock, sendbuf, packet->target,
                            CMD_ACQUIRE_ACK, NULL, 0);
                    break;
                case DHT_RELEASE_ACK:
                    // A lock was released
                    sendcmd(cmdsock, sendbuf, packet->target,
                            CMD_RELEASE_ACK, NULL, 0);
                    break;
                default:
                    LOG_WARN(TAG_NODE, "Invalid packet type %d", packet->type);
            }
            free(packet->payload);
            free(packet);

        } else if (FD_ISSET(listensock, &rfds)) {
            // Other node tries to connect
            struct sockaddr_in tempaddr;
            int tempsock;
            unsigned int addrlen = sizeof(struct sockaddr_storage);

            if ((tempsock = accept(listensock, (struct sockaddr*)&tempaddr, &addrlen)) == -1) {
                DIE(TAG_NODE, "%s", strerror(errno));
            }

            if (wait_hs(tempsock) == 0) {
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
    int deregs_no = 0;

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
            DIE(TAG_NODE, "Select failed");
        } else if (FD_ISSET(servsock, &rfds)) {
            recvall(servsock, recvbuf, MAX_PACKET_SIZE);
            struct packet *packet = unpack(recvbuf);
            if (packet->type == DHT_DEREGISTER_DONE) {
                    // Simply leave after receiving confirmations from
                    // both neighbours
                    deregs_no++;
                    if (deregs_no == 2) {
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
                DIE(TAG_NODE, "Invalid socket selected");
            }

            if (*slice_n != NULL) {
                blocklen = read_block(blockdir, (*slice_n)->key, blockbuf, MAX_BLOCK_SIZE);
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

            // Inform Java about number of blocks still maintained
            sendcmd(cmdsock, sendbuf, host_key,
                    CMD_BLOCKS_MAINTAINED, (byte *)&blocks_no, sizeof(int));
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
