#ifndef __LOGGER_H__
#define __LOGGER_H__

using namespace std;
#include <iostream>

#define RESET_COLOR "\033[0m"
#define RED_COLOR "\033[31m"
#define YELLOW_COLOR "\033[33m"
#define CYAN_COLOR "\033[36m"
#define GREEN_COLOR "\033[0;32m"
#define DARK_BLUE_COLOR "\033[0;34m"
#define PINK_COLOR "\033[1;35m"

const string MAIN_SEND_LOG = "main sent reqests to buildings !";
const string MAIN_BUILDING_LOG = " building as building number ";
const string BUILDING_PROCESS_CREATED_LOG = " process created !";
const string BUILDING_RECIEVED_MAIN_LOG = " recieved main request !";
const string BUILDING_SEND_MAIN_LOG = " sent rport to main !";
const string BUILDING_RECIEVED_RESOURCE_RECORD_LOG = " recieved ";
const string BUILDING_SEND_LOG = " sent records to bill center !";
const string BUILDING_RECIEVED_BILL_LOG = " recieved bills from bill center !";
const string BILL_CENTER_CREATED_LOG = "bill center process created !";
const string BILL_CENTER_RECIEVED_LOG = "bill center recieved records from ";
const string BILL_CENTER_SEND_LOG = "bill center send bills to ";
const string RESOURCE_PROCESS_CREATED_LOG = " resource reduction process created !";
const string RESOURCE_SEND_LOG = " resource reduction send records to its parent !";

const string FAILED_TO_FORK_ERROR = " failed to fork ";
const string FAILED_TO_EXEC_ERROR = " failed to exec";
const string FAILED_TO_SEND_NAMED_PIPE_ERROR = "failed to send on named pipe !";
const string FAILED_TO_MAKE_FIFO_NAMED_PIPE_ERROR = "failed to create fifo in named pipe !";
const string FAILED_TO_OPEN_SERVER_NAMED_PIPE_ERROR = "failed to open server side on named pipe ";
const string FAILED_TO_CREATE_PIPE_UNNAMED_PIPE_ERROR = "failed to create pipe in unnamed pipe !";
const string FAILED_TO_CLOSE_UNNAMED_PIPE_ERROR = "failed to close pipe in unnamed pipe !";
const string FAILED_TO_WRITE_UNNAMED_PIPE_ERROR = "failed to write on pipe in unnamed pipe !";
const string FAILED_TO_READ_UNNAMED_PIPE_ERROR = "failed to read from pipe in unnamed pipe !";

enum LogType
{
    Warning,
    Prompt,
    Error,
    Send,
    Recieve,
    Pointer,
    Normal
};

string colorify_text(const string &message, LogType type);
void log_to_terminal(const string &message, LogType type);

#endif