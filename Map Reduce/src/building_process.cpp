#include "building_process.hpp"

BuildingProcess::BuildingProcess(string path_, int write_pipe_fd_, int read_pipe_fd_, vector<vector<int>> *resources_read_pipes_)
{
    path = path_;
    name = get_building_name(path);
    write_pipe_fd = write_pipe_fd_;
    read_pipe_fd = read_pipe_fd_;
    resources_read_pipes = resources_read_pipes_;
    building_server = make_shared<NamedPipeServer>(name);
    building_client = make_shared<NamedPipeClient>(BILL_SERVER);
}

BuildingProcess::~BuildingProcess()
{
    for (int i = 0; i < int(resources_read_pipes->size()); i++)
    {
        close_fd((*resources_read_pipes)[i][0]);
    }

    close_fd(write_pipe_fd);
    close_fd(read_pipe_fd);
    NamedPipe::remove_pipe(name);
}

string BuildingProcess::gather_resource_data()
{
    string encoded_data = "";
    for (int i = 0; i < RESOURCES_COUNT; i++)
    {
        string data = read_fd((*resources_read_pipes)[i][0]);
        string resource_name;
        if (Resource(i) == Electricity)
        {
            resource_records_map[ELECTRICITY_CMD] = month_record_decoder(data);
            resource_name = ELECTRICITY;
        }
        else if (Resource(i) == Water)
        {
            resource_records_map[WATER_CMD] = month_record_decoder(data);
            resource_name = WATER;
        }
        else if (Resource(i) == Gas)
        {
            resource_records_map[GAS_CMD] = month_record_decoder(data);
            resource_name = GAS;
        }
        log_to_terminal(path + BUILDING_RECIEVED_RESOURCE_RECORD_LOG + resource_name + " record !", Recieve);
        encoded_data += (data + ELEMENT_SEPERATOR);
    }
    encoded_data.pop_back();
    return encoded_data;
}

string BuildingProcess::get_main_command()
{
    string cmd;
    for (;;)
    {
        cmd = read_fd(read_pipe_fd);
        if (cmd != "")
        {
            break;
        }
    }
    log_to_terminal(path + BUILDING_RECIEVED_MAIN_LOG, Recieve);
    return cmd;
}

void BuildingProcess::get_bills()
{
    const string resources[3] = {ELECTRICITY, WATER, GAS};
    string encoded_bills;
    for (;;)
    {
        encoded_bills = building_server->receive();
        if (encoded_bills != "")
        {
            break;
        }
    }
    bills_map = month_bill_decoder(encoded_bills);
    log_to_terminal(path + BUILDING_RECIEVED_BILL_LOG, Recieve);
}

void BuildingProcess::run()
{
    string encoded_data = gather_resource_data();
    building_client->send(name + ELEMENT_SEPERATOR + encoded_data);
    log_to_terminal(path + BUILDING_SEND_LOG, Send);
    get_bills();
    string cmd = get_main_command();
    if (cmd == KILL_PROCESS_MSG)
        return;
    string report_parameter;
    vector<string> resources;
    main_cmd_decoder(cmd, resources, report_parameter);
    string building_record = create_record_building(report_parameter, resources);
    write_fd(building_record.c_str(), building_record.size(), write_pipe_fd);
    log_to_terminal(path + BUILDING_SEND_MAIN_LOG, Send);
}

template <typename T>
string BuildingProcess::monthly_report_generator(vector<map<string, T>> &records, string report_parameter, string parameter_tag)
{
    string report = "";
    for (int i = 0; i < int(records.size()); i++)
    {
        report += colorify_text("year : ", Prompt) + to_string(int(records[i]["year"])) + "\n";
        report += colorify_text("month : ", Prompt) + to_string(int(records[i]["month"])) + "\n";
        report += colorify_text(parameter_tag + " : ", Prompt) + to_string(records[i][report_parameter]) + "\n\n";
    }
    return report;
}

string BuildingProcess::create_record_building(string report_parameter,
                                               vector<string> resources)
{
    string report = "";

    for (int i = 0; i < int(resources.size()); i++)
    {
        report += colorify_text("-- " + resources[i] + " --\n", Error);
        if (report_parameter == BILL_CMD)
        {
            report += monthly_report_generator(bills_map, resources[i], BILL_TAG);
        }
        else
        {
            report += monthly_report_generator(resource_records_map[resources[i]], report_parameter, report_parameter);
        }
    }

    return report;
}

string BuildingProcess::get_building_name(string path)
{
    stringstream stream(path);
    string sentence;
    int i = 0;
    while (getline(stream, sentence, '/'))
    {
        if (i == 2)
            return sentence;
        i++;
    }
}
