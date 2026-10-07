#include <iostream>
#include <iomanip>
#include <stdexcept>

#include "Option_Price.h"
#include "test.h"

using namespace std;

int main()
{
    /*
        Run unit tests first.
    */

    cout << "Running unit tests:" << endl;
    runUnitTests();

    cout << endl;

    /*
        User input.
    */

    double K;
    double S;
    double r;
    double T;
    double sigma;
    char flag;

    cout << "European Option Pricing" << endl;
    cout << "-----------------------" << endl;

    cout << "Enter strike price K: ";
    cin >> K;

    cout << "Enter current underlying price S: ";
    cin >> S;

    cout << "Enter risk-free rate r: ";
    cin >> r;

    cout << "Enter time to maturity T: ";
    cin >> T;

    cout << "Enter volatility sigma: ";
    cin >> sigma;

    cout << "Enter option type "
         << "(c/C for call, p/P for put): ";
    cin >> flag;

    /*
        Basic input validation.
    */

    if (cin.fail())
    {
        cout << "Error: invalid numeric input." << endl;
        return 1;
    }

    try
    {
        Option_Price option(
            K,
            S,
            r,
            T,
            sigma,
            flag);

        PricingResult bsm_result =
            option.BSM_Pricer();

        PricingResult binomial_result =
            option.Binomial_Pricer();

        cout << fixed << setprecision(6);

        cout << endl;

        cout << "Black-Scholes-Merton:" << endl;

        cout << "Price: "
             << bsm_result.price
             << endl;

        cout << "Delta: "
             << bsm_result.delta
             << endl;

        cout << endl;

        cout << "Binomial Lattice:" << endl;

        cout << "Price: "
             << binomial_result.price
             << endl;

        cout << "Delta: "
             << binomial_result.delta
             << endl;

        cout << endl;

        cout << "Price difference "
             << "(Binomial - BSM): "
             << binomial_result.price -
                    bsm_result.price
             << endl;

        cout << "Delta difference "
             << "(Binomial - BSM): "
             << binomial_result.delta -
                    bsm_result.delta
             << endl;
    }
    catch (const invalid_argument& error)
    {
        cout << "Error: "
             << error.what()
             << endl;

        return 1;
    }

    return 0;
}