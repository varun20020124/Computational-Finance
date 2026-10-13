#ifndef BLACK_SCHOLES_MODEL_H
#define BLACK_SCHOLES_MODEL_H

#include "Option.h"

class BlackScholesModel
{
private:
    Option option;

    double normalCDF(double x) const;

public:
    BlackScholesModel(const Option& option);

    double price(
        double S,
        double r,
        double tau,
        double sigma
    ) const;

    double delta(
        double S,
        double r,
        double tau,
        double sigma
    ) const;

    double impliedVolatility(
        double S,
        double r,
        double tau,
        double market_price
    ) const;
};

#endif