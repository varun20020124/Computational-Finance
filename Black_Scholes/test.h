#ifndef TEST_H
#define TEST_H

#include "Option_Price.h"

#include <cmath>
#include <iostream>
#include <stdexcept>

inline bool approximatelyEqual(
    double first,
    double second,
    double tolerance = 1e-3)
{
    return std::fabs(first - second) < tolerance;
}

inline void runUnitTests()
{
    int passed = 0;
    int total = 0;

    /*
        Standard benchmark:

        S = 100
        K = 100
        r = 5%
        T = 1 year
        sigma = 20%
    */

    Option_Price call_option(
        100.0,
        100.0,
        0.05,
        1.0,
        0.20,
        'c');

    Option_Price put_option(
        100.0,
        100.0,
        0.05,
        1.0,
        0.20,
        'p');

    PricingResult call_bsm =
        call_option.BSM_Pricer();

    PricingResult put_bsm =
        put_option.BSM_Pricer();

    /*
        Test 1:
        BSM call price.
    */

    total++;

    if (approximatelyEqual(
            call_bsm.price,
            10.4506,
            1e-3))
    {
        passed++;
    }
    else
    {
        std::cout
            << "Test 1 failed: BSM call price. "
            << "Expected about 10.4506, got "
            << call_bsm.price
            << std::endl;
    }

    /*
        Test 2:
        BSM call Delta.
    */

    total++;

    if (approximatelyEqual(
            call_bsm.delta,
            0.6368,
            1e-3))
    {
        passed++;
    }
    else
    {
        std::cout
            << "Test 2 failed: BSM call Delta. "
            << "Expected about 0.6368, got "
            << call_bsm.delta
            << std::endl;
    }

    /*
        Test 3:
        BSM put price.
    */

    total++;

    if (approximatelyEqual(
            put_bsm.price,
            5.5735,
            1e-3))
    {
        passed++;
    }
    else
    {
        std::cout
            << "Test 3 failed: BSM put price. "
            << "Expected about 5.5735, got "
            << put_bsm.price
            << std::endl;
    }

    /*
        Test 4:
        BSM put Delta.
    */

    total++;

    if (approximatelyEqual(
            put_bsm.delta,
            -0.3632,
            1e-3))
    {
        passed++;
    }
    else
    {
        std::cout
            << "Test 4 failed: BSM put Delta. "
            << "Expected about -0.3632, got "
            << put_bsm.delta
            << std::endl;
    }

    /*
        Test 5:
        Uppercase C should behave
        exactly like lowercase c.
    */

    total++;

    Option_Price uppercase_call(
        100.0,
        100.0,
        0.05,
        1.0,
        0.20,
        'C');

    PricingResult uppercase_call_result =
        uppercase_call.BSM_Pricer();

    if (approximatelyEqual(
            uppercase_call_result.price,
            call_bsm.price,
            1e-10))
    {
        passed++;
    }
    else
    {
        std::cout
            << "Test 5 failed: uppercase C."
            << std::endl;
    }

    /*
        Test 6:
        Uppercase P should behave
        exactly like lowercase p.
    */

    total++;

    Option_Price uppercase_put(
        100.0,
        100.0,
        0.05,
        1.0,
        0.20,
        'P');

    PricingResult uppercase_put_result =
        uppercase_put.BSM_Pricer();

    if (approximatelyEqual(
            uppercase_put_result.price,
            put_bsm.price,
            1e-10))
    {
        passed++;
    }
    else
    {
        std::cout
            << "Test 6 failed: uppercase P."
            << std::endl;
    }

    /*
        Test 7:
        Binomial call should converge
        closely to BSM.
    */

    total++;

    PricingResult call_binomial =
        call_option.Binomial_Pricer();

    if (approximatelyEqual(
            call_binomial.price,
            call_bsm.price,
            0.01) &&
        approximatelyEqual(
            call_binomial.delta,
            call_bsm.delta,
            0.01))
    {
        passed++;
    }
    else
    {
        std::cout
            << "Test 7 failed: binomial call."
            << std::endl;

        std::cout
            << "BSM price = "
            << call_bsm.price
            << ", Binomial price = "
            << call_binomial.price
            << std::endl;

        std::cout
            << "BSM delta = "
            << call_bsm.delta
            << ", Binomial delta = "
            << call_binomial.delta
            << std::endl;
    }

    /*
        Test 8:
        Binomial put should converge
        closely to BSM.
    */

    total++;

    PricingResult put_binomial =
        put_option.Binomial_Pricer();

    if (approximatelyEqual(
            put_binomial.price,
            put_bsm.price,
            0.01) &&
        approximatelyEqual(
            put_binomial.delta,
            put_bsm.delta,
            0.01))
    {
        passed++;
    }
    else
    {
        std::cout
            << "Test 8 failed: binomial put."
            << std::endl;
    }

    /*
        Test 9:
        Put-call parity.

        C - P = S - K * exp(-rT)
    */

    total++;

    double left_side =
        call_bsm.price -
        put_bsm.price;

    double right_side =
        100.0 -
        100.0 * std::exp(-0.05);

    if (approximatelyEqual(
            left_side,
            right_side,
            1e-6))
    {
        passed++;
    }
    else
    {
        std::cout
            << "Test 9 failed: put-call parity."
            << std::endl;
    }

    /*
        Test 10:
        Invalid option flag must be rejected.
    */

    total++;

    bool invalid_flag_caught = false;

    try
    {
        Option_Price invalid_option(
            100.0,
            100.0,
            0.05,
            1.0,
            0.20,
            'x');

        invalid_option.BSM_Pricer();
    }
    catch (const std::invalid_argument&)
    {
        invalid_flag_caught = true;
    }

    if (invalid_flag_caught)
    {
        passed++;
    }
    else
    {
        std::cout
            << "Test 10 failed: invalid flag."
            << std::endl;
    }

    /*
        Test 11:
        Invalid volatility must be rejected.
    */

    total++;

    bool invalid_sigma_caught = false;

    try
    {
        Option_Price invalid_option(
            100.0,
            100.0,
            0.05,
            1.0,
            -0.20,
            'c');

        invalid_option.BSM_Pricer();
    }
    catch (const std::invalid_argument&)
    {
        invalid_sigma_caught = true;
    }

    if (invalid_sigma_caught)
    {
        passed++;
    }
    else
    {
        std::cout
            << "Test 11 failed: invalid volatility."
            << std::endl;
    }

    /*
        Test 12:
        Invalid maturity must be rejected.
    */

    total++;

    bool invalid_maturity_caught = false;

    try
    {
        Option_Price invalid_option(
            100.0,
            100.0,
            0.05,
            0.0,
            0.20,
            'c');

        invalid_option.BSM_Pricer();
    }
    catch (const std::invalid_argument&)
    {
        invalid_maturity_caught = true;
    }

    if (invalid_maturity_caught)
    {
        passed++;
    }
    else
    {
        std::cout
            << "Test 12 failed: invalid maturity."
            << std::endl;
    }

    std::cout
        << "Unit tests passed: "
        << passed
        << "/"
        << total
        << std::endl;
}

#endif