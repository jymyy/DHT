#include "socketio.h"

int init_hs(int socket) {
	LOG_INFO(TAG_SOCKET, "Handshaking");
	uint16_t client_shake = htons(DHT_CLIENT_SHAKE);
    uint16_t server_shake = htons(DHT_SERVER_SHAKE);
    uint16_t buf = 0;
    send(socket, &client_shake, 2, 0);
    while (buf != server_shake) {
    	recv(socket, &buf, 2, 0);
    }
    LOG_DEBUG(TAG_SOCKET, "Handshake completed with %d", socket);
    return 0;
}

int wait_hs(int socket) {
	LOG_INFO(TAG_SOCKET, "Handshaking");
	uint16_t client_shake = htons(DHT_CLIENT_SHAKE);
    uint16_t server_shake = htons(DHT_SERVER_SHAKE);
    uint16_t gui_shake = htons(CMD_GUI_SHAKE);
    uint16_t node_shake = htons(CMD_NODE_SHAKE);
    uint16_t buf = 0;

    while (buf != client_shake && buf != gui_shake) {
    	recv(socket, &buf, 2, 0);
    }

    if (buf == client_shake) {
        send(socket, &server_shake, 2, 0);
        LOG_DEBUG(TAG_SOCKET, "Handshake completed with %d", socket);
        return socket;
    } else {
        send(socket, &node_shake, 2, 0);
        LOG_DEBUG(TAG_SOCKET, "Handshake completed with GUI");
        return 0;
    }
}

int open_conn(int *sock, struct tcp_addr *addr) {
    LOG_INFO(TAG_SOCKET, "Opening connection");
    LOG_DEBUG(TAG_SOCKET, "Address: %s", addr->addr);
    LOG_DEBUG(TAG_SOCKET, "Port: %s", addr->port);
    int status = 0;
    struct addrinfo hints, *info;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((status = getaddrinfo(addr->addr, addr->port, &hints, &info)) != 0) {
        DIE(TAG_SOCKET, "%s", gai_strerror(status));
    }
    if ((*sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == -1) {
        DIE(TAG_SOCKET, "%s", strerror(errno));
    }
    if ((status = connect(*sock, info->ai_addr, info->ai_addrlen)) == -1) {
        DIE(TAG_SOCKET, "%s", strerror(errno));
        close(*sock);
    }
    freeaddrinfo(info);
    LOG_DEBUG(TAG_SOCKET, "Connection opened");
    return 0;
}

int create_listen_socket(char *port) {
    int fd;
    int t;

    struct sockaddr_in a;

    a.sin_addr.s_addr = INADDR_ANY;
    a.sin_family = AF_INET;
    a.sin_port = htons(atoi(port));

    fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        DIE(TAG_SOCKET, "%s", strerror(errno));

    t = bind(fd, (struct sockaddr *)(&a), sizeof(struct sockaddr_in));
    if (t == -1)
        DIE(TAG_SOCKET, "%s", strerror(errno));

    t = listen(fd, MAX_CONNECTIONS);
    if (t == -1)
        DIE(TAG_SOCKET, "%s", strerror(errno));        

    return fd;
}

