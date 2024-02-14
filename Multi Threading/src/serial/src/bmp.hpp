#ifndef __BMP_H__
#define __BMP_H__

#include "types.hpp"

using namespace std;

#pragma pack(push, 1)

typedef struct tagBITMAPFILEHEADER
{
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;
#pragma pack(pop)

bool fill_and_allocate(char *&buffer, const char *file_name, int &rows, int &cols, int &buffer_size);
vector<vector<Pixel>> get_pixels_from_bmp24(int end, int rows, int cols, char *file_read_buffer);
void write_out_bmp24(vector<vector<Pixel>> &image, int rows, int cols, char *file_buffer, const char *name_of_file_to_create, int buffer_size);

#endif