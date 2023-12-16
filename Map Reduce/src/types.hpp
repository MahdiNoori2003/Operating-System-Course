#ifndef __TYPES_H__
#define __TYPES_H__

#include <iostream>
#include <filesystem>
#include <vector>
#include <map>
#include <unistd.h>
#include <fstream>
#include <cstring>
#include <sys/wait.h>
#include <sstream>

using namespace std;

#define MONTH_COUNT 12
#define DAY_COUNT 30
#define HOUR_COUNT 6
#define RESOURCES_COUNT 3
#define BUFF_SIZE 256
#define MAX_HOURS_COEFF 1.25
#define MIN_HOURS_COEFF 0.75

#define PATH_TO_BULDING_PROGRAM "./building.out"
#define PATH_TO_RESOURCE_PROGRAM "./resource_reduction.out"
#define PATH_TO_BILL_PROGRAM "./bill_center.out"

#define WATER "Water"
#define GAS "Gas"
#define ELECTRICITY "Electricity"

#define WATER_CMD "water"
#define GAS_CMD "gas"
#define ELECTRICITY_CMD "electricity"

#define WHOLE_USAGE_CMD "whole-usage"
#define AVG_USAGE_CMD "average-usage"
#define MAX_HOUR_CMD "max-usage-hour"
#define BILL_CMD "bill"
#define DIFF_MAX_AVG_CMD "diff-max-avg"
#define USAGE_IN_MAX_HOURS_CMD "whole-usage-in-max-hours"
#define USAGE_IN_MIN_HOURS_CMD "whole-usage-in-min-hours"

#define BILL_SERVER "bill"

#define BILL_TAG "bill-cost"

#define GET_BUILDINGS_PROMPT "which buildings you need the report for ?"
#define GET_RESOURCES_PROMPT "which resources you need the report for (electricity/water/gas) ?"
#define GET_REPORT_PROMPT "which field you need (whole-usage/average-usage/max-usage-hour/bill/diff-max-avg)?"
#define KILL_PROCESS_MSG "Terminate"

#define POINTER ">> "
#define NUMBER_SEPERATOR ','
#define ARRAY_SEPERATOR '/'
#define ELEMENT_SEPERATOR '*'
#define USER_INPUT_SEPERATOR ' '

enum Resource
{
    Electricity,
    Water,
    Gas
};

struct MonthRecord
{
    int year;
    int month;
    int whole_usage;
    double average_usage;
    int max_usage_hour;
    int whole_usage_in_max_hours;
    int difference_from_average_in_max_hours;
    int whole_usage_in_min_hours;
};

struct MonthBill
{
    int year;
    int month;
    double elec_bill;
    double gas_bill;
    double water_bill;
};

int argmax(const vector<int> &numbers);

int argmin(const vector<int> &numbers);

string month_record_encoder(vector<MonthRecord> &records);

vector<map<string, int>> month_record_decoder(string &encoded_string);

string month_bill_encoder(vector<MonthBill> &bills);

vector<map<string, double>> month_bill_decoder(string &encoded_string);

void main_cmd_decoder(string cmd, vector<string> &resources, string &report_parameter);

// void print_to_terminal(const string &message, LogType type);

#endif