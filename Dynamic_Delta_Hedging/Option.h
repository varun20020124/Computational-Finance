#ifndef OPTION_H
#define OPTION_H

#include <string>

class Option
{
private:
    double strike_price;
    std::string expiration_date;
    char flag;

public:
    Option();

    Option(
        double strike_price,
        const std::string& expiration_date,
        char flag
    );

    ~Option();

    double getStrikePrice() const;
    std::string getExpirationDate() const;
    char getFlag() const;

    bool isCall() const;
    bool isPut() const;
};

#endif