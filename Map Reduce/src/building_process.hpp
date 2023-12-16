#ifndef __BULDING_PROCESS_H__
#define __BULDING_PROCESS_H__

#include "unnamed_pipe.hpp"
#include "named_pipe.hpp"
#include "logger.hpp"

class BuildingProcess
{
private:
    int write_pipe_fd;
    int read_pipe_fd;
    string path;
    string name;
    shared_ptr<NamedPipeServer> building_server;
    shared_ptr<NamedPipeClient> building_client;
    vector<vector<int>> *resources_read_pipes;
    map<string, vector<map<string, int>>> resource_records_map;
    vector<map<string, double>> bills_map;

    string gather_resource_data();
    string create_record_building(string report_parameter,
                                  vector<string> resources);
    template <typename T>
    string monthly_report_generator(vector<map<string, T>> &records, string report_parameter, string parameter_tag);
    string get_main_command();
    void get_bills();
    string get_building_name(string path);

public:
    BuildingProcess(string path_, int write_pipe_fd_, int read_pipe_fd_, vector<vector<int>> *resources_read_pipes_);
    void run();
    ~BuildingProcess();
};

#endif