#ifndef TEST_H
#define TEST_H

#include "Option.h"
#include "BlackScholesModel.h"
#include "DeltaHedger.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include "Simulation.h"
#include "MarketData.h"
inline bool approximatelyEqual(
    double first,
    double second,
    double tolerance = 1e-4)
{
    return std::fabs(first - second) < tolerance;
}

inline void runBlackScholesTests()
{
    int passed = 0;
    int total = 0;

    /*
        Standard benchmark:

        S     = 100
        K     = 100
        r     = 0.05
        tau   = 1
        sigma = 0.20
    */

    Option call_option(
        100.0,
        "2027-01-01",
        'C');

    BlackScholesModel call_model(
        call_option);

    /*
        Test 1:
        Black-Scholes call price.
    */

    total++;

    double call_price =
        call_model.price(
            100.0,
            0.05,
            1.0,
            0.20);

    if (approximatelyEqual(
            call_price,
            10.450584,
            1e-4))
    {
        passed++;
    }
    else
    {
        std::cout
            << "Test 1 failed: call price. Expected about "
            << 10.450584
            << ", got "
            << call_price
            << std::endl;
    }


    /*
        Test 2:
        Call Delta.
    */

    total++;

    double call_delta =
        call_model.delta(
            100.0,
            0.05,
            1.0,
            0.20);

    if (approximatelyEqual(
            call_delta,
            0.636831,
            1e-4))
    {
        passed++;
    }
    else
    {
        std::cout
            << "Test 2 failed: call Delta. Expected about "
            << 0.636831
            << ", got "
            << call_delta
            << std::endl;
    }


    /*
        Test 3:
        Recover implied volatility.

        Generate a market price using
        known sigma = 0.20, then solve
        backwards for sigma.
    */

    total++;

    double recovered_sigma =
        call_model.impliedVolatility(
            100.0,
            0.05,
            1.0,
            call_price);

    if (approximatelyEqual(
            recovered_sigma,
            0.20,
            1e-4))
    {
        passed++;
    }
    else
    {
        std::cout
            << "Test 3 failed: implied volatility. "
            << "Expected 0.20, got "
            << recovered_sigma
            << std::endl;
    }


    /*
        Test 4:
        Put price and Delta.
    */

    total++;

    Option put_option(
        100.0,
        "2027-01-01",
        'P');

    BlackScholesModel put_model(
        put_option);

    double put_price =
        put_model.price(
            100.0,
            0.05,
            1.0,
            0.20);

    double put_delta =
        put_model.delta(
            100.0,
            0.05,
            1.0,
            0.20);

    if (approximatelyEqual(
            put_price,
            5.573526,
            1e-4) &&
        approximatelyEqual(
            put_delta,
            -0.363169,
            1e-4))
    {
        passed++;
    }
    else
    {
        std::cout
            << "Test 4 failed: put price/Delta."
            << std::endl;
    }


    /*
        Test 5:
        At expiration, option value
        equals intrinsic value.

        Call:
        S = 120
        K = 100

        payoff = 20
    */

    total++;

    double expiration_value =
        call_model.price(
            120.0,
            0.05,
            0.0,
            0.20);

    if (approximatelyEqual(
            expiration_value,
            20.0))
    {
        passed++;
    }
    else
    {
        std::cout
            << "Test 5 failed: expiration payoff."
            << std::endl;
    }


    /*
        Test 6:
        Impossible market price should
        not produce an implied volatility.
    */

    total++;

    bool invalid_price_caught = false;

    try
    {
        call_model.impliedVolatility(
            100.0,
            0.05,
            1.0,
            200.0);
    }
    catch (const std::invalid_argument&)
    {
        invalid_price_caught = true;
    }

    if (invalid_price_caught)
    {
        passed++;
    }
    else
    {
        std::cout
            << "Test 6 failed: impossible market price."
            << std::endl;
    }


    std::cout
        << "Black-Scholes tests passed: "
        << passed
        << "/"
        << total
        << std::endl;
}

inline void runSimulationTests()
{
    int passed = 0;
    int total = 0;

    Simulation simulation(
        100.0,
        0.4,
        0.05,
        0.24,
        100,
        1000
    );

    /*
        Test 1:
        Correct number of paths.
    */

    total++;

    std::vector<std::vector<double> > paths =
        simulation.generateStockPaths(42);

    if (paths.size() == 1000)
    {
        passed++;
    }
    else
    {
        std::cout
            << "Simulation Test 1 failed: expected 1000 paths."
            << std::endl;
    }

    /*
        Test 2:
        Each path should contain
        S0 plus 100 simulated observations.
    */

    total++;

    if (!paths.empty() &&
        paths[0].size() == 101)
    {
        passed++;
    }
    else
    {
        std::cout
            << "Simulation Test 2 failed: expected 101 observations."
            << std::endl;
    }

    /*
        Test 3:
        Every path starts at S0 = 100.
    */

    total++;

    bool correct_initial_price = true;

    for (std::size_t i = 0;
         i < paths.size();
         i++)
    {
        if (!approximatelyEqual(
                paths[i][0],
                100.0,
                1e-10))
        {
            correct_initial_price = false;
            break;
        }
    }

    if (correct_initial_price)
    {
        passed++;
    }
    else
    {
        std::cout
            << "Simulation Test 3 failed: incorrect initial stock price."
            << std::endl;
    }

    /*
        Test 4:
        Fixed random seed should make
        the simulation reproducible.
    */

    total++;

    std::vector<std::vector<double> > paths_again =
        simulation.generateStockPaths(42);

    bool reproducible =
        approximatelyEqual(
            paths[0][1],
            paths_again[0][1],
            1e-12);

    if (reproducible)
    {
        passed++;
    }
    else
    {
        std::cout
            << "Simulation Test 4 failed: simulation not reproducible."
            << std::endl;
    }

    std::cout
        << "Simulation tests passed: "
        << passed
        << "/"
        << total
        << std::endl;
}

inline void runDeltaHedgerTests()
{
    int passed = 0;
    int total = 0;

    DeltaHedger hedger(0.1);

    /*
        Test 1:
        Verify initial bank balance.

        S0 = 100
        V0 = 10
        delta0 = 0.5

        B0 = V0 - delta0*S0
           = 10 - 50
           = -40
    */

    total++;

    std::vector<double> stock_prices;
    stock_prices.push_back(100.0);
    stock_prices.push_back(110.0);

    std::vector<double> option_prices;
    option_prices.push_back(10.0);
    option_prices.push_back(15.0);

    std::vector<double> deltas;
    deltas.push_back(0.5);

    std::vector<double> bank =
        hedger.calculateBankBalances(
            stock_prices,
            option_prices,
            deltas,
            0.0
        );

    if (approximatelyEqual(
            bank[0],
            -40.0,
            1e-10))
    {
        passed++;
    }
    else
    {
        std::cout
            << "DeltaHedger Test 1 failed: "
            << "expected B0 = -40, got "
            << bank[0]
            << std::endl;
    }


    /*
        Test 2:
        Verify hedging error.

        With r = 0:

        HE1 =
            delta0*S1 + B0 - V1

            = 0.5*110 - 40 - 15
            = 0
    */

    total++;

    std::vector<double> errors =
        hedger.calculateHedgingErrors(
            stock_prices,
            option_prices,
            deltas,
            0.0
        );

    if (approximatelyEqual(
            errors[1],
            0.0,
            1e-10))
    {
        passed++;
    }
    else
    {
        std::cout
            << "DeltaHedger Test 2 failed: "
            << "expected HE1 = 0, got "
            << errors[1]
            << std::endl;
    }


    /*
        Test 3:
        Verify rebalancing over multiple periods.
    */

    total++;

    std::vector<double> stock2;
    stock2.push_back(100.0);
    stock2.push_back(102.0);
    stock2.push_back(104.0);

    std::vector<double> option2;
    option2.push_back(10.0);
    option2.push_back(11.0);
    option2.push_back(12.0);

    std::vector<double> delta2;
    delta2.push_back(0.5);
    delta2.push_back(0.6);

    std::vector<double> errors2 =
        hedger.calculateHedgingErrors(
            stock2,
            option2,
            delta2,
            0.0
        );

    /*
        B0 = 10 - 0.5*100 = -40

        B1 =
            0.5*102 - 40 - 0.6*102
            = -50.2

        HE2 =
            0.6*104 - 50.2 - 12
            = 0.2
    */

    if (approximatelyEqual(
            errors2[2],
            0.2,
            1e-10))
    {
        passed++;
    }
    else
    {
        std::cout
            << "DeltaHedger Test 3 failed: "
            << "expected HE2 = 0.2, got "
            << errors2[2]
            << std::endl;
    }


    std::cout
        << "DeltaHedger tests passed: "
        << passed
        << "/"
        << total
        << std::endl;
}
inline void runMarketDataTests()
{
    int passed = 0;
    int total = 0;

    MarketData market_data(
        "interest.csv",
        "sec_GOOG.csv",
        "op_GOOG.csv"
    );

    /*
        Test 1:
        Required GOOG contract has 19 observations.
    */

    total++;

    std::vector<MarketDataPoint> data =
        market_data.getOptionSeries(
            "2011-07-05",
            "2011-07-29",
            "2011-09-17",
            500.0,
            'C'
        );

    if (data.size() == 19)
    {
        passed++;
    }
    else
    {
        std::cout
            << "MarketData Test 1 failed: expected 19 observations, got "
            << data.size()
            << std::endl;
    }


    /*
        Test 2:
        Verify first GOOG observation.
    */

    total++;

    bool correct_first_row =
        data.front().date == "2011-07-05"
        &&
        approximatelyEqual(
            data.front().stock_price,
            532.44,
            1e-8
        )
        &&
        approximatelyEqual(
            data.front().option_price,
            44.20,
            1e-8
        )
        &&
        approximatelyEqual(
            data.front().risk_free_rate,
            0.0019,
            1e-10
        );

    if (correct_first_row)
    {
        passed++;
    }
    else
    {
        std::cout
            << "MarketData Test 2 failed: incorrect first observation."
            << std::endl;
    }


    /*
        Test 3:
        Verify trading-day time to maturity.

        52 trading days / 252.
    */

    total++;

    double tau =
        market_data.timeToMaturity(
            "2011-07-05",
            "2011-09-17"
        );

    if (approximatelyEqual(
            tau,
            52.0 / 252.0,
            1e-10
        ))
    {
        passed++;
    }
    else
    {
        std::cout
            << "MarketData Test 3 failed: incorrect time to maturity."
            << std::endl;
    }


    std::cout
        << "MarketData tests passed: "
        << passed
        << "/"
        << total
        << std::endl;
}
#endif