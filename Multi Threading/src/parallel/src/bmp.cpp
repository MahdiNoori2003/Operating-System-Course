#include "bmp.hpp"

bool fill_and_allocate(char *&buffer, const char *file_name, int &rows, int &cols, int &buffer_size)
{
    ifstream file(file_name);
    if (!file)
    {
        cout << "File" << file_name << UNABLE_TO_FIND_FILE_ERROR << endl;
        return false;
    }

    file.seekg(0, ios::end);
    streampos length = file.tellg();
    file.seekg(0, ios::beg);

    buffer = new char[length];
    file.read(&buffer[0], length);

    PBITMAPFILEHEADER file_header;
    PBITMAPINFOHEADER info_header;

    file_header = (PBITMAPFILEHEADER)(&buffer[0]);
    info_header = (PBITMAPINFOHEADER)(&buffer[0] + sizeof(BITMAPFILEHEADER));
    rows = info_header->biHeight;
    cols = info_header->biWidth;
    buffer_size = file_header->bfSize;
    return true;
}

void *get_pixels_from_bmp24(void *thread)
{
    Thread *thread_struct = static_cast<Thread *>(thread);
    int rows = thread_struct->args->rows;
    int cols = thread_struct->args->cols;
    long tid = thread_struct->tid;
    int buffer_size = thread_struct->args->buffer_size;
    vector<vector<Pixel>> *image = thread_struct->args->image;

    int start_row = float(float(rows) / NUM_OF_IO_THREADS) * tid;
    int end_row = float(float(rows) / NUM_OF_IO_THREADS) * (tid + 1);
    if (rows < end_row)
        end_row = rows;

    int extra = cols % 4;
    int count = (tid * (rows / NUM_OF_IO_THREADS)) * (cols * 3 + extra) + 1;

    for (int i = start_row; i < end_row; i++)
    {
        count += extra;
        for (int j = cols - 1; j >= 0; j--)
        {
            unsigned char pixels[3];
            for (int k = 0; k < 3; k++)
            {
                pixels[k] = thread_struct->args->file_buffer[buffer_size - count];
                count++;
            }
            (*image)[i][j] = Pixel(pixels[0], pixels[1], pixels[2]);
        }
    }
    pthread_exit(NULL);
}

void *write_out_bmp24(void *thread)
{

    Thread *thread_struct = static_cast<Thread *>(thread);
    int rows = thread_struct->args->rows;
    int cols = thread_struct->args->cols;
    long tid = thread_struct->tid;
    int buffer_size = thread_struct->args->buffer_size;
    vector<vector<Pixel>> *image = thread_struct->args->image;

    int start_row = float(float(rows) / NUM_OF_IO_THREADS) * tid;
    int end_row = float(float(rows) / NUM_OF_IO_THREADS) * (tid + 1);
    if (rows < end_row)
        end_row = rows;
    int extra = cols % 4;
    int count = (tid * (rows / NUM_OF_IO_THREADS)) * (cols * 3 + extra) + 1;
    for (int i = start_row; i < end_row; i++)
    {
        count += extra;
        for (int j = cols - 1; j >= 0; j--)
        {
            for (int k = 0; k < 3; k++)
            {
                switch (k)
                {
                case 0:
                    thread_struct->args->file_buffer[buffer_size - count] = (*image)[i][j].red;
                    break;
                case 1:
                    thread_struct->args->file_buffer[buffer_size - count] = (*image)[i][j].green;
                    break;
                case 2:
                    thread_struct->args->file_buffer[buffer_size - count] = (*image)[i][j].blue;
                    break;
                }
                count++;
            }
        }
    }

    pthread_exit(NULL);
}
