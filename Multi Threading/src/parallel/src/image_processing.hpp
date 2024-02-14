#ifndef __IMG_PROCESSING_H__
#define __IMG_PROCESSING_H__
#include "bmp.hpp"

using namespace std;

class ImageProcessing
{
private:
    vector<vector<Pixel>> main_image;
    string image_addr;
    int main_rows;
    int main_cols;
    void draw_hashor(int src_x, int src_y, int dest_x, int dest_y);

public:
    ImageProcessing(string image_addr_);
    static void *vertical_mirror_filter(void *args);
    static void *blur_filter(void *args);
    static void *purple_haze_filter(void *args);
    void hashor_filter();
    void run();
    string generate_execution_report(ExecutionReport rep);
};

#endif
