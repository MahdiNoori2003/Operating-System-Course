#include "main_utils.hpp"

MainUtils::MainUtils(string buildings_dir_path_)
{
    buildings_dir_path = buildings_dir_path_;
}

MainUtils::~MainUtils()
{
    for (int i = 0; i < int(main_read_pipes.size()); i++)
    {
        close_fd(main_read_pipes[i][0]);
        close_fd(main_write_pipes[i][1]);
    }
}

void MainUtils::create_building_names_map()
{
    int folder_count = 0;

    for (const auto &entry : filesystem::directory_iterator(buildings_dir_path))
    {

        if (entry.is_directory())
        {
            main_read_pipes.push_back(create_pipe());
            main_write_pipes.push_back(create_pipe());
            string building_name = entry.path().filename().string();
            building_names_map[building_name] = folder_count;
            ++folder_count;
            log_to_terminal(building_name + MAIN_BUILDING_LOG + to_string(folder_count), Warning);
        }
    }
}

void MainUtils::make_buildings()
{
    map<string, int>::iterator it = building_names_map.begin();
    while (it != building_names_map.end())
    {
        int pid = fork();

        if (pid == -1)
        {
            log_to_terminal("main" + FAILED_TO_FORK_ERROR + it->first, Error);
        }
        else if (!pid)
        {

            close_fd(main_read_pipes[it->second][0]);
            close_fd(main_write_pipes[it->second][1]);

            execl(PATH_TO_BULDING_PROGRAM, PATH_TO_BULDING_PROGRAM, (buildings_dir_path + "/" + it->first).c_str(),
                  to_string(main_read_pipes[it->second][1]).c_str(), to_string(main_write_pipes[it->second][0]).c_str(), nullptr);

            log_to_terminal(it->first + FAILED_TO_EXEC_ERROR, Error);
            return;
        }
        else
        {
            close_fd(main_read_pipes[it->second][1]);
            close_fd(main_write_pipes[it->second][0]);
        }
        ++it;
    }
}

void MainUtils::make_bill_center()
{
    int pid = fork();

    if (pid == -1)
    {
        log_to_terminal("main" + FAILED_TO_FORK_ERROR + BILL_TAG, Error);
    }
    else if (!pid)
    {
        execl(PATH_TO_BILL_PROGRAM, PATH_TO_BILL_PROGRAM, to_string(main_read_pipes.size()).c_str(), nullptr);
        log_to_terminal(BILL_TAG + FAILED_TO_EXEC_ERROR, Error);
        return;
    }
}

void MainUtils::get_user_request(string &buidings_requested, string &resources_requested,
                                 string &report_parameter_requested)
{
    cout << colorify_text(POINTER, Pointer) << GET_BUILDINGS_PROMPT << endl;
    getline(cin, buidings_requested);
    cout << colorify_text(POINTER, Pointer) << GET_RESOURCES_PROMPT << endl;
    getline(cin, resources_requested);
    cout << colorify_text(POINTER, Pointer) << GET_REPORT_PROMPT << endl;
    getline(cin, report_parameter_requested);
}

void MainUtils::send_user_request_to_buildings(string &buidings_requested,
                                               string &resources_requested, string &report_parameter_requested)
{
    map<string, int>::iterator it = building_names_map.begin();
    while (it != building_names_map.end())
    {
        stringstream stream(buidings_requested);
        string building;
        bool found = false;
        while (getline(stream, building, USER_INPUT_SEPERATOR))
        {
            if (it->first == building)
            {
                found = true;
                string msg = resources_requested + ARRAY_SEPERATOR + report_parameter_requested;
                write_fd(msg.c_str(), msg.size(), main_write_pipes[it->second][1]);
            }
        }
        if (!found)
            write_fd(KILL_PROCESS_MSG, strlen(KILL_PROCESS_MSG), main_write_pipes[it->second][1]);

        ++it;
    }

    log_to_terminal(MAIN_SEND_LOG, Send);
}

void MainUtils::recieve_reports()
{
    map<string, int>::iterator it = building_names_map.begin();
    cout << endl;
    while (it != building_names_map.end())
    {
        string data = read_fd(main_read_pipes[it->second][0]);
        if (data != "")
        {
            cout << colorify_text("== Building: " + it->first + " ==", Warning) << endl;
            cout << data << endl;
        }
        ++it;
    }
}

void MainUtils::run()
{
    create_building_names_map();
    string buidings_requested;
    string resources_requested;
    string report_parameter_requested;
    get_user_request(buidings_requested, resources_requested, report_parameter_requested);
    make_bill_center();
    make_buildings();
    send_user_request_to_buildings(buidings_requested, resources_requested, report_parameter_requested);
    for (int i = 0; i < int(main_read_pipes.size()) + 1; i++)
        wait(NULL);
    recieve_reports();
}