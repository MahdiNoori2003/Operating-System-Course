#include "named_pipe.hpp"
#include "logger.hpp"
class BillProcess
{
private:
    shared_ptr<NamedPipeServer> bill_server;
    shared_ptr<NamedPipeClient> bill_client;
    map<string, vector<map<string, int>>> building_records_map;
    int buildings_count;
    string client;
    void decode_building_data(string &encoded_data);
    vector<MonthBill> create_bills();
    double calc_monthly_elec_bill(int whole_usage, int whole_usage_max_hours, int whole_usage_min_hours);
    double calc_monthly_gas_bill(int whole_usage);
    double calc_monthly_water_bill(int whole_usage, int whole_usage_max_hours);

public:
    BillProcess(int buildings_count_);
    void run();
    ~BillProcess();
};
