#include "bill_process.hpp"

int main(int argc, char const *argv[])
{
    BillProcess bp = BillProcess(stoi(argv[1]));
    bp.run();
}
