#ifndef __CUSTOMER_H__
#define __CUSTOMER_H__
#include "network.h"
#include "utils.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#define SHOW_RESTAURANT_COMMAND "show restaurants"
#define SHOW_MENU_COMMAND "show menu"
#define ORDER_FOOD_COMMAND "order food"
#define GET_RESTAURANT_PORT_PROMPT "port of restaurant : "
#define GET_FOOD_NAME_PROMPT "name of food : "
#define WAIT_FOR_FOOD_PROMPT "waiting for the restaurant's response ..\n"
#define FOOD_READY_PROMPT "Restaurant accepted and your food is ready!\n"
#define FOOD_REJECT_PROMPT "Restaurant denied and cry about it!\n"
#define FOOD_TIMEOUT_PROMPT "Time out!\n"

typedef struct customer_info
{
    char *username;
    unsigned short int tcp_port;
    unsigned short int udp_port;
    int udp_fd;
    int tcp_fd_client;
    int tcp_fd_server;

} customer_info;

int customer_init(customer_info *customer, struct sockaddr_in *addr_out);

void customer_input_handler(char *input, customer_info *customer);

void customer_tcp_msg_handler(char *msg, customer_info *customer);

int send_food_order(customer_info *customer);

void wait_for_food_order_answer(customer_info *customer);

void dashboard(customer_info *customer);

#endif