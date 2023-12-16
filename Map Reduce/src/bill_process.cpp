#include "bill_process.hpp"

BillProcess::BillProcess(int buildings_count_)
{
    buildings_count = buildings_count_;
    const string addrs = BILL_SERVER;
    bill_server = make_shared<NamedPipeServer>(addrs);
    log_to_terminal(BILL_CENTER_CREATED_LOG, Prompt);
}

BillProcess::~BillProcess()
{
    string addrs = BILL_SERVER;
    NamedPipe::remove_pipe(addrs);
}

void BillProcess::decode_building_data(string &encoded_data)
{
    const string resources[3] = {ELECTRICITY, WATER, GAS};

    stringstream stream(encoded_data);
    string sentence;
    int i = 0;
    while (getline(stream, sentence, ELEMENT_SEPERATOR))
    {
        if (i == 0)
        {
            client = sentence;
            bill_client = make_shared<NamedPipeClient>(sentence);
        }
        else
        {
            building_records_map[resources[i - 1]] = month_record_decoder(sentence);
        }
        i++;
    }
}

vector<MonthBill> BillProcess::create_bills()
{
    vector<MonthBill> bills;
    for (int i = 0; i < int(building_records_map[ELECTRICITY].size()); i++)
    {
        MonthBill bill;
        bill.year = building_records_map[ELECTRICITY][i]["year"];

        bill.month = building_records_map[ELECTRICITY][i]["month"];

        bill.elec_bill = calc_monthly_elec_bill(building_records_map[ELECTRICITY][i][WHOLE_USAGE_CMD],
                                                building_records_map[ELECTRICITY][i][USAGE_IN_MAX_HOURS_CMD],
                                                building_records_map[ELECTRICITY][i][USAGE_IN_MIN_HOURS_CMD]);

        bill.gas_bill = calc_monthly_gas_bill(building_records_map[GAS][i][WHOLE_USAGE_CMD]);

        bill.water_bill = calc_monthly_water_bill(building_records_map[WATER][i][WHOLE_USAGE_CMD],
                                                  building_records_map[WATER][i][USAGE_IN_MAX_HOURS_CMD]);

        bills.push_back(bill);
    }
    return bills;
}

double BillProcess::calc_monthly_elec_bill(int whole_usage, int whole_usage_max_hours,
                                           int whole_usage_min_hours)
{
    return (whole_usage - (whole_usage_max_hours + whole_usage_min_hours)) + (whole_usage_max_hours * MAX_HOURS_COEFF) +
           (whole_usage_min_hours * MIN_HOURS_COEFF);
}

double BillProcess::calc_monthly_gas_bill(int whole_usage)
{
    return whole_usage;
}

double BillProcess::calc_monthly_water_bill(int whole_usage, int whole_usage_max_hours)
{
    return (whole_usage - whole_usage_max_hours) + (whole_usage_max_hours * MAX_HOURS_COEFF);
}

void BillProcess::run()
{

    int msg_recieved_count = 0;
    for (;;)
    {
        string building_data = bill_server->receive();
        if (building_data != "")
        {

            decode_building_data(building_data);
            log_to_terminal(BILL_CENTER_RECIEVED_LOG + client + " !", Recieve);
            vector<MonthBill>
                bills = create_bills();
            string encoded_bills = month_bill_encoder(bills);
            bill_client->send(encoded_bills);
            log_to_terminal(BILL_CENTER_SEND_LOG + client + " !", Send);
            msg_recieved_count++;
        }
        if (msg_recieved_count == buildings_count)
            return;
    }
}