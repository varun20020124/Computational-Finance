#include "Option_Price.h"
#include <vector>
#include <cmath>
#include <stdexcept>

namespace
{
    double normalCDF(double x)
    {
        return 0.5 * std::erfc(-x / std::sqrt(2.0));
    }
}

Option_Price::Option_Price(
    double strike_price,
    double underlying_price,
    double risk_free_rate,
    double time_to_maturity,
    double volatility,
    char option_flag)
    : Option(
          strike_price,
          underlying_price,
          risk_free_rate,
          time_to_maturity,
          volatility),
      flag(option_flag)
{
}

Option_Price::~Option_Price()
{
}

PricingResult Option_Price::BSM_Pricer() const
{
    double K = getK();
    double S = getS();
    double r = getR();
    double T = getT();
    double sigma = getSigma();

    if (K <= 0.0 ||
        S <= 0.0 ||
        T <= 0.0 ||
        sigma <= 0.0)
    {
        throw std::invalid_argument(
            "K, S, T, and sigma must be positive.");
    }

    double d1 =
        (std::log(S / K) +
         (r + 0.5 * sigma * sigma) * T) /
        (sigma * std::sqrt(T));

    double d2 =
        d1 - sigma * std::sqrt(T);

    PricingResult result;

    if (flag == 'c' || flag == 'C')
    {
        result.price =
            S * normalCDF(d1) -
            K * std::exp(-r * T) * normalCDF(d2);

        result.delta =
            normalCDF(d1);
    }
    else if (flag == 'p' || flag == 'P')
    {
        result.price =
            K * std::exp(-r * T) * normalCDF(-d2) -
            S * normalCDF(-d1);

        result.delta =
            normalCDF(d1) - 1.0;
    }
    else
    {
        throw std::invalid_argument(
            "Option flag must be c/C or p/P.");
    }

    return result;
}

PricingResult Option_Price::Binomial_Pricer() const
{
    double K = getK();
    double S = getS();
    double r = getR();
    double T = getT();
    double sigma = getSigma();

    if (K <= 0.0 ||
        S <= 0.0 ||
        T <= 0.0 ||
        sigma <= 0.0)
    {
        throw std::invalid_argument(
            "K, S, T, and sigma must be positive.");
    }

    if (!(flag == 'c' || flag == 'C' ||
          flag == 'p' || flag == 'P'))
    {
        throw std::invalid_argument(
            "Option flag must be c/C or p/P.");
    }

    const int steps = 1000;

    double dt =
        T / steps;

    double u =
        std::exp(
            sigma * std::sqrt(dt));

    double d =
        1.0 / u;

    double p =
        (std::exp(r * dt) - d) /
        (u - d);

    if (p < 0.0 || p > 1.0)
    {
        throw std::invalid_argument(
            "Invalid risk-neutral probability.");
    }

    double discount =
        std::exp(-r * dt);

    std::vector<double> option_values(
        steps + 1);

    /*
        Calculate terminal option payoffs.

        j represents the number of up moves.
    */

    for (int j = 0; j <= steps; j++)
    {
        double stock_price =
            S *
            std::pow(u, j) *
            std::pow(d, steps - j);

        if (flag == 'c' || flag == 'C')
        {
            double payoff =
                stock_price - K;

            option_values[j] =
                payoff > 0.0 ? payoff : 0.0;
        }
        else
        {
            double payoff =
                K - stock_price;

            option_values[j] =
                payoff > 0.0 ? payoff : 0.0;
        }
    }

    double value_up = 0.0;
    double value_down = 0.0;

    /*
        Work backward through the tree.
    */

    for (int step = steps - 1;
         step >= 0;
         step--)
    {
        for (int j = 0; j <= step; j++)
        {
            option_values[j] =
                discount *
                (
                    p * option_values[j + 1] +
                    (1.0 - p) * option_values[j]
                );
        }

        /*
            When step == 1, the tree contains
            the two option values one period
            after today.

            These are used for Delta.
        */

        if (step == 1)
        {
            value_down =
                option_values[0];

            value_up =
                option_values[1];
        }
    }

    double stock_up =
        S * u;

    double stock_down =
        S * d;

    PricingResult result;

    result.price =
        option_values[0];

    result.delta =
        (value_up - value_down) /
        (stock_up - stock_down);

    return result;
}