#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

double average(vector<double> v)
{
// code for calculating average of members of v
// and returning the average
}

double find_rate(vector<double> rate_vec, vector<string> date_vec, string date)
{
// code for finding the Baa rate for the
// given date (in yyyy-mm format)
// and returning the rate for that month
}

int main()
{
    vector<double> rate;
    vector<string> date;
    ifstream infile("./hw1_H.15_Baa_Data.csv");

// code for loading rate and date vectors from the file hw1_H.15_Baa_Data.csv
// the headers should be handled properly. do not delete them manually


   infile.close();

// code for prompting user for a date and returning the rate
// and the difference between the rate for that date and the
// average rate
//
// This code should allow the user to continue to input dates
// until the user inputs the EOF (End-of-file), namely control-d (in linux/mac) or control-z (in windows)
// This should not crash if a bad date is entered.

   return 0.0; // program end
}

