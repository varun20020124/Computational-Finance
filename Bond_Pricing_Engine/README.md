# Bond Pricing Engine

## Overview

This project implements Homework 2 for ISyE 6767 — Systems for Computational Finance.

The program implements a C++ `Bond` class, a bond pricing function based on a continuously compounded zero-rate term structure, and the valuation of an arithmetic-average exotic security.

The project uses the supplied `Bond_Ex3.csv` interest-rate data and follows the bond-pricing conventions specified in the assignment.

## Project Structure

```text
Bond_Pricing_Engine/
├── bond.h
├── bond.cpp
├── main.cpp
├── Bond_Ex3.csv
├── Bond.xlsx
├── README.md
└── screenshots/
    └── program_run.png
```

### `bond.h`

Contains the declaration of the `Bond` class.

The class stores:

- Expiration date
- Payment frequency
- Coupon rate

It also declares:

- Default constructor
- Destructor
- Copy constructor
- Parameterized constructor
- `ToString()`
- `Price()`

### `bond.cpp`

Contains the implementation of the `Bond` class and bond-pricing logic.

The `Price()` function:

1. Determines the remaining time to maturity.
2. Constructs the remaining coupon-payment schedule.
3. Handles a fractional first coupon period when necessary.
4. Interpolates interest rates from the supplied term structure.
5. Calculates discount factors using continuously compounded zero rates.
6. Discounts all remaining coupon and principal cash flows.
7. Returns the present value of the bond.

The discount factor used is:

```text
DF(T) = exp(-y(T) * T)
```

where `y(T)` is the continuously compounded zero rate associated with maturity `T`.

## Task 1 — Bond Class

The program creates and prints:

- A default bond.
- A semiannual bond with a 7% coupon rate expiring on November 19, 2035.

Example:

```text
Bond (11/19/2035, 0.5, 0.07)
```

## Task 2 — Bond Pricing

The bond-pricing function supports:

- Face value
- Coupon rate
- Payment frequency
- Expiration date
- Valuation date
- Interest-rate term structure

Only future cash flows are included in the valuation.

If the first remaining coupon period is shorter than the regular payment interval, the first coupon is adjusted according to the length of the stub period.

Interest rates that do not exactly appear in the supplied term structure are estimated using linear interpolation.

## Unit Tests

The program contains unit tests covering:

1. A regular semiannual coupon bond.
2. A bond with a fractional first coupon period.
3. A bond valued at maturity.

The current implementation passes:

```text
Unit tests passed: 3/3
```

## Task 3 — Arithmetic-Average Security

The underlying security is a 10-year bond issued on January 1, 2010 and maturing on January 1, 2020.

Parameters:

```text
Face value:      100
Coupon rate:     5%
Payment interval: 0.5 years
```

The bond is valued on January 1 of:

```text
2016
2017
2018
2019
2020
```

The resulting prices are:

```text
01/01/2016: 118.359996
01/01/2017: 113.117879
01/01/2018: 107.355219
01/01/2019: 103.491541
01/01/2020: 100.000000
```

The arithmetic-average payoff is:

```text
108.464927
```

The payoff is made on December 31, 2020 and is discounted back to the purchase date of August 3, 2015.

The calculated fair value is:

```text
105.906513
```

The actual purchase price was:

```text
98.000000
```

Therefore:

```text
Fair value - purchase price = 7.906513
```

Under this valuation model, the security was purchased below its calculated fair value and was therefore a favorable investment.

## Compilation

From inside the `Bond_Pricing_Engine` directory:

```bash
g++ -Wall -pedantic -std=c++11 bond.cpp main.cpp -o main
```

## Running the Program

Run:

```bash
./main
```

The program will:

- Display the Bond class examples.
- Run the unit tests.
- Load `Bond_Ex3.csv`.
- Value the five underlying bond observations.
- Calculate the arithmetic-average payoff.
- Discount the payoff to the purchase date.
- Compare the fair value with the $98 purchase price.

## Data Files

`Bond_Ex3.csv` contains the interest-rate term structure used for valuation.

`Bond.xlsx` is the supplied reference workbook.

The supplied data files were not manually modified.

## Author

Varun Jhaveri  
Georgia Institute of Technology  
ISyE 6767 — Systems for Computational Finance