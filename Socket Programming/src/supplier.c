#include "supplier.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/timeb.h>
#include <fcntl.h>
#include <unistd.h>

int supplier_init(supplier_info *supplier, struct sockaddr_in *addr_out)
{
    char buffer[INP_BUFF_SIZE];
    write(STDIN_FILENO, SIGNIN_PROMPT, strlen(SIGNIN_PROMPT));
    int bytes_read = read(STDIN_FILENO, buffer, INP_BUFF_SIZE);
    buffer[bytes_read - 1] = '\0';
    supplier->username = (char *)malloc(bytes_read);
    strcpy(supplier->username, buffer);
    supplier->tcp_port = find_tcp_port();
    supplier->udp_fd = init_broadcast(IP_ADDR, supplier->udp_port, addr_out);
    supplier->tcp_fd_client = connect_server(supplier->tcp_port);
    supplier->tcp_fd_server = init_server(supplier->tcp_port);
    supplier->order_count = 0;
    supplier->ings = get_ingridients();
    username_check(supplier->udp_port, supplier->tcp_port, supplier->tcp_fd_server, supplier->username, Supplier);
    print_welcome(supplier->username, Supplier);
    generate_file(supplier->username);
    log_msg(supplier->username, custom_sprintf("%s %s", supplier->username, WELLCOME_LOG));
}

int find_ing_index(supplier_info *supplier, char *ing_name)
{
    int found = -1;
    int ing_index = 0;
    while (supplier->ings[ing_index] != NULL)
    {

        if (!strcmp(ing_name, supplier->ings[ing_index]))
        {
            found = ing_index;
            break;
        }

        ing_index++;
    }
    return found;
}

void add_new_order(supplier_info *supplier, char *ing_name, unsigned short sender_tcp_port, int ing_count)
{
    time(&supplier->order.rcv_time);
    supplier->order.ing_name = ing_name;
    supplier->order.sender_tcp_port = sender_tcp_port;
    supplier->order.status = Pending;
    if (supplier->order_count == 0)
    {
        supplier->order_count = 1;
    }
}

void supplier_tcp_msg_handler(char *msg, supplier_info *supplier)
{
    decoded_msg msg_struct = decode_msg(msg);
    parse_obj command = parse_line(msg_struct.detail, " ");

    if (!strcmp(command.argv[0], ORDER_INGS))
    {
        if (find_ing_index(supplier, command.argv[1]) == -1)
        {

            write(STDIN_FILENO, INVALID_ING_ERROR, strlen(INVALID_ING_ERROR));
            send_tcp_msg(connect_server(msg_struct.sender_tcp_port), encode_msg(msg_struct_generator(supplier->username, supplier->tcp_port, Supplier, REJECT)), MAX_TCP_RETRIES);
            log_msg(supplier->username, custom_sprintf("%s %s on port %d", supplier->username, REJECT_ING_LOG, msg_struct.sender_tcp_port));
            return;
        }

        if (supplier->order_count != 0)
        {

            if (!check_order_timed_out(supplier->order.rcv_time, SUPPLIER_TIMEOUT_DOUBLE) &&
                supplier->order.status == Pending)
            {

                send_tcp_msg(connect_server(msg_struct.sender_tcp_port), encode_msg(msg_struct_generator(supplier->username, supplier->tcp_port, Supplier, REJECT)), MAX_TCP_RETRIES);
                log_msg(supplier->username, custom_sprintf("%s %s on port %d", supplier->username, REJECT_ING_LOG, msg_struct.sender_tcp_port));
                return;
            }
        }
        add_new_order(supplier, command.argv[1], msg_struct.sender_tcp_port, atoi(command.argv[2]));
        write(STDIN_FILENO, ING_REQUEST_PROMPT, strlen(ING_REQUEST_PROMPT));
        log_msg(supplier->username, custom_sprintf("%s %s on port %d", supplier->username, RCV_ING_LOG, supplier->order.sender_tcp_port));
    }
}

void supplier_input_handler(char *input, supplier_info *supplier)
{
    if (!strcmp(input, ANSWER_ING_REQ_COMMAND))
    {
        if (supplier->order_count == 0)
        {
            return;
        }

        if (check_order_timed_out(supplier->order.rcv_time, SUPPLIER_TIMEOUT_DOUBLE))
        {
            write(STDIN_FILENO, ORDER_TIMEOUT_ERROR, strlen(ORDER_TIMEOUT_ERROR));
            supplier->order.status = TimedOut;
            return;
        }

        if (supplier->order.status != Pending)
        {
            return;
        }

        write(STDIN_FILENO, GET_ACCEPT_ING_PROMPT, strlen(GET_ACCEPT_ING_PROMPT));
        char buffer[INP_BUFF_SIZE];
        int bytes_read = read(STDIN_FILENO, buffer, INP_BUFF_SIZE);
        buffer[bytes_read - 1] = '\0';

        int fd = connect_server(supplier->order.sender_tcp_port);

        if (!strcmp(buffer, "yes"))
        {
            supplier->order.status = Accepted;
            send_tcp_msg(fd, encode_msg(msg_struct_generator(supplier->username, supplier->tcp_port, Supplier, ACCEPT)), MAX_TCP_RETRIES);
            log_msg(supplier->username, custom_sprintf("%s %s on port %d", supplier->username, ACCEPT_ING_LOG, supplier->order.sender_tcp_port));
        }
        else
        {
            supplier->order.status = Rejected;
            send_tcp_msg(fd, encode_msg(msg_struct_generator(supplier->username, supplier->tcp_port, Supplier, REJECT)), MAX_TCP_RETRIES);
            log_msg(supplier->username, custom_sprintf("%s %s on port %d", supplier->username, REJECT_ING_LOG, supplier->order.sender_tcp_port));
        }
    }
}

void dashboard(supplier_info *supplier)
{
    fd_set master_set, working_set;
    int max_sd = supplier->tcp_fd_server;
    FD_ZERO(&master_set);
    FD_SET(STDIN_FILENO, &master_set);
    FD_SET(supplier->udp_fd, &master_set);
    FD_SET(supplier->tcp_fd_server, &master_set);
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
                if (i == supplier->udp_fd)
                {
                    char *msg = rcv_broadcast(supplier->udp_fd);
                    udp_handler(msg, supplier->username, supplier->tcp_port, Supplier);
                }
                else if (i == supplier->tcp_fd_server)
                {
                    int new_client_socket = acc_client(supplier->tcp_fd_server);
                    FD_SET(new_client_socket, &master_set);
                    if (new_client_socket > max_sd)
                        max_sd = new_client_socket;
                }
                else if (i == STDIN_FILENO)
                {
                    int inp_size = read(STDIN_FILENO, buffer, INP_BUFF_SIZE);
                    buffer[inp_size - 1] = '\0';
                    supplier_input_handler(buffer, supplier);
                }
                else
                {
                    char *msg = rcv_tcp_msg(i);
                    supplier_tcp_msg_handler(msg, supplier);
                }
            }
        }
    }
}

int main(int argc, char const *argv[])
{
    supplier_info supplier;
    if (argc != 2)
    {
        perror(PORT_NOT_ENTERED_ERROR);
        exit(EXIT_SUCCESS);
    }
    supplier.udp_port = atoi(argv[1]);
    struct sockaddr_in addr_out;
    supplier_init(&supplier, &addr_out);
    dashboard(&supplier);
}
