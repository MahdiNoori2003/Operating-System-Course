#include "thread.hpp"

void parallel_exec(void *(*runner)(void *), Args *args, int num_of_threads)
{
    pthread_t threads[num_of_threads];
    int rc;
    long i;
    for (i = 0; i < num_of_threads; i++)
    {
        Thread *thread = new Thread(i, args);
        rc = pthread_create(&threads[i], NULL, runner, (void *)(thread));
        if (rc)
        {
            cout << UNABLE_TO_CREATE_THREAD_ERROR << endl;
            exit(-1);
        }
    }
    for (i = 0; i < num_of_threads; i++)
    {
        rc = pthread_join(threads[i], NULL);
        if (rc)
        {
            cout << UNABLE_TO_JOIN_THREAD_ERROR << endl;
            exit(-1);
        }
    }
}