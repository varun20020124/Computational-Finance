#include "DeltaHedger.h"

#include <cmath>
#include <fstream>
#include <stdexcept>

DeltaHedger::DeltaHedger(double dt)
    : dt(dt)
{
    if (dt <= 0.0)
    {
        throw std::invalid_argument(
            "Time step must be positive.");
    }
}


std::vector<double>
DeltaHedger::calculateBankBalances(
    const std::vector<double>& stock_prices,
    const std::vector<double>& option_prices,
    const std::vector<double>& deltas,
    double risk_free_rate) const
{
    std::size_t n = stock_prices.size();

    if (n == 0 ||
        option_prices.size() != n ||
        deltas.size() < n - 1)
    {
        throw std::invalid_argument(
            "Invalid hedge input dimensions.");
    }

    std::vector<double> bank(n, 0.0);

    /*
        Initial hedge:

        B0 = V0 - delta0 * S0
    */

    bank[0] =
        option_prices[0]
        - deltas[0] * stock_prices[0];

    /*
        Rebalance through the path.

        At the final maturity point we do
        not need another hedge position.
    */

    for (std::size_t i = 1;
         i < n - 1;
         i++)
    {
        double grown_bank =
            bank[i - 1] *
            std::exp(risk_free_rate * dt);

        double portfolio_value =
            deltas[i - 1] * stock_prices[i]
            + grown_bank;

        bank[i] =
            portfolio_value
            - deltas[i] * stock_prices[i];
    }

    return bank;
}


std::vector<double>
DeltaHedger::calculateHedgingErrors(
    const std::vector<double>& stock_prices,
    const std::vector<double>& option_prices,
    const std::vector<double>& deltas,
    double risk_free_rate) const
{
    std::size_t n = stock_prices.size();

    if (n == 0 ||
        option_prices.size() != n ||
        deltas.size() < n - 1)
    {
        throw std::invalid_argument(
            "Invalid hedge input dimensions.");
    }

    std::vector<double> bank =
        calculateBankBalances(
            stock_prices,
            option_prices,
            deltas,
            risk_free_rate);

    std::vector<double> hedging_errors(
        n,
        0.0);

    /*
        HE0 = 0 because the hedge is
        established exactly at t0.
    */

    hedging_errors[0] = 0.0;

    for (std::size_t i = 1;
         i < n;
         i++)
    {
        double grown_bank =
            bank[i - 1] *
            std::exp(risk_free_rate * dt);

        hedging_errors[i] =
            deltas[i - 1] * stock_prices[i]
            + grown_bank
            - option_prices[i];
    }

    return hedging_errors;
}


void DeltaHedger::writeTerminalErrorsCSV(
    const std::vector<double>& terminal_errors,
    const std::string& filename) const
{
    std::ofstream outfile(filename.c_str());

    if (!outfile.is_open())
    {
        throw std::runtime_error(
            "Could not open hedging-error output file.");
    }

    outfile << "path,terminal_hedging_error\n";

    for (std::size_t i = 0;
         i < terminal_errors.size();
         i++)
    {
        outfile
            << i
            << ","
            << terminal_errors[i]
            << "\n";
    }

    outfile.close();
}

std::vector<double>
DeltaHedger::calculateBankBalances(
    const std::vector<double>& stock_prices,
    const std::vector<double>& option_prices,
    const std::vector<double>& deltas,
    const std::vector<double>& risk_free_rates) const
{
    std::size_t n =
        stock_prices.size();

    if (n == 0 ||
        option_prices.size() != n ||
        deltas.size() != n ||
        risk_free_rates.size() != n)
    {
        throw std::invalid_argument(
            "Invalid variable-rate hedge input dimensions.");
    }

    std::vector<double> bank(
        n,
        0.0
    );

    /*
        Initial bank position:

        B0 = V0 - delta0*S0
    */

    bank[0] =
        option_prices[0]
        - deltas[0] * stock_prices[0];


    /*
        Daily rebalance.

        The project specifies that the cash
        position grows using r_(i-1).
    */

    for (std::size_t i = 1;
         i < n;
         i++)
    {
        double grown_bank =
            bank[i - 1]
            * std::exp(
                risk_free_rates[i - 1] * dt
            );

        double portfolio_value =
            deltas[i - 1] * stock_prices[i]
            + grown_bank;

        bank[i] =
            portfolio_value
            - deltas[i] * stock_prices[i];
    }

    return bank;
}


std::vector<double>
DeltaHedger::calculateHedgingErrors(
    const std::vector<double>& stock_prices,
    const std::vector<double>& option_prices,
    const std::vector<double>& deltas,
    const std::vector<double>& risk_free_rates) const
{
    std::size_t n =
        stock_prices.size();

    if (n == 0 ||
        option_prices.size() != n ||
        deltas.size() != n ||
        risk_free_rates.size() != n)
    {
        throw std::invalid_argument(
            "Invalid variable-rate hedge input dimensions.");
    }

    std::vector<double> bank =
        calculateBankBalances(
            stock_prices,
            option_prices,
            deltas,
            risk_free_rates
        );

    std::vector<double> hedging_errors(
        n,
        0.0
    );

    /*
        At t0 the hedge has just been
        established, so HE0 = 0.
    */

    hedging_errors[0] =
        0.0;

    for (std::size_t i = 1;
         i < n;
         i++)
    {
        double grown_bank =
            bank[i - 1]
            * std::exp(
                risk_free_rates[i - 1] * dt
            );

        hedging_errors[i] =
            deltas[i - 1] * stock_prices[i]
            + grown_bank
            - option_prices[i];
    }

    return hedging_errors;
}