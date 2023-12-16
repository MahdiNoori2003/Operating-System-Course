#include "log.h"

void generate_file(char *username)
{
    char *path = custom_sprintf("%s.log", username);

    int fd = creat(path, RD_WR);
    if (fd < 0)
        return;

    close(fd);
}

void log_msg(char *username, char *msg)
{
    char *path = custom_sprintf("%s.log", username);

    int fd = open(path, O_WRONLY | O_APPEND);

    char *result = custom_sprintf("%s\n", msg);
    write(fd, result, strlen(result));

    close(fd);
}