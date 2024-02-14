#include "image_processing.hpp"

ImageProcessing::ImageProcessing(string image_addr_)
{
    image_addr = image_addr_;
}

void ImageProcessing::vertical_mirror_filter()
{
    for (int i = 0; i < (int)(rows / 2); i++)
    {
        for (int j = 0; j < cols; j++)
        {
            Pixel temp = {image[i][j].red, image[i][j].green, image[i][j].blue};
            image[i][j] = image[rows - 1 - i][j];
            image[rows - 1 - i][j] = temp;
        }
    }
}

Pixel ImageProcessing::three_by_three_convolution(int row, int col, vector<vector<Pixel>> &cpy_image)
{
    float red = 0, blue = 0, green = 0;

    for (int i = -1; i <= 1; ++i)
    {
        for (int j = -1; j <= 1; ++j)
        {
            int cur_row = row + i;
            int cur_col = col + j;
            Pixel pixel = cpy_image[cur_row][cur_col];
            red += (pixel.red) * blur_kernel[i + 1][j + 1];
            blue += (pixel.blue) * blur_kernel[i + 1][j + 1];
            green += (pixel.green) * blur_kernel[i + 1][j + 1];
        }
    }
    Pixel result = {(int)red, (int)green, (int)blue};
    return result;
}

void ImageProcessing::blur_filter(vector<vector<Pixel>> &cpy_image)
{
    for (int i = 1; i < rows - 1; ++i)
    {
        for (int j = 1; j < cols - 1; ++j)
        {

            image[i][j] = three_by_three_convolution(i, j, cpy_image);
        }
    }
}

void ImageProcessing::purple_haze_filter()
{
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            float red = 0, blue = 0, green = 0;
            red = (0.5 * image[i][j].red) + (0.3 * image[i][j].green) + (0.5 * image[i][j].blue);
            green = (0.16 * image[i][j].red) + (0.5 * image[i][j].green) + (0.16 * image[i][j].blue);
            blue = (0.6 * image[i][j].red) + (0.2 * image[i][j].green) + (0.8 * image[i][j].blue);
            Pixel result = {(int)red, (int)green, (int)blue};
            image[i][j] = result;
        }
    }
}

void ImageProcessing::hashor_filter()
{
    int i_starts[3] = {(int)(rows / 2) - 1,
                       rows - 1,
                       rows - 1};
    int i_ends[3] = {0,
                     0,
                     (int)(rows / 2) - 1};
    int j_starts[3] = {0,
                       0,
                       (int)(cols / 2) - 1};
    int j_ends[3] = {(int)(cols / 2) - 1,
                     cols - 1,
                     cols - 1};
    for (int k = 0; k < 3; k++)
    {
        draw_hashor(j_starts[k], i_starts[k], j_ends[k], i_ends[k]);
    }
}

string ImageProcessing::generate_execution_report(ExecutionReport rep)
{
    string result = "";
    result += READ_SIGN + to_string(rep.read_time).substr(0, to_string(rep.read_time).find(".") + 4) + TIME_SLICE;
    result += FLIP_SIGN + to_string(rep.flip).substr(0, to_string(rep.flip).find(".") + 4) + TIME_SLICE;
    result += BLUR_SIGN + to_string(rep.blur).substr(0, to_string(rep.blur).find(".") + 4) + TIME_SLICE;
    result += PURPLE_SIGN + to_string(rep.purple).substr(0, to_string(rep.purple).find(".") + 4) + TIME_SLICE;
    result += LINES_SIGN + to_string(rep.lines).substr(0, to_string(rep.lines).find(".") + 4) + TIME_SLICE;
    result += EXECUTION_SIGN + to_string(rep.exec_time).substr(0, to_string(rep.exec_time).find(".") + 4) + TIME_SLICE;
    return result;
}

void ImageProcessing::run()
{
    auto start_time = chrono::high_resolution_clock::now();

    char *file_buffer;
    int buffer_size;
    ExecutionReport report;

    if (!fill_and_allocate(file_buffer, image_addr.c_str(), rows, cols, buffer_size))
    {
        cout << FAILED_TO_READ_FILE_ERROR << endl;
        return;
    }
    vector<vector<Pixel>> cpy_image;

    auto start_read = chrono::high_resolution_clock::now();

    image = get_pixels_from_bmp24(buffer_size, rows, cols, file_buffer);
    auto end_read = chrono::high_resolution_clock::now();
    report.read_time = chrono::duration_cast<chrono::microseconds>(end_read - start_read).count() / 1000.0;

    auto start_flip = chrono::high_resolution_clock::now();
    vertical_mirror_filter();
    auto end_flip = chrono::high_resolution_clock::now();
    report.flip = chrono::duration_cast<chrono::microseconds>(end_flip - start_flip).count() / 1000.0;

    copy(image.begin(), image.end(), back_inserter(cpy_image));

    auto start_blur = chrono::high_resolution_clock::now();
    blur_filter(cpy_image);
    auto end_blur = chrono::high_resolution_clock::now();
    report.blur = chrono::duration_cast<chrono::microseconds>(end_blur - start_blur).count() / 1000.0;

    auto start_purple = chrono::high_resolution_clock::now();
    purple_haze_filter();
    auto end_purple = chrono::high_resolution_clock::now();
    report.purple = chrono::duration_cast<chrono::microseconds>(end_purple - start_purple).count() / 1000;

    auto start_lines = chrono::high_resolution_clock::now();
    hashor_filter();
    auto end_lines = chrono::high_resolution_clock::now();
    report.lines = chrono::duration_cast<chrono::microseconds>(end_lines - start_lines).count() / 1000.0;
    write_out_bmp24(image, rows, cols, file_buffer, OUTPUT_FILE, buffer_size);

    auto end_time = chrono::high_resolution_clock::now();
    report.exec_time = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count() / 1000.0;
    cout << generate_execution_report(report) << endl;
}

void ImageProcessing::draw_hashor(int src_x, int src_y, int dest_x, int dest_y)
{
    int dx = dest_x - src_x;
    int dy = dest_y - src_y;
    int steps = max(abs(dx), abs(dy));
    double x_step = static_cast<double>(dx) / (double)steps;
    double y_step = static_cast<double>(dy) / (double)steps;
    auto x = static_cast<double>(src_x);
    auto y = static_cast<double>(src_y);
    for (int i = 0; i <= steps; ++i)
    {
        image[y][x] = Pixel(WHITE, WHITE, WHITE);
        x += x_step;
        y += y_step;
    }
}