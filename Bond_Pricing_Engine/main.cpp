#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <iomanip>
#include "bond.h"

using namespace std;

bool loadInterestRateCurve(
    string filename,
    vector<double>& maturities,
    vector<double>& interest_rates)
{
    ifstream infile(filename.c_str());

    if (!infile.is_open())
    {
        return false;
    }

    string line;

    // Skip CSV header.
    getline(infile, line);

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

        string maturity_text = line.substr(0, comma);
        string rate_text = line.substr(comma + 1);

        try
        {
            double maturity = stod(maturity_text);
            double rate = stod(rate_text);

            maturities.push_back(maturity);
            interest_rates.push_back(rate);
        }
        catch (...)
        {
            continue;
        }
    }

    infile.close();

    return !maturities.empty() &&
           maturities.size() == interest_rates.size();
}

bool approximatelyEqual(
    double first,
    double second,
    double tolerance = 1e-6)
{
    return fabs(first - second) < tolerance;
}

void runUnitTests()
{
    int passed = 0;
    int total = 0;

    vector<double> test_maturities;

    test_maturities.push_back(0.0);
    test_maturities.push_back(0.25);
    test_maturities.push_back(0.5);
    test_maturities.push_back(1.0);
    test_maturities.push_back(2.0);

    vector<double> zero_rates(
        test_maturities.size(),
        0.0);

    /*
        Test 1

        One-year semiannual 6% bond.

        At zero interest rates:

        coupon = 100 * 0.06 * 0.5 = 3

        Cash flows:
            0.5 years -> 3
            1.0 years -> 103

        Price = 106
    */

    total++;

    Bond test_bond(
        "01/01/2027",
        0.5,
        0.06);

    double price =
        test_bond.Price(
            100.0,
            "01/01/2026",
            test_maturities,
            zero_rates);

    if (approximatelyEqual(price, 106.0))
    {
        passed++;
    }
    else
    {
        cout << "Unit test 1 failed. Expected 106, got "
             << price << endl;
    }

    /*
        Test 2

        Valuation date: 01/01/2026
        Maturity:       04/01/2026

        This is a 0.25-year stub.

        coupon =
            100 * 0.06 * 0.25
            = 1.5

        Price at zero rates =
            100 + 1.5
            = 101.5
    */

    total++;

    Bond stub_bond(
        "04/01/2026",
        0.5,
        0.06);

    double stub_price =
        stub_bond.Price(
            100.0,
            "01/01/2026",
            test_maturities,
            zero_rates);

    if (approximatelyEqual(stub_price, 101.5))
    {
        passed++;
    }
    else
    {
        cout << "Unit test 2 failed. Expected 101.5, got "
             << stub_price << endl;
    }

    /*
        Test 3

        Homework convention:
        price at maturity = par value.
    */

    total++;

    Bond maturity_bond(
        "01/01/2026",
        0.5,
        0.06);

    double maturity_price =
        maturity_bond.Price(
            100.0,
            "01/01/2026",
            test_maturities,
            zero_rates);

    if (approximatelyEqual(
            maturity_price,
            100.0))
    {
        passed++;
    }
    else
    {
        cout << "Unit test 3 failed. Expected 100, got "
             << maturity_price << endl;
    }

    cout << "Unit tests passed: "
         << passed
         << "/"
         << total
         << endl;
}

int main()
{
    /*
        Task 1
    */

    Bond default_bond;

    cout << "Default Bond:" << endl;
    cout << default_bond.ToString() << endl;

    Bond bond_2035(
        "11/19/2035",
        0.5,
        0.07);

    cout << endl;

    cout << "2035 Bond:" << endl;
    cout << bond_2035.ToString() << endl;

    /*
        Task 2 Unit Tests
    */

    cout << endl;
    cout << "Running pricing unit tests:" << endl;

    runUnitTests();

    /*
        Load provided interest-rate curve.
    */

    vector<double> maturities;
    vector<double> interest_rates;

    if (!loadInterestRateCurve(
            "Bond_Ex3.csv",
            maturities,
            interest_rates))
    {
        cout << "Error: could not load Bond_Ex3.csv"
             << endl;

        return 1;
    }

    cout << endl;

    cout << "Interest-rate curve loaded successfully."
         << endl;

    cout << "Number of observations: "
         << maturities.size()
         << endl;

    return 0;
}