#ifndef __SUPPLIER_H__
#define __SUPPLIER_H__
#include "network.h"
#include "utils.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#define ING_REQUEST_PROMPT "new request ingredient!\n"
#define ANSWER_ING_REQ_COMMAND "answer request"
#define GET_ACCEPT_ING_PROMPT "your answer (yes/no): "

typedef struct ingredient_order
{
    time_t rcv_time;
    unsigned short sender_tcp_port;
    char *ing_name;
    order_status status;
} ingredient_order;

typedef struct supplier_info
{
    char *username;
    unsigned short int tcp_port;
    unsigned short int udp_port;
    int udp_fd;
    int tcp_fd_client;
    int tcp_fd_server;
    ingredient_order order;
    int order_count;
    char **ings;

} supplier_info;

int supplier_init(supplier_info *supplier, struct sockaddr_in *addr_out);

void supplier_input_handler(char *input, supplier_info *supplier);

void supplier_tcp_msg_handler(char *msg, supplier_info *supplier);

int find_ing_index(supplier_info *supplier, char *ing_name);

void add_new_order(supplier_info *supplier, char *ing_name, unsigned short sender_tcp_port, int ing_count);

void dashboard(supplier_info *supplier);

#endif