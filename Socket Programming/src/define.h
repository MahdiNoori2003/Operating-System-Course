#ifndef __DEFINE_H__
#define __DEFINE_H__

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "json_utils.h"

// ports consts
#define INP_BUFF_SIZE 1024
#define IP_ADDR "127.255.255.255"
#define FOOD_TIMEOUT 120
#define FOOD_TIMEOUT_DOUBLE 120.0
#define SUPPLIER_TIMEOUT 90
#define SUPPLIER_TIMEOUT_DOUBLE 90.0

// prompts consts
#define SIGNIN_PROMPT "please enter your username : "
#define RESTAURANT_OPEN_PROMPT " restaurant opened!\n"
#define RESTAURANT_CLOSE_PROMPT " restaurant closed!\n"

// errors
#define PORT_NOT_ENTERED_ERROR "please enter your port\n"
#define INVALID_USERNAME_ERROR "invalid username\n"
#define INVALID_COMMAND_ERROR "invalid command\n"
#define INVALID_PORT_NUMBER_ERROR "invalid port\n"
#define ORDER_TIMEOUT_ERROR "order timed out\n"
#define INSUFFICIENT_INGS_ERROR "insufficient ingredients\n"
#define UNABLE_TO_SEND_TCP_ERROR "unable to send tcp message\n"
#define UNABLE_TO_SEND_UDP_ERROR "unable to send udp message\n"
#define UNABLE_TO_CONNECT "unable to connect\n"
#define INVALID_FOOD_ERROR "invalid food name\n"
#define INVALID_ING_ERROR "invalid ingredient name\n"

// UDP commands
#define CHECK_USERNAME_COMMAND "check_username"
#define NOTIFY_RESTAURANT_OPEN "opened_restaurant"
#define NOTIFY_RESTAURANT_CLOSE "closed_restaurant"
#define GET_RESTAURANT_INFO "restaurant_info"
#define GET_SUPPLIER_INFO "supplier_info"

// TCP commands
#define SHARE_RESTAURANT_INFO "restaurant_info"
#define SHARE_SUPPLIER_INFO "supplier_info"
#define ORDER_FOOD "order_food"
#define ORDER_INGS "order_ings"
#define ACCEPT "accept"
#define REJECT "reject"

typedef enum sender_type
{
    Customer,
    Restaurant,
    Supplier
} sender_type;

typedef enum order_status
{
    Accepted,
    Rejected,
    TimedOut,
    Pending

} order_status;

typedef struct parse_obj
{
    int count;
    char **argv;
} parse_obj;

typedef struct decoded_msg
{
    char *sender_username;
    unsigned short sender_tcp_port;
    sender_type type;
    char *detail;
} decoded_msg;

#endif
