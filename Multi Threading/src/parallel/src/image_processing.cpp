#include "image_processing.hpp"

ImageProcessing::ImageProcessing(string image_addr_)
{
    image_addr = image_addr_;
}

void *ImageProcessing::vertical_mirror_filter(void *thread)
{
    Thread *thread_struct = static_cast<Thread *>(thread);
    int rows = thread_struct->args->rows;
    int cols = thread_struct->args->cols;
    long tid = thread_struct->tid;
    vector<vector<Pixel>> *image = thread_struct->args->image;

    int start_col = float(float(cols) / NUM_OF_THREADS) * tid;
    int end_col = float(float(cols) / NUM_OF_THREADS) * (tid + 1);

    if (cols < end_col)
        end_col = cols;

    for (int i = 0; i < (int)(rows / 2); i++)
    {
        for (int j = start_col; j < end_col; j++)
        {
            Pixel temp = {(*image)[i][j].red, (*image)[i][j].green, (*image)[i][j].blue};
            (*image)[i][j] = (*image)[rows - 1 - i][j];
            (*image)[rows - 1 - i][j] = temp;
        }
    }
    pthread_exit(NULL);
}

void *ImageProcessing::blur_filter(void *thread)
{
    Thread *thread_struct = static_cast<Thread *>(thread);
    int rows = thread_struct->args->rows;
    int cols = thread_struct->args->cols;
    long tid = thread_struct->tid;
    vector<vector<Pixel>> *image = thread_struct->args->image;
    vector<vector<Pixel>> *copy_image = thread_struct->args->copy_image;

    int start_row = float(float(rows) / NUM_OF_THREADS) * tid;
    int end_row = float(float(rows) / NUM_OF_THREADS) * (tid + 1);
    if (rows <= end_row)
        end_row = rows - 1;

    if (start_row == 0)
        start_row = 1;

    for (int k = start_row; k < end_row; ++k)
    {
        for (int t = 1; t < cols - 1; ++t)
        {
            float red = 0, blue = 0, green = 0;

            for (int i = -1; i <= 1; ++i)
            {
                for (int j = -1; j <= 1; ++j)
                {
                    int cur_row = k + i;
                    int cur_col = t + j;
                    Pixel pixel = (*copy_image)[cur_row][cur_col];
                    red += (pixel.red) * blur_kernel[i + 1][j + 1];
                    blue += (pixel.blue) * blur_kernel[i + 1][j + 1];
                    green += (pixel.green) * blur_kernel[i + 1][j + 1];
                }
            }
            (*image)[k][t] = {(int)red, (int)green, (int)blue};
        }
    }
    pthread_exit(NULL);
}

void *ImageProcessing::purple_haze_filter(void *thread)
{
    Thread *thread_struct = static_cast<Thread *>(thread);
    int rows = thread_struct->args->rows;
    int cols = thread_struct->args->cols;
    long tid = thread_struct->tid;
    vector<vector<Pixel>> *image = thread_struct->args->image;

    int start_row = float(float(rows) / NUM_OF_THREADS) * tid;
    int end_row = float(float(rows) / NUM_OF_THREADS) * (tid + 1);
    if (rows < end_row)
        end_row = rows;

    for (int i = start_row; i < end_row; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            float red = 0, blue = 0, green = 0;
            red = (0.5 * (*image)[i][j].red) + (0.3 * (*image)[i][j].green) + (0.5 * (*image)[i][j].blue);
            green = (0.16 * (*image)[i][j].red) + (0.5 * (*image)[i][j].green) + (0.16 * (*image)[i][j].blue);
            blue = (0.6 * (*image)[i][j].red) + (0.2 * (*image)[i][j].green) + (0.8 * (*image)[i][j].blue);
            Pixel result = {(int)red, (int)green, (int)blue};
            (*image)[i][j] = result;
        }
    }
    pthread_exit(NULL);
}

void ImageProcessing::hashor_filter()
{
    int i_starts[3] = {(int)(main_rows / 2) - 1,
                       main_rows - 1,
                       main_rows - 1};
    int i_ends[3] = {0,
                     0,
                     (int)(main_rows / 2) - 1};
    int j_starts[3] = {0,
                       0,
                       (int)(main_cols / 2) - 1};
    int j_ends[3] = {(int)(main_cols / 2) - 1,
                     main_cols - 1,
                     main_cols - 1};
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

    if (!fill_and_allocate(file_buffer, image_addr.c_str(), main_rows, main_cols, buffer_size))
    {
        cout << FAILED_TO_READ_FILE_ERROR << endl;
        return;
    }

    main_image.assign(main_rows, vector<Pixel>(main_cols, {0, 0, 0}));
    vector<vector<Pixel>> copy_image;

    Args args = {&main_image, &copy_image, file_buffer, buffer_size, main_rows, main_cols};

    auto start_read = chrono::high_resolution_clock::now();
    parallel_exec(get_pixels_from_bmp24, &args, NUM_OF_IO_THREADS);

    auto end_read = chrono::high_resolution_clock::now();
    report.read_time = chrono::duration_cast<chrono::microseconds>(end_read - start_read).count() / 1000.0;

    auto start_flip = chrono::high_resolution_clock::now();
    parallel_exec(vertical_mirror_filter, &args, NUM_OF_THREADS);
    auto end_flip = chrono::high_resolution_clock::now();
    report.flip = chrono::duration_cast<chrono::microseconds>(end_flip - start_flip).count() / 1000.0;

    copy(main_image.begin(), main_image.end(), back_inserter(copy_image));

    auto start_blur = chrono::high_resolution_clock::now();
    parallel_exec(blur_filter, &args, NUM_OF_THREADS);
    auto end_blur = chrono::high_resolution_clock::now();
    report.blur = chrono::duration_cast<chrono::microseconds>(end_blur - start_blur).count() / 1000.0;

    auto start_purple = chrono::high_resolution_clock::now();
    parallel_exec(purple_haze_filter, &args, NUM_OF_THREADS);
    auto end_purple = chrono::high_resolution_clock::now();
    report.purple = chrono::duration_cast<chrono::microseconds>(end_purple - start_purple).count() / 1000;
    auto start_lines = chrono::high_resolution_clock::now();
    hashor_filter();
    auto end_lines = chrono::high_resolution_clock::now();
    report.lines = chrono::duration_cast<chrono::microseconds>(end_lines - start_lines).count() / 1000.0;
    ofstream write(OUTPUT_FILE);
    if (!write)
    {
        cout << FAILED_TO_WRITE_ERROR << OUTPUT_FILE << endl;
        exit(0);
    }
    parallel_exec(write_out_bmp24, &args, NUM_OF_IO_THREADS);
    write.write(file_buffer, buffer_size);

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
        main_image[y][x] = Pixel(WHITE, WHITE, WHITE);
        x += x_step;
        y += y_step;
    }
}
