#include "unnamed_pipe.hpp"
#include "logger.hpp"

vector<int> create_pipe()
{
    vector<int> result;
    int fd[2];
    if (pipe(fd) == -1)
    {
        log_to_terminal(FAILED_TO_CREATE_PIPE_UNNAMED_PIPE_ERROR, Error);
    }
    result.push_back(fd[0]);
    result.push_back(fd[1]);
    return result;
}

bool close_fd(int fd)
{
    if (close(fd) == -1)
    {
        log_to_terminal(FAILED_TO_CLOSE_UNNAMED_PIPE_ERROR, Error);
        return false;
    }

    return true;
}

int write_fd(const char *buffer, int size, int write_fd)
{
    int bytes_written = write(write_fd, buffer, size);
    if (bytes_written == -1)
    {
        log_to_terminal(FAILED_TO_WRITE_UNNAMED_PIPE_ERROR, Error);
    }

    return bytes_written;
}

string read_fd(int read_fd)
{
    char buffer[BUFF_SIZE];
    string result;

    while (true)
    {
        int read_bytes = read(read_fd, buffer, BUFF_SIZE);

        if (read_bytes == -1)
        {
            if (errno == EAGAIN)
            {
                break;
            }
            else
            {
                log_to_terminal(FAILED_TO_READ_UNNAMED_PIPE_ERROR, Error);
                break;
            }
        }
        else if (read_bytes < BUFF_SIZE)
        {
            result.append(buffer, read_bytes);
            break;
        }

        result.append(buffer, read_bytes);
    }
    return result;
}
