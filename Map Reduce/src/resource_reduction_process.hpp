#ifndef __RESOURCE_REDUCTION_HPP
#define __RESOURCE_REDUCTION_HPP

#include "unnamed_pipe.hpp"
#include "logger.hpp"
using namespace std;

class ResourceReductionProcess
{
private:
    int pipe_fd;
    string file_path;
    string name;
    Resource res_type;
    vector<MonthRecord> records;

    void get_monthly_record();
    void fill_record(vector<vector<int>> &usages, MonthRecord &record);
    int calc_whole_usage(vector<vector<int>> &usages);
    double calc_average_usage(int whole_usage);
    int calc_max_usage_hour(vector<vector<int>> &usages);
    int calc_whole_usage_in_max_hours(vector<vector<int>> &usages, int max_hour);
    int calc_difference_from_average_in_max_hours(int whole_usage, int max_hour_usage);
    int calc_whole_usage_in_min_hours(vector<vector<int>> &usages);

public:
    ResourceReductionProcess(string file_path_, int pipe_fd_, Resource res_type_);
    ~ResourceReductionProcess();
    void run();
};

#endif