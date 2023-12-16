#include "building_process.hpp"

int main(int argc, char const *argv[])
{
    if (argc != 4)
    {
        exit(EXIT_FAILURE);
    }

    string building_dir_path = argv[1];
    vector<vector<int>> resources_pipes;
    string resource_types[] = {ELECTRICITY, WATER, GAS};
    log_to_terminal(building_dir_path + BUILDING_PROCESS_CREATED_LOG, Prompt);

    for (int i = 0; i < RESOURCES_COUNT; i++)
    {
        resources_pipes.push_back(create_pipe());
        int pid = fork();
        if (pid == -1)
        {
            log_to_terminal(building_dir_path + FAILED_TO_FORK_ERROR + resource_types[i], Error);
        }
        else if (!pid)
        {
            close_fd(resources_pipes[i][0]);
            execl(PATH_TO_RESOURCE_PROGRAM, PATH_TO_RESOURCE_PROGRAM, (building_dir_path + "/" + resource_types[i]).c_str(), to_string(resources_pipes[i][1]).c_str(), to_string(int(Resource(i))).c_str(), nullptr);
            log_to_terminal(building_dir_path + "/" + resource_types[i] + FAILED_TO_FORK_ERROR, Error);
            return 1;
        }
        else
            close_fd(resources_pipes[i][1]);
    }

    for (int i = 0; i < RESOURCES_COUNT; i++)
    {
        wait(NULL);
    }

    BuildingProcess building = BuildingProcess(building_dir_path, atoi(argv[2]), atoi(argv[3]), &resources_pipes);
    building.run();
}
