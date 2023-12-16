#ifndef __MAIN_UTILS_H__
#define __MAIN_UTILS_H__
#include "unnamed_pipe.hpp"
#include "logger.hpp"

class MainUtils
{
private:
    map<string, int> building_names_map;
    string buildings_dir_path;
    vector<vector<int>> main_read_pipes;
    vector<vector<int>> main_write_pipes;
    void create_building_names_map();
    void make_buildings();
    void make_bill_center();
    void get_user_request(string &buidings_requested, string &resources_requested,
                          string &report_parameter_requested);

    void send_user_request_to_buildings(string &buidings_requested, string &resources_requested,
                                        string &report_parameter_requested);
    void recieve_reports();

public:
    MainUtils(string buildings_dir_path_);
    void run();
    ~MainUtils();
};

#endif