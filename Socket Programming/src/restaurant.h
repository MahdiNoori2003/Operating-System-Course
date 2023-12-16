#ifndef __RESTAURANT_H__
#define __RESTAURANT_H__
#include "utils.h"
#include "network.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <time.h>

#define OPEN_RESTAURANT_COMMAND "start working"
#define CLOSE_RESTAURANT_COMMAND "break"
#define ANSWER_FOOD_REQ_COMMAND "answer request"
#define SHOW_SUPPLIER_COMMAND "show suppliers"
#define SHOW_RECIPES_COMMAND "show recipes"
#define SHOW_SALES_HISTORY_COMMAND "show sales history"
#define SHOW_REQS_LIST_COMMAND "show requests list"
#define ORDER_INGS_COMMAND "request ingredient"
#define SHOW_INGS_COMMAND "show ingredients"
#define FOOD_REQUEST_PROMPT "new order!\n"
#define GET_USER_PORT_PROMPT "port of request : "
#define GET_ACCEPT_FOOD_PROMPT "your answer (yes/no): "
#define GET_INGREDIENT_NAME_PROMPT "name of ingredient : "
#define GET_INGREDIENT_COUNT_PROMPT "number of ingredient : "
#define GET_SUPPLIER_PORT_PROMPT "port of supplier : "
#define WAIT_FOR_INGS_PROMPT "waiting for supplier's response... \n"
#define INGS_READY_PROMPT "Supplier accepted!\n"
#define INGS_REJECT_PROMPT "Supplier denied!\n"
#define INGS_TIMEOUT_PROMPT "Time out!\n"

#define ACCEPTED_PROMPT "accepted"
#define REJECTED_PROMPT "rejected"
#define TIMEDOUT_PROMPT "timed out"

typedef enum Status
{
    Open,
    Close
} Status;

typedef struct foods_info
{
    food *foods;
    int food_count;
} foods_info;

typedef struct order
{
    time_t rcv_time;
    char *username;
    unsigned short sender_tcp_port;
    char *food_name;
    order_status status;

} order;

typedef struct restaurant_info
{
    char *username;
    unsigned short int tcp_port;
    unsigned short int udp_port;
    int udp_fd;
    int tcp_fd_client;
    int tcp_fd_server;
    int order_count;
    order *orders;
    Status status;
    foods_info foods;
    int ings_count;
    ingredient *ingredients;

} restaurant_info;

int restaurant_init(restaurant_info *restaurant, struct sockaddr_in *addr_out);

void restaurant_input_handler(char *input, restaurant_info *restaurant);

void print_foods(foods_info foods);

void restaurant_tcp_msg_handler(char *msg, restaurant_info *restaurant);

void add_new_order(restaurant_info *restaurant, char *food_name, unsigned short sender_tcp_port, char *sender_username);

int find_food_index(restaurant_info *restaurant, char *food_name);

int find_order_index(restaurant_info *restaurant, unsigned short port);

int can_prepare_food(restaurant_info *restaurant, int order_index);

int have_enough_ingredient(restaurant_info *restaurant, int food_index);

void prepare_food(restaurant_info *restaurant, int food_index);

void print_reqs(restaurant_info *restaurant);

void print_history(restaurant_info *restaurant);

void wait_for_ings_order_answer(restaurant_info *restaurant, char *ing_name, int count);

int send_ings_order(restaurant_info *restaurant, unsigned short port, char *ing_name, int count);

void add_ingredient(restaurant_info *restaurant, char *ing_name, int count);

int find_ingredient_index(restaurant_info *restaurant, char *ing_name);

void order_ingredient(restaurant_info *restaurant);

void answer_food_order(restaurant_info *restaurant);

void dashboard();

#endif