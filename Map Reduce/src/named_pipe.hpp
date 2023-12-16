#ifndef __NAMED_PIPE_H__
#define __NAMED_PIPE_H__

#include "types.hpp"
using namespace std;

class NamedPipe
{
public:
    NamedPipe(const string &pipe_name);

    static void remove_pipe(const string &pipe_path);

protected:
    string pipe_name_;
    int pipe_fd_;
};

class NamedPipeClient : NamedPipe
{
public:
    NamedPipeClient(const string &pipe_name);

    void send(const string &msg);

    ~NamedPipeClient();
};

class NamedPipeServer : NamedPipe
{
public:
    NamedPipeServer(const string &pipe_name);

    string receive();

    ~NamedPipeServer();
};

#endif
