#include "customer.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/timeb.h>
#include <fcntl.h>
#include <unistd.h>

int customer_init(customer_info *customer, struct sockaddr_in *addr_out)
{
    char buffer[INP_BUFF_SIZE];
    write(STDIN_FILENO, SIGNIN_PROMPT, strlen(SIGNIN_PROMPT));
    int bytes_read = read(STDIN_FILENO, buffer, INP_BUFF_SIZE);
    buffer[bytes_read - 1] = '\0';
    customer->username = (char *)malloc(bytes_read);
    strcpy(customer->username, buffer);
    customer->tcp_port = find_tcp_port();
    customer->udp_fd = init_broadcast(IP_ADDR, customer->udp_port, addr_out);
    customer->tcp_fd_client = connect_server(customer->tcp_port);
    customer->tcp_fd_server = init_server(customer->tcp_port);
    username_check(customer->udp_port, customer->tcp_port, customer->tcp_fd_server, customer->username, Customer);
    print_welcome(customer->username, Customer);
    generate_file(customer->username);
    log_msg(customer->username, custom_sprintf("%s %s", customer->username, WELLCOME_LOG));
}

void print_menu(char **menu)
{
    int i = 0;
    while (menu[i] != NULL)
    {
        char *result = custom_sprintf("%d- %s\n", i + 1, menu[i]);
        write(STDIN_FILENO, result, strlen(result));
        i++;
    }
}

int send_food_order(customer_info *customer)
{
    write(STDIN_FILENO, GET_FOOD_NAME_PROMPT, strlen(GET_FOOD_NAME_PROMPT));
    char buffer[INP_BUFF_SIZE];
    int bytes_read = read(STDIN_FILENO, buffer, INP_BUFF_SIZE);
    buffer[bytes_read - 1] = '\0';

    write(STDIN_FILENO, GET_RESTAURANT_PORT_PROMPT, strlen(GET_RESTAURANT_PORT_PROMPT));
    char buffer1[INP_BUFF_SIZE];
    bytes_read = read(STDIN_FILENO, buffer1, INP_BUFF_SIZE);
    buffer1[bytes_read - 1] = '\0';
    int fd = connect_server(atoi(buffer1));
    if (fd < 0)
    {
        write(STDIN_FILENO, INVALID_PORT_NUMBER_ERROR, strlen(INVALID_PORT_NUMBER_ERROR));
        return 0;
    }
    send_tcp_msg(fd, encode_msg(msg_struct_generator(customer->username, customer->tcp_port, Customer, custom_sprintf("%s %s", ORDER_FOOD, buffer))), MAX_TCP_RETRIES);
    log_msg(customer->username, custom_sprintf("%s %s %s to port %d", customer->username, ORDER_FOOD, buffer, atoi(buffer1)));
    return 1;
}

void wait_for_food_order_answer(customer_info *customer)
{
    write(STDIN_FILENO, WAIT_FOR_FOOD_PROMPT, strlen(WAIT_FOR_FOOD_PROMPT));
    signal(SIGALRM, alarm_handler);
    siginterrupt(SIGALRM, 1);
    console_lock();
    alarm(FOOD_TIMEOUT);
    int fd;

    fd = acc_client(customer->tcp_fd_server);
    if (fd >= 0)
    {
        char *msg = rcv_tcp_msg(fd);
        decoded_msg msg_struct = decode_msg(msg);
        // close(fd);
        if (!strcmp(msg_struct.detail, ACCEPT))
        {
            char *prompt = custom_sprintf("%s %s", msg_struct.sender_username, FOOD_READY_PROMPT);
            log_msg(customer->username, custom_sprintf("%s %s on port %d", msg_struct.sender_username, ACCEPT_FOOD_LOG, msg_struct.sender_tcp_port));
            write(STDIN_FILENO, prompt, strlen(prompt));
        }
        else if (!strcmp(msg_struct.detail, REJECT))
        {
            char *prompt = custom_sprintf("%s %s", msg_struct.sender_username, FOOD_REJECT_PROMPT);
            log_msg(customer->username, custom_sprintf("%s %s on port %d", msg_struct.sender_username, REJECT_FOOD_LOG, msg_struct.sender_tcp_port));
            write(STDIN_FILENO, prompt, strlen(prompt));
        }
        else
        {
            fd = -1;
        }
    }
    alarm(0);
    console_unlock();
    if (fd < 0)
    {
        log_msg(customer->username, custom_sprintf("%s %s", customer->username, TIMEDOUT_FOOD_LOG));
        write(STDIN_FILENO, FOOD_TIMEOUT_PROMPT, strlen(FOOD_TIMEOUT_PROMPT));
    }
}

void customer_input_handler(char *input, customer_info *customer)
{
    if (!strcmp(input, SHOW_RESTAURANT_COMMAND))
    {
        char *msg = encode_msg(msg_struct_generator(customer->username, customer->tcp_port, Customer, GET_RESTAURANT_INFO));
        broadcast_msg(customer->udp_port, msg);
        log_msg(customer->username, custom_sprintf("%s %s", customer->username, GET_RESTAURANT_LOG));
    }
    else if (!strcmp(input, SHOW_MENU_COMMAND))
    {
        char **menu = get_food_names();
        print_menu(menu);
    }

    else if (!strcmp(input, ORDER_FOOD_COMMAND))
    {
        if (send_food_order(customer))
        {
            wait_for_food_order_answer(customer);
        }
    }

    else
    {
        write(STDIN_FILENO, INVALID_COMMAND_ERROR, strlen(INVALID_COMMAND_ERROR));
    }
}

void customer_tcp_msg_handler(char *msg, customer_info *customer)
{
    decoded_msg msg_struct = decode_msg(msg);
    if (!strcmp(msg_struct.detail, SHARE_RESTAURANT_INFO))
    {
        char *result = custom_sprintf("%s %d\n", msg_struct.sender_username, msg_struct.sender_tcp_port);
        write(STDIN_FILENO, result, strlen(result));
    }
}

void dashboard(customer_info *customer)
{
    fd_set master_set, working_set;
    int max_sd = customer->tcp_fd_server;
    FD_ZERO(&master_set);
    FD_SET(STDIN_FILENO, &master_set);
    FD_SET(customer->udp_fd, &master_set);
    FD_SET(customer->tcp_fd_server, &master_set);
    char buffer[INP_BUFF_SIZE] = {0};

    while (1)
    {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_sd; i++)
        {
            if (FD_ISSET(i, &working_set))
            {
                memset(buffer, 0, INP_BUFF_SIZE);
                if (i == customer->udp_fd)
                {
                    char *msg = rcv_broadcast(customer->udp_fd);
                    udp_handler(msg, customer->username, customer->tcp_port, Customer);
                }
                else if (i == customer->tcp_fd_server)
                {
                    int new_client_socket = acc_client(customer->tcp_fd_server);
                    FD_SET(new_client_socket, &master_set);
                    if (new_client_socket > max_sd)
                        max_sd = new_client_socket;
                }
                else if (i == STDIN_FILENO)
                {
                    int inp_size = read(STDIN_FILENO, buffer, INP_BUFF_SIZE);
                    buffer[inp_size - 1] = '\0';
                    customer_input_handler(buffer, customer);
                }
                else
                {
                    char *msg = rcv_tcp_msg(i);
                    customer_tcp_msg_handler(msg, customer);
                }
            }
        }
    }
}

int main(int argc, char const *argv[])
{
    customer_info customer;
    if (argc != 2)
    {
        perror(PORT_NOT_ENTERED_ERROR);
        exit(EXIT_SUCCESS);
    }
    customer.udp_port = atoi(argv[1]);
    struct sockaddr_in addr_out;
    customer_init(&customer, &addr_out);
    dashboard(&customer);
}
