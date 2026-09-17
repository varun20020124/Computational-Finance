#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

double average(vector<double> v)
{
    if (v.empty())
    {
        return 0.0;
    }

    double total = 0.0;

    for (double x : v)
    {
        total += x;
    }

    return total / v.size();
}

double find_rate(vector<double> rate_vec, vector<string> date_vec, string date)
{
    for (size_t i = 0; i < date_vec.size() && i < rate_vec.size(); i++)
    {
        if (date_vec[i] == date)
        {
            return rate_vec[i];
        }
    }

    return -1.0;
}

int main()
{
    vector<double> rate;
    vector<string> date;

    ifstream infile("./hw1_H.15_Baa_Data.csv");

    // Make sure the file opened successfully
    if (!infile.is_open())
    {
        cout << "Error: could not open hw1_H.15_Baa_Data.csv" << endl;
        return 1;
    }

    string line;

    // Skip all metadata/header rows until "Time Period"
    while (getline(infile, line))
    {
        if (line.find("Time Period") != string::npos)
        {
            break;
        }
    }

    // Load dates and rates
    while (getline(infile, line))
    {
        if (line.empty())
        {
            continue;
        }

        size_t comma = line.find(',');

        if (comma == string::npos)
        {
            continue;
        }

        string date_value = line.substr(0, comma);
        string rate_value = line.substr(comma + 1);

        try
        {
            double rate_number = stod(rate_value);

            date.push_back(date_value);
            rate.push_back(rate_number);
        }
        catch (...)
        {
            // Ignore malformed data rows rather than crashing
            continue;
        }
    }

    infile.close();

    // Make sure data was actually loaded
    if (rate.empty() || date.empty())
    {
        cout << "Error: no data was loaded from the file." << endl;
        return 1;
    }

    double avg_rate = average(rate);

    string user_date;

    cout << "Enter a date in yyyy-mm format (Ctrl-D to exit):" << endl;

    while (cin >> user_date)
    {
        double current_rate = find_rate(rate, date, user_date);

        if (current_rate == -1.0)
        {
            cout << "The input date is not in the data." << endl;
        }
        else
        {
            cout << "Rate: " << current_rate << endl;

            cout << "Difference from average: "
                 << current_rate - avg_rate << endl;
        }

        cout << "Enter another date (Ctrl-D to exit):" << endl;
    }

    return 0;
}