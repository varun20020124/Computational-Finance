#ifndef BOND_H
#define BOND_H
#include <string>
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
};

#endif