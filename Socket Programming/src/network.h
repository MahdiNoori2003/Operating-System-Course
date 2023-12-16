#ifndef NETWORK_H_INCLUDE
#define NETWORK_H_INCLUDE

#include <netinet/in.h>
#include "define.h"

#define SOCKET_CREATION_ERROR "Socket creation failed"
#define SOCKET_BIND_ERROR "Socket binding failed"
#define GET_SOCKET_NAME_ERROR "getsockname failed"

#define MAX_TCP_RETRIES 10

int init_broadcast(const char *ip_addr, unsigned short port, struct sockaddr_in *addr_out);
int init_server(unsigned short port);
int acc_client(int socket_id);
int connect_server(unsigned short port);
void broadcast_msg(unsigned short port, const char *msg);
char *rcv_broadcast(int sock_fd);
int send_tcp_msg(int sock_fd, const char *msg, int max_tries);
char *rcv_tcp_msg(int sock_fd);

#endif
