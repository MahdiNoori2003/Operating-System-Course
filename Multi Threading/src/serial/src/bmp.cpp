#include "bmp.hpp"

bool fill_and_allocate(char *&buffer, const char *file_name, int &rows, int &cols, int &buffer_size)
{
    std::ifstream file(file_name);
    if (!file)
    {
        std::cout << "File" << file_name << UNABLE_TO_FIND_FILE_ERROR << std::endl;
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streampos length = file.tellg();
    file.seekg(0, std::ios::beg);

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

vector<vector<Pixel>> get_pixels_from_bmp24(int end, int rows, int cols, char *file_read_buffer)
{
    vector<vector<Pixel>> image(rows, vector<Pixel>(cols, {0, 0, 0}));
    int count = 1;
    int extra = cols % 4;
    for (int i = 0; i < rows; i++)
    {
        count += extra;
        for (int j = cols - 1; j >= 0; j--)
        {
            for (int k = 0; k < 3; k++)
            {
                switch (k)
                {
                case 0:
                    image[i][j].red = file_read_buffer[end - count];
                    break;
                case 1:
                    image[i][j].green = file_read_buffer[end - count];
                    break;
                case 2:
                    image[i][j].blue = file_read_buffer[end - count];
                    break;
                }
                count++;
            }
        }
    }
    return image;
}

void write_out_bmp24(vector<vector<Pixel>> &image, int rows, int cols, char *file_buffer, const char *name_of_file_to_create, int buffer_size)
{
    std::ofstream write(name_of_file_to_create);
    if (!write)
    {
        std::cout << FAILED_TO_WRITE_ERROR << name_of_file_to_create << std::endl;
        return;
    }

    int count = 1;
    int extra = cols % 4;
    for (int i = 0; i < rows; i++)
    {
        count += extra;
        for (int j = cols - 1; j >= 0; j--)
        {
            for (int k = 0; k < 3; k++)
            {
                switch (k)
                {
                case 0:
                    file_buffer[buffer_size - count] = image[i][j].red;
                    break;
                case 1:
                    file_buffer[buffer_size - count] = image[i][j].green;
                    break;
                case 2:
                    file_buffer[buffer_size - count] = image[i][j].blue;
                    break;
                }
                count++;
            }
        }
    }
    write.write(file_buffer, buffer_size);
}
