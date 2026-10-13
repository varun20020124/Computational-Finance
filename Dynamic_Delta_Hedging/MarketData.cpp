#include "MarketData.h"

#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>


MarketData::MarketData(
    const std::string& interest_file,
    const std::string& stock_file,
    const std::string& option_file)
    : option_filename(option_file)
{
    loadInterestRates(interest_file);
    loadStockPrices(stock_file);
}


void MarketData::loadInterestRates(
    const std::string& filename)
{
    std::ifstream infile(filename.c_str());

    if (!infile.is_open())
    {
        throw std::runtime_error(
            "Could not open interest-rate file.");
    }

    std::string line;

    // Skip header.
    std::getline(infile, line);

    while (std::getline(infile, line))
    {
        if (line.empty())
        {
            continue;
        }

        std::stringstream ss(line);

        std::string date;
        std::string rate_string;

        std::getline(ss, date, ',');
        std::getline(ss, rate_string, ',');

        double rate_percent =
            std::stod(rate_string);

        /*
            Example:

            0.29 in the CSV means 0.29%.

            Black-Scholes therefore uses:
            0.29 / 100 = 0.0029.
        */

        interest_rates[date] =
            rate_percent / 100.0;
    }
}


void MarketData::loadStockPrices(
    const std::string& filename)
{
    std::ifstream infile(filename.c_str());

    if (!infile.is_open())
    {
        throw std::runtime_error(
            "Could not open stock-price file.");
    }

    std::string line;

    // Skip header.
    std::getline(infile, line);

    while (std::getline(infile, line))
    {
        if (line.empty())
        {
            continue;
        }

        std::stringstream ss(line);

        std::string date;
        std::string price_string;

        std::getline(ss, date, ',');
        std::getline(ss, price_string, ',');

        stock_prices[date] =
            std::stod(price_string);
    }
}


std::vector<MarketDataPoint>
MarketData::getOptionSeries(
    const std::string& start_date,
    const std::string& end_date,
    const std::string& expiration_date,
    double strike,
    char option_flag) const
{
    std::ifstream infile(
        option_filename.c_str());

    if (!infile.is_open())
    {
        throw std::runtime_error(
            "Could not open option-price file.");
    }

    std::map<std::string, double>
        option_prices;

    std::string line;

    // Skip header.
    std::getline(infile, line);

    while (std::getline(infile, line))
    {
        if (line.empty())
        {
            continue;
        }

        std::stringstream ss(line);

        std::string date;
        std::string exdate;
        std::string flag_string;
        std::string strike_string;
        std::string bid_string;
        std::string offer_string;

        std::getline(ss, date, ',');
        std::getline(ss, exdate, ',');
        std::getline(ss, flag_string, ',');
        std::getline(ss, strike_string, ',');
        std::getline(ss, bid_string, ',');
        std::getline(ss, offer_string, ',');

        /*
            ISO dates YYYY-MM-DD can be compared
            directly as strings.
        */

        if (date < start_date ||
            date > end_date)
        {
            continue;
        }

        if (exdate != expiration_date)
        {
            continue;
        }

        if (flag_string.empty() ||
            flag_string[0] != option_flag)
        {
            continue;
        }

        double row_strike =
            std::stod(strike_string);

        if (std::fabs(
                row_strike - strike) > 1e-8)
        {
            continue;
        }

        double bid =
            std::stod(bid_string);

        double offer =
            std::stod(offer_string);

        /*
            Project definition:

            V = (best_bid + best_offer) / 2
        */

        double option_price =
            0.5 * (bid + offer);

        option_prices[date] =
            option_price;
    }


    std::vector<MarketDataPoint> result;

    for (std::map<std::string, double>::const_iterator it =
             option_prices.begin();
         it != option_prices.end();
         ++it)
    {
        const std::string& date =
            it->first;

        std::map<std::string, double>::const_iterator
            stock_it =
                stock_prices.find(date);

        std::map<std::string, double>::const_iterator
            rate_it =
                interest_rates.find(date);

        if (stock_it == stock_prices.end())
        {
            throw std::runtime_error(
                "Missing stock price for " + date);
        }

        if (rate_it == interest_rates.end())
        {
            throw std::runtime_error(
                "Missing interest rate for " + date);
        }

        MarketDataPoint point;

        point.date =
            date;

        point.stock_price =
            stock_it->second;

        point.option_price =
            it->second;

        point.risk_free_rate =
            rate_it->second;

        result.push_back(point);
    }

    if (result.empty())
    {
        throw std::runtime_error(
            "No matching option data found.");
    }

    return result;
}


int MarketData::tradingDaysBetween(
    const std::string& start_date,
    const std::string& end_date) const
{
    if (end_date < start_date)
    {
        throw std::invalid_argument(
            "End date cannot be before start date.");
    }

    int trading_days = 0;

    /*
        Count actual dates appearing in the
        supplied GOOG trading calendar.

        Exclude start_date.

        If expiration is a weekend, such as
        2011-09-17, the final counted date is
        naturally the previous trading day.
    */

    for (std::map<std::string, double>::const_iterator it =
             stock_prices.upper_bound(start_date);
         it != stock_prices.end() &&
         it->first <= end_date;
         ++it)
    {
        trading_days++;
    }

    return trading_days;
}


double MarketData::timeToMaturity(
    const std::string& current_date,
    const std::string& expiration_date) const
{
    return
        tradingDaysBetween(
            current_date,
            expiration_date)
        / 252.0;
}