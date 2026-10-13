#ifndef MARKET_DATA_H
#define MARKET_DATA_H

#include <map>
#include <string>
#include <vector>

struct MarketDataPoint
{
    std::string date;
    double stock_price;
    double option_price;
    double risk_free_rate;
};

class MarketData
{
private:
    std::map<std::string, double> interest_rates;
    std::map<std::string, double> stock_prices;

    std::string option_filename;

    void loadInterestRates(
        const std::string& filename
    );

    void loadStockPrices(
        const std::string& filename
    );

public:
    MarketData(
        const std::string& interest_file,
        const std::string& stock_file,
        const std::string& option_file
    );

    std::vector<MarketDataPoint>
    getOptionSeries(
        const std::string& start_date,
        const std::string& end_date,
        const std::string& expiration_date,
        double strike,
        char option_flag
    ) const;

    int tradingDaysBetween(
        const std::string& start_date,
        const std::string& end_date
    ) const;

    double timeToMaturity(
        const std::string& current_date,
        const std::string& expiration_date
    ) const;
};

#endif