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

double interpolateRateForTime(
    double time,
    const vector<double>& maturities,
    const vector<double>& interest_rates)
{
    if (maturities.empty() ||
        maturities.size() != interest_rates.size())
    {
        return 0.0;
    }

    const double tolerance = 1e-10;

    for (size_t i = 0; i < maturities.size(); i++)
    {
        if (fabs(maturities[i] - time) < tolerance)
        {
            return interest_rates[i];
        }
    }

    bool lower_found = false;
    bool upper_found = false;

    double lower_time = 0.0;
    double upper_time = 0.0;

    double lower_rate = 0.0;
    double upper_rate = 0.0;

    for (size_t i = 0; i < maturities.size(); i++)
    {
        if (maturities[i] < time)
        {
            if (!lower_found ||
                maturities[i] > lower_time)
            {
                lower_time = maturities[i];
                lower_rate = interest_rates[i];
                lower_found = true;
            }
        }

        if (maturities[i] > time)
        {
            if (!upper_found ||
                maturities[i] < upper_time)
            {
                upper_time = maturities[i];
                upper_rate = interest_rates[i];
                upper_found = true;
            }
        }
    }

    if (!lower_found && upper_found)
    {
        return upper_rate;
    }

    if (lower_found && !upper_found)
    {
        return lower_rate;
    }

    if (!lower_found && !upper_found)
    {
        return 0.0;
    }

    double weight =
        (time - lower_time) /
        (upper_time - lower_time);

    return lower_rate +
           weight * (upper_rate - lower_rate);
}


double yearFraction(
    int start_month,
    int start_day,
    int start_year,
    int end_month,
    int end_day,
    int end_year)
{
    int month_difference =
        (end_year - start_year) * 12 +
        (end_month - start_month);

    double years =
        month_difference / 12.0;

    years +=
        (end_day - start_day) / 365.0;

    return years;
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
    cout << endl;
cout << "Task 3: Arithmetic-Average Security" << endl;

/*
    Underlying bond:

    Issued:      01/01/2010
    Maturity:    01/01/2020
    Face value:  100
    Coupon rate: 5%
    Frequency:   0.5 years
*/

Bond underlying_bond(
    "01/01/2020",
    0.5,
    0.05);

vector<string> valuation_dates;

valuation_dates.push_back("01/01/2016");
valuation_dates.push_back("01/01/2017");
valuation_dates.push_back("01/01/2018");
valuation_dates.push_back("01/01/2019");
valuation_dates.push_back("01/01/2020");

vector<double> bond_prices;

double price_sum = 0.0;

cout << fixed << setprecision(6);

for (size_t i = 0;
     i < valuation_dates.size();
     i++)
{
    double price =
        underlying_bond.Price(
            100.0,
            valuation_dates[i],
            maturities,
            interest_rates);

    bond_prices.push_back(price);

    price_sum += price;

    cout << valuation_dates[i]
         << " bond price: "
         << price
         << endl;
}

/*
    Arithmetic average of the five
    observed bond prices.
*/

double average_price =
    price_sum / bond_prices.size();

cout << "Arithmetic-average payoff: "
     << average_price
     << endl;

/*
    The payoff is made on 12/31/2020.

    Discount the payoff back to the
    purchase date of 08/03/2015.
*/

double payoff_time =
    yearFraction(
        8, 3, 2015,
        12, 31, 2020);

double discount_rate =
    interpolateRateForTime(
        payoff_time,
        maturities,
        interest_rates);

double discount_factor =
    exp(
        -discount_rate *
        payoff_time);

double fair_value =
    average_price *
    discount_factor;

double purchase_price = 98.0;

cout << "Time from purchase to payoff: "
     << payoff_time
     << " years"
     << endl;

cout << "Interpolated discount rate: "
     << discount_rate
     << endl;

cout << "Discount factor: "
     << discount_factor
     << endl;

cout << "Fair value on 08/03/2015: "
     << fair_value
     << endl;

cout << "Purchase price: "
     << purchase_price
     << endl;

cout << "Fair value minus purchase price: "
     << fair_value - purchase_price
     << endl;

if (fair_value > purchase_price)
{
    cout << "Conclusion: The security was purchased "
         << "below its calculated fair value, so the "
         << "investment was favorable under this "
         << "valuation model."
         << endl;
}
else if (fair_value < purchase_price)
{
    cout << "Conclusion: The security was purchased "
         << "above its calculated fair value, so the "
         << "investment was unfavorable under this "
         << "valuation model."
         << endl;
}
else
{
    cout << "Conclusion: The purchase price equals "
         << "the calculated fair value."
         << endl;
}
    return 0;
}