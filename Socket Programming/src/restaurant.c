#include "restaurant.h"
#include <stdio.h>

int restaurant_init(restaurant_info *restaurant, struct sockaddr_in *addr_out)
{
    char buffer[INP_BUFF_SIZE];
    write(STDIN_FILENO, SIGNIN_PROMPT, strlen(SIGNIN_PROMPT));
    int bytes_read = read(STDIN_FILENO, buffer, INP_BUFF_SIZE);
    buffer[bytes_read - 1] = '\0';
    restaurant->username = (char *)malloc(bytes_read);
    strcpy(restaurant->username, buffer);
    restaurant->tcp_port = find_tcp_port();
    restaurant->udp_fd = init_broadcast(IP_ADDR, restaurant->udp_port, addr_out);
    restaurant->tcp_fd_client = connect_server(restaurant->tcp_port);
    restaurant->tcp_fd_server = init_server(restaurant->tcp_port);
    username_check(restaurant->udp_port, restaurant->tcp_port, restaurant->tcp_fd_server, restaurant->username, Restaurant);
    char *msg = encode_msg(msg_struct_generator(restaurant->username, restaurant->tcp_port, Restaurant, NOTIFY_RESTAURANT_OPEN));
    broadcast_msg(restaurant->udp_port, msg);
    restaurant->status = Open;
    restaurant->foods.foods = get_foods(&restaurant->foods.food_count);
    restaurant->order_count = 0;
    restaurant->ings_count = 0;
    print_welcome(restaurant->username, Restaurant);
    generate_file(restaurant->username);
    log_msg(restaurant->username, custom_sprintf("%s %s", restaurant->username, WELLCOME_LOG));
}

void print_foods(foods_info foods)
{
    for (int i = 0; i < foods.food_count; i++)
    {
        char *result = custom_sprintf("%d- %s:\n", i + 1, foods.foods[i].name);
        write(1, result, strlen(result));
        for (int j = 0; j < foods.foods[i].ingredient_count; j++)
        {
            char *result1 = custom_sprintf("\t\t%s : %d\n", foods.foods[i].ingredients[j].name, foods.foods[i].ingredients[j].count);
            write(1, result1, strlen(result1));
        }
    }
}

int find_food_index(restaurant_info *restaurant, char *food_name)
{
    int found = -1;
    int food_index = 0;
    for (food_index; food_index < restaurant->foods.food_count; food_index++)
    {
        if (!strcmp(food_name, restaurant->foods.foods[food_index].name))
        {
            found = food_index;
            break;
        }
    }
    return found;
}

int find_order_index(restaurant_info *restaurant, unsigned short port)
{
    int found = -1;
    int order_index = 0;
    for (order_index; order_index < restaurant->order_count; order_index++)
    {
        if (port == restaurant->orders[order_index].sender_tcp_port)
        {
            found = order_index;
        }
    }
    return found;
}

void print_reqs(restaurant_info *restaurant)
{
    for (int i = 0; i < restaurant->order_count; i++)
    {
        if (check_order_timed_out(restaurant->orders[i].rcv_time, FOOD_TIMEOUT_DOUBLE) && restaurant->orders[i].status == Pending)
        {
            restaurant->orders[i].status = TimedOut;
        }
        if (restaurant->orders[i].status == Pending)
        {
            char *result = custom_sprintf("%s  %d  %s\n", restaurant->orders[i].username, restaurant->orders[i].sender_tcp_port, restaurant->orders[i].food_name);
            write(1, result, strlen(result));
        }
    }
}

void print_history(restaurant_info *restaurant)
{
    char *order_statuses[3] = {ACCEPTED_PROMPT, REJECTED_PROMPT, TIMEDOUT_PROMPT};
    for (int i = 0; i < restaurant->order_count; i++)
    {
        if (check_order_timed_out(restaurant->orders[i].rcv_time, FOOD_TIMEOUT_DOUBLE) && restaurant->orders[i].status == Pending)
        {
            restaurant->orders[i].status = TimedOut;
        }
        if (restaurant->orders[i].status != Pending)
        {
            char *result;
            result = custom_sprintf("%s %s %s\n", restaurant->orders[i].username, restaurant->orders[i].food_name, order_statuses[(int)restaurant->orders[i].status]);
            write(STDERR_FILENO, result, strlen(result));
        }
    }
}

int have_enough_ingredient(restaurant_info *restaurant, int food_index)
{
    if (restaurant->ings_count == 0)
    {
        return 0;
    }
    int have_enough_ingredients = 1;
    for (int i = 0; i < restaurant->foods.foods[food_index].ingredient_count; i++)
    {
        int have_enough_ingredient = 0;
        for (int j = 0; j < restaurant->ings_count; j++)
        {
            if (!strcmp(restaurant->foods.foods[food_index].ingredients[i].name, restaurant->ingredients[j].name))
            {
                if (restaurant->foods.foods[food_index].ingredients[i].count <= restaurant->ingredients[j].count)
                {
                    have_enough_ingredient = 1;
                    break;
                }
            }
        }
        if (!have_enough_ingredient)
        {
            have_enough_ingredients = 0;
            return have_enough_ingredients;
        }
    }
    return have_enough_ingredients;
}

void prepare_food(restaurant_info *restaurant, int food_index)
{
    for (int i = 0; i < restaurant->foods.foods[food_index].ingredient_count; i++)
    {
        for (int j = 0; j < restaurant->ings_count; j++)
        {
            if (!strcmp(restaurant->foods.foods[food_index].ingredients[i].name, restaurant->ingredients[j].name))
            {
                restaurant->ingredients[j].count -= restaurant->foods.foods[food_index].ingredients[i].count;
            }
        }
    }
}

int can_prepare_food(restaurant_info *restaurant, int order_index)
{

    int food_index = find_food_index(restaurant, restaurant->orders[order_index].food_name);
    if (!have_enough_ingredient(restaurant, food_index))
    {
        write(STDIN_FILENO, INSUFFICIENT_INGS_ERROR, strlen(INSUFFICIENT_INGS_ERROR));
        return 0;
    }

    prepare_food(restaurant, food_index);
    return 1;
}

int find_ingredient_index(restaurant_info *restaurant, char *ing_name)
{
    for (int ingredient_index = 0; ingredient_index < restaurant->ings_count; ingredient_index++)
    {
        if (!strcmp(ing_name, restaurant->ingredients[ingredient_index].name))
        {
            return ingredient_index;
        }
    }
    return -1;
}

void add_ingredient(restaurant_info *restaurant, char *ing_name, int count)
{
    int ingredient_index = find_ingredient_index(restaurant, ing_name);
    if (ingredient_index >= 0)
    {
        restaurant->ingredients[ingredient_index].count += count;
    }
    else
    {
        if (restaurant->ings_count == 0)
            restaurant->ingredients = (ingredient *)malloc((restaurant->ings_count + 1) * sizeof(ingredient));
        else
            restaurant->ingredients = (ingredient *)realloc(restaurant->ingredients, (restaurant->ings_count + 1) * sizeof(ingredient));

        restaurant->ingredients[restaurant->ings_count].name = custom_sprintf("%s", ing_name);

        restaurant->ingredients[restaurant->ings_count].count = count;
        restaurant->ings_count++;
    }
}

int send_ings_order(restaurant_info *restaurant, unsigned short port, char *ing_name, int count)
{

    int fd = connect_server(port);
    if (fd < 0)
    {
        write(STDIN_FILENO, INVALID_PORT_NUMBER_ERROR, strlen(INVALID_PORT_NUMBER_ERROR));
        return 0;
    }
    send_tcp_msg(fd, encode_msg(msg_struct_generator(restaurant->username, restaurant->tcp_port, Restaurant, custom_sprintf("%s %s %d", ORDER_INGS, ing_name, count))), MAX_TCP_RETRIES);
    log_msg(restaurant->username, custom_sprintf("%s %s %s to port %d", restaurant->username, ORDER_INGS, ing_name, port));
    return 1;
}

void wait_for_ings_order_answer(restaurant_info *restaurant, char *ing_name, int count)
{
    write(STDIN_FILENO, WAIT_FOR_INGS_PROMPT, strlen(WAIT_FOR_INGS_PROMPT));
    signal(SIGALRM, alarm_handler);
    siginterrupt(SIGALRM, 1);
    console_lock();
    alarm(SUPPLIER_TIMEOUT);
    int fd;

    fd = acc_client(restaurant->tcp_fd_server);
    if (fd >= 0)
    {
        char *msg = rcv_tcp_msg(fd);
        decoded_msg msg_struct = decode_msg(msg);
        // close(fd);
        if (!strcmp(msg_struct.detail, ACCEPT))
        {
            char *prompt = custom_sprintf("%s %s", msg_struct.sender_username, INGS_READY_PROMPT);
            write(STDIN_FILENO, prompt, strlen(prompt));
            log_msg(restaurant->username, custom_sprintf("%s %s on port %d", msg_struct.sender_username, ACCEPT_ING_LOG, msg_struct.sender_tcp_port));
            add_ingredient(restaurant, ing_name, count);
        }
        else if (!strcmp(msg_struct.detail, REJECT))
        {
            char *prompt = custom_sprintf("%s %s", msg_struct.sender_username, INGS_REJECT_PROMPT);
            log_msg(restaurant->username, custom_sprintf("%s %s on port %d", msg_struct.sender_username, REJECT_ING_LOG, msg_struct.sender_tcp_port));
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
        log_msg(restaurant->username, custom_sprintf("%s %s", restaurant->username, TIMEDOUT_ING_LOG));
        write(STDIN_FILENO, INGS_TIMEOUT_PROMPT, strlen(INGS_TIMEOUT_PROMPT));
    }
}

void show_ingredients(restaurant_info *restaurant)
{
    for (int i = 0; i < restaurant->ings_count; i++)
    {
        if (restaurant->ingredients[i].count != 0)
        {
            char *result = custom_sprintf("%s : %d\n", restaurant->ingredients[i].name, restaurant->ingredients[i].count);
            write(1, result, strlen(result));
        }
    }
}

void order_ingredient(restaurant_info *restaurant)
{
    write(STDIN_FILENO, GET_INGREDIENT_NAME_PROMPT, strlen(GET_INGREDIENT_NAME_PROMPT));
    char buffer[INP_BUFF_SIZE];
    int bytes_read = read(STDIN_FILENO, buffer, INP_BUFF_SIZE);
    buffer[bytes_read - 1] = '\0';

    write(STDIN_FILENO, GET_INGREDIENT_COUNT_PROMPT, strlen(GET_INGREDIENT_COUNT_PROMPT));
    char buffer1[INP_BUFF_SIZE];
    bytes_read = read(STDIN_FILENO, buffer1, INP_BUFF_SIZE);
    buffer1[bytes_read - 1] = '\0';

    write(STDIN_FILENO, GET_SUPPLIER_PORT_PROMPT, strlen(GET_SUPPLIER_PORT_PROMPT));
    char buffer2[INP_BUFF_SIZE];
    bytes_read = read(STDIN_FILENO, buffer2, INP_BUFF_SIZE);
    buffer2[bytes_read - 1] = '\0';
    if (send_ings_order(restaurant, atoi(buffer2), buffer, atoi(buffer1)))
    {
        wait_for_ings_order_answer(restaurant, buffer, atoi(buffer1));
    }
}

void answer_food_order(restaurant_info *restaurant)
{
    write(STDIN_FILENO, GET_USER_PORT_PROMPT, strlen(GET_USER_PORT_PROMPT));
    char buffer[INP_BUFF_SIZE];
    int bytes_read = read(STDIN_FILENO, buffer, INP_BUFF_SIZE);
    buffer[bytes_read - 1] = '\0';

    write(STDIN_FILENO, GET_ACCEPT_FOOD_PROMPT, strlen(GET_ACCEPT_FOOD_PROMPT));
    char buffer1[INP_BUFF_SIZE];
    bytes_read = read(STDIN_FILENO, buffer1, INP_BUFF_SIZE);
    buffer1[bytes_read - 1] = '\0';

    int order_index = find_order_index(restaurant, atoi(buffer));
    if (order_index == -1)
    {
        write(STDIN_FILENO, INVALID_PORT_NUMBER_ERROR, strlen(INVALID_PORT_NUMBER_ERROR));
        return;
    }
    if (restaurant->orders[order_index].status != Pending)
    {
        return;
    }

    if (check_order_timed_out(restaurant->orders[order_index].rcv_time, FOOD_TIMEOUT_DOUBLE) && restaurant->orders[order_index].status == Pending)
    {
        write(STDIN_FILENO, ORDER_TIMEOUT_ERROR, strlen(ORDER_TIMEOUT_ERROR));
        restaurant->orders[order_index].status = TimedOut;
        return;
    }

    int fd = connect_server(atoi(buffer));
    if (!strcmp(buffer1, "yes") && can_prepare_food(restaurant, order_index))
    {
        restaurant->orders[order_index].status = Accepted;
        send_tcp_msg(fd, encode_msg(msg_struct_generator(restaurant->username, restaurant->tcp_port, Restaurant, ACCEPT)), MAX_TCP_RETRIES);
        log_msg(restaurant->username, custom_sprintf("%s %s on port %d", restaurant->username, ACCEPT_FOOD_LOG, restaurant->orders[order_index].sender_tcp_port));
    }
    else
    {
        restaurant->orders[order_index].status = Rejected;
        send_tcp_msg(fd, encode_msg(msg_struct_generator(restaurant->username, restaurant->tcp_port, Restaurant, REJECT)), MAX_TCP_RETRIES);
        log_msg(restaurant->username, custom_sprintf("%s %s on port %d", restaurant->username, REJECT_FOOD_LOG, restaurant->orders[order_index].sender_tcp_port));
    }
}

void restaurant_input_handler(char *input, restaurant_info *restaurant)
{
    if (!strcmp(input, OPEN_RESTAURANT_COMMAND))
    {
        if (restaurant->status == Close)
        {
            restaurant->status = Open;
            char *msg = encode_msg(msg_struct_generator(restaurant->username, restaurant->tcp_port, Restaurant, NOTIFY_RESTAURANT_OPEN));
            broadcast_msg(restaurant->udp_port, msg);
            log_msg(restaurant->username, custom_sprintf("%s %s", restaurant->username, RESTAURANT_OPEN_LOG));
        }
    }

    if (!strcmp(input, SHOW_SUPPLIER_COMMAND))
    {

        char *msg = encode_msg(msg_struct_generator(restaurant->username, restaurant->tcp_port, Restaurant, GET_SUPPLIER_INFO));
        broadcast_msg(restaurant->udp_port, msg);
        log_msg(restaurant->username, custom_sprintf("%s %s", restaurant->username, GET_SUPPLIER_LOG));
    }

    else if (!strcmp(input, CLOSE_RESTAURANT_COMMAND))
    {
        if (restaurant->status == Open)
        {
            restaurant->status = Close;
            char *msg = encode_msg(msg_struct_generator(restaurant->username, restaurant->tcp_port, Restaurant, NOTIFY_RESTAURANT_CLOSE));
            broadcast_msg(restaurant->udp_port, msg);
            log_msg(restaurant->username, custom_sprintf("%s %s", restaurant->username, RESTAURANT_CLOSE_LOG));
        }
    }

    else if (!strcmp(input, SHOW_INGS_COMMAND))
    {
        if (restaurant->status == Open)
        {
            show_ingredients(restaurant);
        }
    }

    else if (!strcmp(input, ORDER_INGS_COMMAND))
    {
        order_ingredient(restaurant);
    }

    else if (!strcmp(input, SHOW_RECIPES_COMMAND))
    {
        if (restaurant->status == Open)
        {
            print_foods(restaurant->foods);
        }
    }

    else if (!strcmp(input, SHOW_REQS_LIST_COMMAND))
    {
        if (restaurant->status == Open)
        {
            print_reqs(restaurant);
        }
    }

    else if (!strcmp(input, SHOW_SALES_HISTORY_COMMAND))
    {
        if (restaurant->status == Open)
        {
            print_history(restaurant);
        }
    }

    else if (!strcmp(input, ANSWER_FOOD_REQ_COMMAND))
    {
        answer_food_order(restaurant);
    }

    else
    {
        write(STDIN_FILENO, INVALID_COMMAND_ERROR, strlen(INVALID_COMMAND_ERROR));
    }
}

void add_new_order(restaurant_info *restaurant, char *food_name, unsigned short sender_tcp_port, char *sender_username)
{

    if (restaurant->order_count == 0)
        restaurant->orders = (order *)malloc((restaurant->order_count + 1) * sizeof(order));
    else
        restaurant->orders = (order *)realloc(restaurant->orders, (restaurant->order_count + 1) * sizeof(order));
    time(&restaurant->orders[restaurant->order_count].rcv_time);
    restaurant->orders[restaurant->order_count].food_name = food_name;
    restaurant->orders[restaurant->order_count].sender_tcp_port = sender_tcp_port;
    restaurant->orders[restaurant->order_count].status = Pending;
    restaurant->orders[restaurant->order_count].username = sender_username;
    restaurant->order_count++;
}

void restaurant_tcp_msg_handler(char *msg, restaurant_info *restaurant)
{
    decoded_msg msg_struct = decode_msg(msg);
    parse_obj command = parse_line(msg_struct.detail, " ");

    if (!strcmp(msg_struct.detail, SHARE_SUPPLIER_INFO))
    {
        char *result = custom_sprintf("%s %d\n", msg_struct.sender_username, msg_struct.sender_tcp_port);
        write(STDIN_FILENO, result, strlen(result));
    }

    else if (!strcmp(command.argv[0], ORDER_FOOD))
    {
        if (restaurant->status == Close)
        {
            return;
        }
        if (find_food_index(restaurant, command.argv[1]) == -1)
        {
            write(STDIN_FILENO, INVALID_FOOD_ERROR, strlen(INVALID_FOOD_ERROR));
            send_tcp_msg(connect_server(msg_struct.sender_tcp_port), encode_msg(msg_struct_generator(restaurant->username, restaurant->tcp_port, Restaurant, REJECT)), MAX_TCP_RETRIES);
            return;
        }

        add_new_order(restaurant, command.argv[1], msg_struct.sender_tcp_port, msg_struct.sender_username);
        write(STDIN_FILENO, FOOD_REQUEST_PROMPT, strlen(FOOD_REQUEST_PROMPT));
        log_msg(restaurant->username, custom_sprintf("%s %s on port %d", restaurant->username, RCV_FOOD_LOG, msg_struct.sender_tcp_port));
    }
}

void dashboard(restaurant_info *restaurant)
{
    fd_set master_set, working_set;
    int max_sd = restaurant->tcp_fd_server;
    FD_ZERO(&master_set);
    FD_SET(STDIN_FILENO, &master_set);
    FD_SET(restaurant->udp_fd, &master_set);
    FD_SET(restaurant->tcp_fd_server, &master_set);
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
                if (i == restaurant->udp_fd)
                {
                    char *msg = rcv_broadcast(restaurant->udp_fd);
                    udp_handler(msg, restaurant->username, restaurant->tcp_port, Restaurant);
                }
                else if (i == restaurant->tcp_fd_server)
                {
                    int new_client_socket = acc_client(restaurant->tcp_fd_server);
                    FD_SET(new_client_socket, &master_set);
                    if (new_client_socket > max_sd)
                        max_sd = new_client_socket;
                }
                else if (i == STDIN_FILENO)
                {
                    int inp_size = read(STDIN_FILENO, buffer, INP_BUFF_SIZE);
                    buffer[inp_size - 1] = '\0';
                    restaurant_input_handler(buffer, restaurant);
                }
                else
                {
                    char *msg = rcv_tcp_msg(i);
                    restaurant_tcp_msg_handler(msg, restaurant);
                }
            }
        }
    }
}

int main(int argc, char const *argv[])
{
    restaurant_info restaurant;
    if (argc != 2)
    {
        perror(PORT_NOT_ENTERED_ERROR);
        exit(EXIT_SUCCESS);
    }

    restaurant.udp_port = atoi(argv[1]);
    struct sockaddr_in addr_out;
    restaurant_init(&restaurant, &addr_out);
    dashboard(&restaurant);

    return 0;
}
