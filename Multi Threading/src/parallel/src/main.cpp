#include "image_processing.hpp"

using namespace std;

int main(int argc, char const *argv[])
{
    string image_addr = argv[1];
    ImageProcessing ip = ImageProcessing(image_addr);
    ip.run();
    return 0;
}
