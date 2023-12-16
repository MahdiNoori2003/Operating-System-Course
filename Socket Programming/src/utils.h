#ifndef __UTILS_H__
#define __UTILS_H__
#include "network.h"
#include "log.h"
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

int find_tcp_port();

void alarm_handler(int sig);

void username_check(unsigned short broadcast_port, unsigned short tcp_port, int tcp_fd_server, char *username, sender_type type);

char *read_line(int fd, int limit);

parse_obj parse_line(char *input_line, const char *delims);

char *encode_msg(decoded_msg msg_struct);

decoded_msg decode_msg(char *msg);

decoded_msg msg_struct_generator(char *username, unsigned short tcp_port, sender_type type, char *detail);

void udp_handler(char *msg, char *username, unsigned short tcp_port, sender_type reciver_type);

void print_welcome(char *username, sender_type type);

char *custom_sprintf(const char *format, ...);

void console_lock();

void console_unlock();

int check_order_timed_out(time_t order_time, double offset);

#endif