#include "resource_reduction_process.hpp"

int main(int argc, char const *argv[])
{
    if (argc != 4)
    {
        exit(EXIT_FAILURE);
    }
    ResourceReductionProcess rrp = ResourceReductionProcess(argv[1], stoi(argv[2]), Resource(stoi(argv[3])));
    rrp.run();
}
