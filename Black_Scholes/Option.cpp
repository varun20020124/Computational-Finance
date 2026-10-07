#include "Option.h"

void Option::init()
{
    K = 0.0;
    S = 0.0;
    r = 0.0;
    T = 0.0;
    sigma = 0.0;
}

Option::Option()
{
    init();
}

Option::Option(
    double strike_price,
    double underlying_price,
    double risk_free_rate,
    double time_to_maturity,
    double volatility)
    : K(strike_price),
      S(underlying_price),
      r(risk_free_rate),
      T(time_to_maturity),
      sigma(volatility)
{
}

Option::~Option()
{
}

double Option::getK() const
{
    return K;
}

double Option::getS() const
{
    return S;
}

double Option::getR() const
{
    return r;
}

double Option::getT() const
{
    return T;
}

double Option::getSigma() const
{
    return sigma;
}