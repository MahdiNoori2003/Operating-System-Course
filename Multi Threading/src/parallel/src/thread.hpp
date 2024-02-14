#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>
#include "types.hpp"

struct Args
{
    vector<vector<Pixel>> *image;
    vector<vector<Pixel>> *copy_image;
    char *file_buffer;
    int buffer_size, rows, cols;
};

struct Thread
{
    Thread(pthread_t tid_, Args *args_)
    {
        tid = tid_;
        args = args_;
    }
    pthread_t tid;
    Args *args;
};

void parallel_exec(void *(*runner)(void *), Args *args, int num_of_threads);

#endif