#include "main_utils.hpp"
using namespace std;

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        exit(EXIT_FAILURE);
    }

    string buildings_dir_path = argv[1];
    MainUtils mu = MainUtils(buildings_dir_path);
    mu.run();
    return 0;
}
