#ifndef OPTION_PRICE_H
#define OPTION_PRICE_H

#include "Option.h"
#include "pricing_method.h"

class Option_Price :
    public Option,
    public Pricing_Method
{
public:
    char flag;

    Option_Price(
        double strike_price,
        double underlying_price,
        double risk_free_rate,
        double time_to_maturity,
        double volatility,
        char option_flag
    );

    ~Option_Price();

    PricingResult BSM_Pricer() const override;

    PricingResult Binomial_Pricer() const override;
};

#endif