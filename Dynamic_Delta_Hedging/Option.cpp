#include "Option.h"

Option::Option()
    : strike_price(0.0),
      expiration_date(""),
      flag('C')
{
}

Option::Option(
    double strike_price,
    const std::string& expiration_date,
    char flag)
    : strike_price(strike_price),
      expiration_date(expiration_date),
      flag(flag)
{
}

Option::~Option()
{
}

double Option::getStrikePrice() const
{
    return strike_price;
}

std::string Option::getExpirationDate() const
{
    return expiration_date;
}

char Option::getFlag() const
{
    return flag;
}

bool Option::isCall() const
{
    return flag == 'C' || flag == 'c';
}

bool Option::isPut() const
{
    return flag == 'P' || flag == 'p';
}