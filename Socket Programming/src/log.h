#ifndef LOGER_H
#define LOGER_H

#include "define.h"
#include "utils.h"

#define RD_WR 0666
#define WELLCOME_LOG "entered!"
#define GET_RESTAURANT_LOG "trying to get restaurant names"
#define ORDER_FOOD_LOG "ordered"
#define ORDER_INGS_LOG "ordered"
#define ACCEPT_FOOD_LOG "accepted food order"
#define REJECT_FOOD_LOG "rejected food order"
#define TIMEDOUT_FOOD_LOG "order timedout"
#define ACCEPT_ING_LOG "accepted ingredient order"
#define REJECT_ING_LOG "rejected ingredient order"
#define TIMEDOUT_ING_LOG "order timedout"
#define ACCEPT_ING_LOG "accepted ingredient order"
#define REJECT_ING_LOG "rejected ingredient order"
#define RCV_ING_LOG "recieved ingredient order"
#define RCV_FOOD_LOG "recieved food order"
#define RESTAURANT_OPEN_LOG "opened restaurant!"
#define RESTAURANT_CLOSE_LOG "closed restaurant!"
#define GET_SUPPLIER_LOG "trying to get supplier names"

void generate_file(char *username);
void log_msg(char *username, char *msg);

#endif