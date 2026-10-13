#ifndef TEST_H
#define TEST_H

#include "Option.h"
#include "BlackScholesModel.h"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include "Simulation.h"

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

#endif