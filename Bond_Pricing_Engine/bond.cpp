#include "bond.h"
#include <sstream>

Bond::Bond()
    : expiration_date("0"), frequency(0.0), coupon_rate(0.0)
{
}

Bond::~Bond()
{
}

Bond::Bond(const Bond& other)
    : expiration_date(other.expiration_date),
      frequency(other.frequency),
      coupon_rate(other.coupon_rate)
{
}

Bond::Bond(std::string expiration_date, double frequency, double coupon_rate)
    : expiration_date(expiration_date),
      frequency(frequency),
      coupon_rate(coupon_rate)
{
}

std::string Bond::ToString() const
{
    std::ostringstream output;
    output << "Bond("
           << expiration_date << ", "
           << frequency << ", "
           << coupon_rate << ")";
    return output.str();
}