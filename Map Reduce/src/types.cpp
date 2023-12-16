#include "types.hpp"

int argmax(const vector<int> &numbers)
{
    int maxIndex = 0;
    int maxValue = numbers[0];

    for (int i = 1; i < int(numbers.size()); i++)
    {
        if (numbers[i] > maxValue)
        {
            maxValue = numbers[i];
            maxIndex = i;
        }
    }

    return maxIndex;
}

int argmin(const vector<int> &numbers)
{
    int minIndex = 0;
    int minValue = numbers[0];

    for (int i = 1; i < int(numbers.size()); i++)
    {
        if (numbers[i] < minValue)
        {
            minValue = numbers[i];
            minIndex = i;
        }
    }

    return minIndex;
}

string month_record_encoder(vector<MonthRecord> &records)
{
    string encoded_records;
    for (int i = 0; i < int(records.size()); i++)
    {
        encoded_records += to_string(records[i].year) + NUMBER_SEPERATOR +
                           to_string(records[i].month) + NUMBER_SEPERATOR +
                           to_string(records[i].whole_usage) + NUMBER_SEPERATOR +
                           to_string(records[i].average_usage) + NUMBER_SEPERATOR +
                           to_string(records[i].max_usage_hour) + NUMBER_SEPERATOR +
                           to_string(records[i].whole_usage_in_max_hours) + NUMBER_SEPERATOR +
                           to_string(records[i].difference_from_average_in_max_hours) + NUMBER_SEPERATOR +
                           to_string(records[i].whole_usage_in_min_hours);
        if (i != int(records.size()) - 1)
            encoded_records += ARRAY_SEPERATOR;
    }
    return encoded_records;
}

vector<map<string, int>> month_record_decoder(string &encoded_string)
{
    vector<map<string, int>> decoded_records;
    stringstream stream(encoded_string);
    string record_string;
    while (getline(stream, record_string, ARRAY_SEPERATOR))
    {
        int i = 0;
        map<string, int> record;
        stringstream number_stream(record_string);
        string word;
        while (getline(number_stream, word, NUMBER_SEPERATOR))
        {
            switch (i)
            {
            case 0:
                record["year"] = stoi(word);
                break;
            case 1:
                record["month"] = stoi(word);
                break;
            case 2:
                record[WHOLE_USAGE_CMD] = stoi(word);
                break;
            case 3:
                record[AVG_USAGE_CMD] = stod(word);
                break;
            case 4:
                record[MAX_HOUR_CMD] = stoi(word);
                break;
            case 5:
                record[USAGE_IN_MAX_HOURS_CMD] = stoi(word);
                break;
            case 6:
                record[DIFF_MAX_AVG_CMD] = stoi(word);
                break;
            case 7:
                record[USAGE_IN_MIN_HOURS_CMD] = stoi(word);
                break;

            default:
                break;
            }
            i++;
        }
        decoded_records.push_back(record);
    }
    return decoded_records;
}

string month_bill_encoder(vector<MonthBill> &bills)
{
    string encoded_bills;
    for (int i = 0; i < int(bills.size()); i++)
    {
        encoded_bills += to_string(bills[i].year) + NUMBER_SEPERATOR + to_string(bills[i].month) + NUMBER_SEPERATOR +
                         to_string(bills[i].elec_bill) + NUMBER_SEPERATOR + to_string(bills[i].gas_bill) + NUMBER_SEPERATOR +
                         to_string(bills[i].water_bill);
        if (i != int(bills.size()) - 1)
            encoded_bills += ARRAY_SEPERATOR;
    }
    return encoded_bills;
}

vector<map<string, double>> month_bill_decoder(string &encoded_string)
{
    vector<map<string, double>> decoded_bills;
    stringstream stream(encoded_string);
    string record_string;
    while (getline(stream, record_string, ARRAY_SEPERATOR))
    {
        int i = 0;
        map<string, double> bill;
        stringstream number_stream(record_string);
        string word;
        while (getline(number_stream, word, NUMBER_SEPERATOR))
        {
            switch (i)
            {
            case 0:
                bill["year"] = stoi(word);
                break;
            case 1:
                bill["month"] = stoi(word);
                break;
            case 2:
                bill[ELECTRICITY_CMD] = stod(word);
                break;
            case 3:
                bill[GAS_CMD] = stod(word);
                break;
            case 4:
                bill[WATER_CMD] = stod(word);
                break;
            default:
                break;
            }
            i++;
        }
        decoded_bills.push_back(bill);
    }
    return decoded_bills;
}

void main_cmd_decoder(string cmd, vector<string> &resources, string &report_parameter)
{
    stringstream stream(cmd);
    string sentence;
    int i = 0;
    while (getline(stream, sentence, ARRAY_SEPERATOR))
    {
        if (i == 0)
        {
            stringstream resource_stream(sentence);
            string resource;
            while (getline(resource_stream, resource, USER_INPUT_SEPERATOR))
            {
                resources.push_back(resource);
            }
        }
        else if (i == 1)
        {
            report_parameter = sentence;
        }
        i++;
    }
}

// void print_to_terminal(const string &message, LogType type)
// {
//     string log_message = "";
//     if (type == Warning)
//     {
//         log_message = YELLOW_COLOR + log_message + RESET_COLOR;
//     }
//     else if (type == Error)
//     {
//         log_message = RED_COLOR + log_message + RESET_COLOR;
//     }
//     else if (type == Prompt)
//     {
//         log_message = CYAN_COLOR + log_message + RESET_COLOR;
//     }

//     cout << log_message << endl;
// }
