#include "resource_reduction_process.hpp"

ResourceReductionProcess::ResourceReductionProcess(string file_path_, int pipe_fd_, Resource res_type_)
{
    name = file_path_;
    file_path = file_path_ + ".csv";
    pipe_fd = pipe_fd_;
    res_type = res_type_;
    log_to_terminal(name + RESOURCE_PROCESS_CREATED_LOG, Prompt);
}

ResourceReductionProcess::~ResourceReductionProcess()
{
    close_fd(pipe_fd);
}

void ResourceReductionProcess::get_monthly_record()
{
    string line, word;
    fstream csv_file(file_path, ios::in);
    if (!csv_file.is_open())
    {
        return;
    }

    getline(csv_file, line);
    bool done = false;
    for (;;)
    {
        MonthRecord record;
        bool end_of_month = false;
        vector<vector<int>> usages(DAY_COUNT, vector<int>(HOUR_COUNT, 0));
        for (;;)
        {
            if (getline(csv_file, line).eof())
            {
                done = true;
                break;
            }
            stringstream str(line);
            int i = 0;
            int day;
            while (getline(str, word, ','))
            {
                if (i == 0)
                    record.year = stoi(word);
                else if (i == 1)
                    record.month = stoi(word);

                else if (i == 2)
                {
                    day = stoi(word);
                    if (stoi(word) == DAY_COUNT)
                        end_of_month = true;
                }

                else
                {
                    usages[day - 1][i - 3] = stoi(word);
                }
                i++;
            }
            if (end_of_month)
            {

                fill_record(usages, record);
                break;
            }
        }
        if (!done)
            records.push_back(record);
        else
            break;
    }
    csv_file.close();
}

void ResourceReductionProcess::fill_record(vector<vector<int>> &usages, MonthRecord &record)
{
    record.whole_usage = calc_whole_usage(usages);
    record.average_usage = calc_average_usage(record.whole_usage);
    record.max_usage_hour = calc_max_usage_hour(usages);
    record.whole_usage_in_max_hours = calc_whole_usage_in_max_hours(usages, record.max_usage_hour);
    record.difference_from_average_in_max_hours = calc_difference_from_average_in_max_hours(record.whole_usage, record.whole_usage_in_max_hours);
    record.whole_usage_in_min_hours = calc_whole_usage_in_min_hours(usages);
}

int ResourceReductionProcess::calc_whole_usage(vector<vector<int>> &usages)
{
    int result = 0;
    for (int i = 0; i < DAY_COUNT; i++)
    {
        for (int j = 0; j < HOUR_COUNT; j++)
        {
            result += usages[i][j];
        }
    }
    return result;
}

double ResourceReductionProcess::calc_average_usage(int whole_usage)
{
    return whole_usage / DAY_COUNT;
}
int ResourceReductionProcess::calc_max_usage_hour(vector<vector<int>> &usages)
{
    vector<int> whole_usage_per_hour = vector<int>(HOUR_COUNT, 0);
    for (int i = 0; i < DAY_COUNT; i++)
    {
        for (int j = 0; j < HOUR_COUNT; j++)
        {
            whole_usage_per_hour[j] += usages[i][j];
        }
    }
    return argmax(whole_usage_per_hour);
}
int ResourceReductionProcess::calc_whole_usage_in_max_hours(vector<vector<int>> &usages, int max_hour)
{
    int result = 0;
    for (int i = 0; i < DAY_COUNT; i++)
    {
        result += usages[i][max_hour];
    }
    return result;
}

int ResourceReductionProcess::calc_difference_from_average_in_max_hours(int whole_usage, int max_hour_usage)
{
    return whole_usage - max_hour_usage;
}

int ResourceReductionProcess::calc_whole_usage_in_min_hours(vector<vector<int>> &usages)
{
    vector<int> whole_usage_per_hour = vector<int>(HOUR_COUNT, 0);
    for (int i = 0; i < DAY_COUNT; i++)
    {
        for (int j = 0; j < HOUR_COUNT; j++)
        {
            whole_usage_per_hour[j] += usages[i][j];
        }
    }
    int result = 0;
    for (int i = 0; i < DAY_COUNT; i++)
    {
        result += usages[i][argmin(whole_usage_per_hour)];
    }
    return result;
}

void ResourceReductionProcess::run()
{
    get_monthly_record();
    string encoded_records = month_record_encoder(records);
    write_fd(encoded_records.c_str(), encoded_records.size(), pipe_fd);
    log_to_terminal(name + RESOURCE_SEND_LOG, Send);
}
