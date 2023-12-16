#include "named_pipe.hpp"
#include "logger.hpp"

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>

#include <string>

NamedPipe::NamedPipe(const string &pipe_name)
    : pipe_name_(pipe_name) {}

void NamedPipe::remove_pipe(const string &pipe_path)
{
    remove(pipe_path.c_str());
}

NamedPipeClient::NamedPipeClient(const string &pipe_name)
    : NamedPipe(pipe_name)
{
    do
    {
        pipe_fd_ = open(pipe_name.c_str(), O_WRONLY);
    } while (pipe_fd_ == -1);
}

void NamedPipeClient::send(const string &msg)
{
    int sent_bytes = write(pipe_fd_, msg.c_str(), msg.size());
    if (sent_bytes != msg.size())
        log_to_terminal(FAILED_TO_SEND_NAMED_PIPE_ERROR, Error);
}

NamedPipeServer::NamedPipeServer(const string &pipe_name)
    : NamedPipe(pipe_name)
{
    if ((pipe_fd_ = open(pipe_name.c_str(), O_RDONLY | O_NONBLOCK)) == -1)
    {
        if (mkfifo(pipe_name.c_str(), 0777) != 0)
        {
            log_to_terminal(FAILED_TO_MAKE_FIFO_NAMED_PIPE_ERROR, Error);
        }
        else
            pipe_fd_ = open(pipe_name_.c_str(), O_RDONLY | O_NONBLOCK);
    }
    if (pipe_fd_ == -1)
        log_to_terminal(FAILED_TO_OPEN_SERVER_NAMED_PIPE_ERROR + pipe_name + " !", Error);
}

NamedPipeClient::~NamedPipeClient()
{
    close(pipe_fd_);
}

string NamedPipeServer::receive()
{
    char buffer[BUFF_SIZE];
    string result;

    while (true)
    {
        int read_bytes = read(pipe_fd_, buffer, BUFF_SIZE);

        if (read_bytes == -1)
        {
            break;
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

NamedPipeServer::~NamedPipeServer()
{
    close(pipe_fd_);
}
