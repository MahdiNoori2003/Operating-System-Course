#include "network.h"

#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <asm-generic/socket.h>

int init_broadcast(const char *ip_addr, unsigned short port, struct sockaddr_in *addr_out)
{
    int socket_id = socket(PF_INET, SOCK_DGRAM, 0);
    if (socket_id < 0)
    {
        perror(SOCKET_CREATION_ERROR);
        return socket_id;
    }

    int broadcast = 1;
    int reuseport = 1;
    setsockopt(socket_id, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(socket_id, SOL_SOCKET, SO_REUSEPORT, &reuseport, sizeof(reuseport));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip_addr, &(addr.sin_addr.s_addr));
    memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

    if (bind(socket_id, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror(SOCKET_BIND_ERROR);
        return socket_id;
    }
    *addr_out = addr;
    return socket_id;
}

int init_server(unsigned short port)
{
    int socket_id = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_id < 0)
    {
        perror(SOCKET_CREATION_ERROR);
        return socket_id;
    }

    int reuseaddr = 1;
    setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

    if (bind(socket_id, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror(SOCKET_BIND_ERROR);
        return socket_id;
    }

    listen(socket_id, 100); // can be changed
    return socket_id;
}

int acc_client(int socket_id)
{
    struct sockaddr_in client_addr;
    int addr_size = sizeof(client_addr);
    int client_id = accept(socket_id, (struct sockaddr *)&client_addr, (socklen_t *)&addr_size);
    return client_id;
}

int connect_server(unsigned short port)
{
    int server_id = socket(PF_INET, SOCK_STREAM, 0);
    if (server_id < 0)
    {
        perror(SOCKET_CREATION_ERROR);
        return server_id;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &(addr.sin_addr.s_addr));
    memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

    if (connect(server_id, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        // perror(UNABLE_TO_CONNECT);
        // return server_id;
    }
    return server_id;
}

void broadcast_msg(unsigned short port, const char *msg)
{
    struct sockaddr_in bc_address;
    int broadcast = 1, reuseport = 1;

    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &reuseport, sizeof(reuseport));
    bc_address.sin_family = AF_INET;
    bc_address.sin_port = htons(port);
    bc_address.sin_addr.s_addr = inet_addr(IP_ADDR);

    bind(sock_fd, (struct sockaddr *)&bc_address, sizeof(bc_address));

    if (sendto(sock_fd, msg, strlen(msg), 0, (struct sockaddr *)&bc_address, sizeof(bc_address)) < 0)
    {
        perror(UNABLE_TO_SEND_UDP_ERROR);
        return;
    }
    close(sock_fd);
}

char *rcv_broadcast(int sock_fd)
{
    char buf[INP_BUFF_SIZE] = {0};

    char *result = NULL;
    int total_bytes_rcvd = 0;

    for (;;)
    {
        int recv_bytes = recv(sock_fd, buf, INP_BUFF_SIZE, 0);
        if (recv_bytes <= 0)
            return result;
        result = (char *)realloc(result, recv_bytes);
        memcpy(result, buf, recv_bytes);
        total_bytes_rcvd += recv_bytes;
        if (recv_bytes == INP_BUFF_SIZE)
        {
            fd_set read_fd_set;
            FD_ZERO(&read_fd_set);
            FD_SET(sock_fd, &read_fd_set);
            struct timeval t = {0, 0};
            select(sock_fd, &read_fd_set, NULL, NULL, &t);
            if (FD_ISSET(sock_fd, &read_fd_set))
                continue;
        }
        break;
    }
    result = (char *)realloc(result, total_bytes_rcvd + 1);
    result[total_bytes_rcvd] = '\0';

    return result;
}

int send_tcp_msg(int sock_fd, const char *msg, int max_tries)
{
    int left_bytes = strlen(msg);
    int tries = 0;
    for (; tries <= max_tries && left_bytes > 0; ++tries)
    {
        int sent_bytes = send(sock_fd, msg, left_bytes, 0);
        if (sent_bytes == -1)
        {
            perror(UNABLE_TO_SEND_TCP_ERROR);
            return -1;
        }
        left_bytes -= sent_bytes;
    }
    if (left_bytes > 0)
        perror(UNABLE_TO_SEND_TCP_ERROR);

    return left_bytes;
}

char *rcv_tcp_msg(int sock_fd)
{
    int num_bytes = 0, cur_size = 0, total_size = 0;
    char buf[1024] = {0};
    char *result = NULL;
    for (;;)
    {
        num_bytes = recv(sock_fd, buf, 1024, 0);
        if (num_bytes <= 0)
            return result;
        total_size += num_bytes;
        result = (char *)realloc(result, cur_size + num_bytes);
        memcpy(&result[cur_size], buf, num_bytes);
        cur_size += num_bytes;
        if (num_bytes < 1024)
            break;
        num_bytes = 0;
    }
    result = (char *)realloc(result, total_size + 1);
    result[total_size] = '\0';
    return result;
}
