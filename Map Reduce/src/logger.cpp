#include "logger.hpp"
#include <fstream>
#include <ctime>

string get_current_time_stamp()
{
    time_t now = time(nullptr);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return buffer;
}

string colorify_text(const string &message, LogType type)
{
    string log_message = message;

    if (type == Warning)
    {
        log_message = YELLOW_COLOR + log_message + RESET_COLOR;
    }
    else if (type == Error)
    {
        log_message = RED_COLOR + log_message + RESET_COLOR;
    }
    else if (type == Prompt)
    {
        log_message = CYAN_COLOR + log_message + RESET_COLOR;
    }
    else if (type == Send)
    {
        log_message = GREEN_COLOR + log_message + RESET_COLOR;
    }
    else if (type == Recieve)
    {
        log_message = PINK_COLOR + log_message + RESET_COLOR;
    }
    else if (type == Pointer)
    {
        log_message = DARK_BLUE_COLOR + log_message + RESET_COLOR;
    }
    return log_message;
}

void log_to_terminal(const string &message, LogType type)
{
    string curr_time = "[" + get_current_time_stamp() + "] ";
    string log_message = colorify_text(message, type);
    cout << curr_time << log_message << endl;
}
