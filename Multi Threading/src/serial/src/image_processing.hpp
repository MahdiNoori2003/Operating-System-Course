#ifndef __IMG_PROCESSING_H__
#define __IMG_PROCESSING_H__
#include "bmp.hpp"

using namespace std;

class ImageProcessing
{
private:
    vector<vector<Pixel>> image;
    string image_addr;
    int rows;
    int cols;
    Pixel three_by_three_convolution(int row, int col, vector<vector<Pixel>> &cpy_image);
    void draw_hashor(int src_x, int src_y, int dest_x, int dest_y);

public:
    ImageProcessing(string image_addr_);
    void vertical_mirror_filter();
    void blur_filter(vector<vector<Pixel>> &cpy_image);
    void purple_haze_filter();
    void hashor_filter();
    void run();
    string generate_execution_report(ExecutionReport rep);
};

#endif
