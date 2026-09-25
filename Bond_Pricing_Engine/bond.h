#ifndef BOND_H
#define BOND_H

#include <string>
#include <vector>

class Bond
{
private:
    std::string expiration_date;
    double frequency;
    double coupon_rate;

public:
    Bond();

    ~Bond();

    Bond(const Bond& other);

    Bond(std::string expiration_date, double frequency, double coupon_rate);

    std::string ToString() const;

    double Price(
        double face_value,
        std::string valuation_date,
        const std::vector<double>& maturities,
        const std::vector<double>& interest_rates
    ) const;
};

#endif