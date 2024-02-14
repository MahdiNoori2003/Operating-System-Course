#ifndef __TYPES_H__
#define __TYPES_H__

#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>

using namespace std;

#define WHITE 255
#define NUM_OF_THREADS 8
#define NUM_OF_IO_THREADS 8

#define FAILED_TO_WRITE_ERROR "Failed to write "
#define UNABLE_TO_FIND_FILE_ERROR " doesn't exist!"
#define FAILED_TO_READ_FILE_ERROR "File read error"
#define UNABLE_TO_CREATE_THREAD_ERROR "unable to create thread"
#define UNABLE_TO_JOIN_THREAD_ERROR "unable to join threads"

#define OUTPUT_FILE "outputs/parallel-output.bmp"

const string CYAN_COLOR = "\033[36m";
const string RESET_COLOR = "\033[0m";

const string READ_SIGN = CYAN_COLOR + "Read: " + RESET_COLOR;
const string FLIP_SIGN = CYAN_COLOR + "Flip: " + RESET_COLOR;
const string BLUR_SIGN = CYAN_COLOR + "Blur: " + RESET_COLOR;
const string PURPLE_SIGN = CYAN_COLOR + "Purple: " + RESET_COLOR;
const string LINES_SIGN = CYAN_COLOR + "Lines: " + RESET_COLOR;
const string EXECUTION_SIGN = CYAN_COLOR + "Execution: " + RESET_COLOR;

const string TIME_SLICE = " ms\n";

typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;

struct Pixel
{
    unsigned char red, green, blue;
    Pixel(int red_, int green_, int blue_)
    {
        red = (unsigned char)min(max<int>(red_, 0), 255);
        blue = (unsigned char)min(max<int>(blue_, 0), 255);
        green = (unsigned char)min(max<int>(green_, 0), 255);
    }
};

struct ExecutionReport
{
    double read_time, blur, flip, purple, lines, exec_time;
};

const float blur_kernel[3][3] = {
    {1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0},
    {2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0},
    {1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0}};

#endif