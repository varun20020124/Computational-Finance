#ifndef OPTION_H
#define OPTION_H

class Option
{
private:
    void init();

    double K;
    double S;
    double r;
    double T;
    double sigma;

public:
    Option();

    Option(
        double strike_price,
        double underlying_price,
        double risk_free_rate,
        double time_to_maturity,
        double volatility
    );

    ~Option();

    double getK() const;
    double getS() const;
    double getR() const;
    double getT() const;
    double getSigma() const;
};

#endif