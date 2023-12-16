#include "network.h"
#include "utils.h"

int find_tcp_port()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror(SOCKET_CREATION_ERROR);
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror(SOCKET_BIND_ERROR);
        close(sockfd);
        return -1;
    }

    socklen_t addrLen = sizeof(addr);
    if (getsockname(sockfd, (struct sockaddr *)&addr, &addrLen) < 0)
    {
        perror(GET_SOCKET_NAME_ERROR);
        close(sockfd);
        return -1;
    }

    int port = ntohs(addr.sin_port);
    close(sockfd);

    return port;
}

void alarm_handler(int sig)
{
    return;
}

void username_check(unsigned short broadcast_port, unsigned short tcp_port, int tcp_fd_server, char *username, sender_type type)
{

    char *msg = encode_msg(msg_struct_generator(username, tcp_port, type, CHECK_USERNAME_COMMAND));

    broadcast_msg(broadcast_port, msg);

    signal(SIGALRM, alarm_handler);
    siginterrupt(SIGALRM, 1);

    alarm(2);
    int fd = acc_client(tcp_fd_server);
    alarm(0);
    if (fd >= 0)
    {
        write(STDERR_FILENO, INVALID_USERNAME_ERROR, strlen(INVALID_USERNAME_ERROR));
        exit(EXIT_SUCCESS);
    }
}

char *read_line(int fd, int limit)
{
    char *result = NULL;
    int curr_size = 0;
    for (; limit != 0; --limit)
    {
        ++curr_size;
        result = (char *)realloc(result, curr_size);
        read(fd, &result[curr_size - 1], 1);
        if (result[curr_size - 1] == '\n')
        {
            result[curr_size - 1] = '\0';
            break;
        }
    }
    return result;
}

parse_obj parse_line(char *input_line, const char *delims)
{
    parse_obj result = {0, NULL};
    int input_line_len = strlen(input_line);
    int last_tok_end = 0;
    for (int i = 0; i <= input_line_len; ++i)
    {
        if (strchr(delims, input_line[i]) != NULL)
        {
            result.argv = (char **)realloc(result.argv, result.count + 1);
            result.argv[result.count] = (char *)calloc(i - last_tok_end + 1, sizeof(char));
            memcpy(result.argv[result.count], &input_line[last_tok_end], i - last_tok_end);
            result.argv[result.count][i - last_tok_end] = '\0';
            ++result.count;
            last_tok_end = i + 1;
        }
    }
    return result;
}

char *encode_msg(decoded_msg msg_struct)
{
    char *result = custom_sprintf("%s-%d-%d-%s", msg_struct.sender_username, msg_struct.sender_tcp_port, msg_struct.type,
                                  msg_struct.detail);
    return result;
}

decoded_msg decode_msg(char *msg)
{
    char **argv = parse_line(msg, "-").argv;
    return msg_struct_generator(argv[0], atoi(argv[1]), (sender_type)atoi(argv[2]), argv[3]);
}

decoded_msg msg_struct_generator(char *username, unsigned short tcp_port, sender_type type, char *detail)
{
    decoded_msg msg_struct;
    msg_struct.sender_username = username;
    msg_struct.sender_tcp_port = tcp_port;
    msg_struct.detail = detail;
    msg_struct.type = type;
    return msg_struct;
}

void udp_handler(char *msg, char *username, unsigned short tcp_port, sender_type reciver_type)
{
    decoded_msg msg_struct = decode_msg(msg);
    if (msg_struct.sender_tcp_port == tcp_port)
    {
        return;
    }
    if (!strcmp(msg_struct.detail, CHECK_USERNAME_COMMAND))
    {
        if (!strcmp(username, msg_struct.sender_username))
        {
            connect_server(msg_struct.sender_tcp_port);
        }
    }
    else if (!strcmp(msg_struct.detail, NOTIFY_RESTAURANT_OPEN))
    {
        if (reciver_type == Customer)
        {
            char *notif = strcat(msg_struct.sender_username, RESTAURANT_OPEN_PROMPT);
            write(STDIN_FILENO, notif, strlen(notif));
        }
    }
    else if (!strcmp(msg_struct.detail, NOTIFY_RESTAURANT_CLOSE))
    {
        if (reciver_type == Customer)
        {
            char *notif = strcat(msg_struct.sender_username, RESTAURANT_CLOSE_PROMPT);
            write(STDIN_FILENO, notif, strlen(notif));
        }
    }
    else if (!strcmp(msg_struct.detail, GET_RESTAURANT_INFO))
    {
        if (reciver_type == Restaurant)
        {
            char *msg = encode_msg(msg_struct_generator(username, tcp_port, reciver_type, SHARE_RESTAURANT_INFO));
            if (send_tcp_msg(connect_server(msg_struct.sender_tcp_port), msg, MAX_TCP_RETRIES) != 0)
            {
                perror(UNABLE_TO_SEND_TCP_ERROR);
            }
        }
    }
    else if (!strcmp(msg_struct.detail, GET_SUPPLIER_INFO))
    {
        if (reciver_type == Supplier)
        {
            char *msg = encode_msg(msg_struct_generator(username, tcp_port, reciver_type, SHARE_SUPPLIER_INFO));
            if (send_tcp_msg(connect_server(msg_struct.sender_tcp_port), msg, MAX_TCP_RETRIES) != 0)
            {
                perror(UNABLE_TO_SEND_TCP_ERROR);
            }
        }
    }
}

void print_welcome(char *username, sender_type type)
{
    char *types[3] = {"customer", "restaurant", "supplier"};
    char *welcome_msg = custom_sprintf("wellcome %s as %s!\n", username, types[(int)type]);
    write(STDIN_FILENO, welcome_msg, strlen(welcome_msg));
}

char *custom_sprintf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    if (size < 0)
    {
        va_end(args);
        return NULL;
    }
    char *result = (char *)malloc(size + 1);
    if (result == NULL)
    {
        va_end(args);
        return NULL;
    }
    vsprintf(result, format, args);

    va_end(args);

    return result;
}

void console_lock()
{
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void console_unlock()
{
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int check_order_timed_out(time_t order_time, double offset)
{
    time_t now;
    time(&now);
    if (difftime(now, order_time) > offset)
    {
        return 1;
    }
    return 0;
}
