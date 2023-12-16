#include "types.hpp"
using namespace std;

vector<int> create_pipe();

bool close_fd(int fd);

int write_fd(const char *buffer, int size, int write_fd);

string read_fd(int read_fd);
