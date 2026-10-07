#ifndef PRICING_METHOD_H
#define PRICING_METHOD_H

struct PricingResult
{
    double price;
    double delta;
};

class Pricing_Method
{
public:
    virtual PricingResult BSM_Pricer() const = 0;

    virtual PricingResult Binomial_Pricer() const = 0;

    virtual ~Pricing_Method()
    {
    }
};

#endif