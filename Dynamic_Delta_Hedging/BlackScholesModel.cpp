#include "BlackScholesModel.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

BlackScholesModel::BlackScholesModel(
    const Option& option)
    : option(option)
{
}

double BlackScholesModel::normalCDF(
    double x) const
{
    return 0.5 *
           std::erfc(
               -x / std::sqrt(2.0));
}

double BlackScholesModel::price(
    double S,
    double r,
    double tau,
    double sigma) const
{
    double K =
        option.getStrikePrice();

    if (S <= 0.0 || K <= 0.0)
    {
        throw std::invalid_argument(
            "Stock price and strike price must be positive.");
    }

    /*
        At expiration, the option value
        equals its intrinsic value.
    */

    if (tau <= 0.0)
    {
        if (option.isCall())
        {
            return std::max(
                S - K,
                0.0);
        }

        if (option.isPut())
        {
            return std::max(
                K - S,
                0.0);
        }

        throw std::invalid_argument(
            "Invalid option flag.");
    }

    if (sigma <= 0.0)
    {
        throw std::invalid_argument(
            "Volatility must be positive.");
    }

    double d1 =
        (
            std::log(S / K) +
            (
                r +
                0.5 * sigma * sigma
            ) * tau
        ) /
        (
            sigma *
            std::sqrt(tau)
        );

    double d2 =
        d1 -
        sigma *
        std::sqrt(tau);

    if (option.isCall())
    {
        return
            S * normalCDF(d1) -
            K *
            std::exp(-r * tau) *
            normalCDF(d2);
    }

    if (option.isPut())
    {
        return
            K *
            std::exp(-r * tau) *
            normalCDF(-d2) -
            S *
            normalCDF(-d1);
    }

    throw std::invalid_argument(
        "Invalid option flag.");
}

double BlackScholesModel::delta(
    double S,
    double r,
    double tau,
    double sigma) const
{
    double K =
        option.getStrikePrice();

    if (S <= 0.0 ||
        K <= 0.0 ||
        tau <= 0.0 ||
        sigma <= 0.0)
    {
        throw std::invalid_argument(
            "Invalid parameters for Delta calculation.");
    }

    double d1 =
        (
            std::log(S / K) +
            (
                r +
                0.5 * sigma * sigma
            ) * tau
        ) /
        (
            sigma *
            std::sqrt(tau)
        );

    if (option.isCall())
    {
        return normalCDF(d1);
    }

    if (option.isPut())
    {
        return normalCDF(d1) - 1.0;
    }

    throw std::invalid_argument(
        "Invalid option flag.");
}

double BlackScholesModel::impliedVolatility(
    double S,
    double r,
    double tau,
    double market_price) const
{
    if (S <= 0.0 ||
        tau <= 0.0 ||
        market_price < 0.0)
    {
        throw std::invalid_argument(
            "Invalid parameters for implied volatility.");
    }

    /*
        Binary-search range for sigma.

        0.000001 = effectively zero volatility
        5.0      = 500% annualized volatility
    */

    double low = 0.000001;
    double high = 5.0;

    const double tolerance = 1e-8;
    const int max_iterations = 200;

    double low_price =
        price(S, r, tau, low);

    double high_price =
        price(S, r, tau, high);

    /*
        If the market price cannot be produced
        within the volatility search interval,
        implied volatility is not valid.
    */

    if (market_price < low_price - tolerance ||
        market_price > high_price + tolerance)
    {
        throw std::invalid_argument(
            "Market price outside valid implied-volatility range.");
    }

    for (int i = 0;
         i < max_iterations;
         i++)
    {
        double mid =
            0.5 * (low + high);

        double model_price =
            price(
                S,
                r,
                tau,
                mid);

        if (std::fabs(
                model_price -
                market_price) < tolerance)
        {
            return mid;
        }

        if (model_price >
            market_price)
        {
            high = mid;
        }
        else
        {
            low = mid;
        }
    }

    return
        0.5 * (low + high);
}