# Black-Scholes Option Pricing

## Overview

This project implements Homework 3 for ISyE 6767 — Systems for Computational Finance.

The program prices European call and put options using two methods:

- Black-Scholes-Merton analytical pricing
- Cox-Ross-Rubinstein binomial lattice pricing

Both methods return:

- Option price
- Option Delta

The program supports call and put options using the flags:

```text
c / C = Call
p / P = Put
```

## Project Structure

```text
Black_Scholes/
├── Option.h
├── Option.cpp
├── pricing_method.h
├── Option_Price.h
├── Option_Price.cpp
├── test.h
├── main.cpp
├── README.md
└── screenshots/
    └── program_run.png
```

### `Option.h`

Declares the `Option` class.

The class contains private members for:

- Strike price `K`
- Current underlying price `S`
- Risk-free rate `r`
- Time to maturity `T`
- Volatility `sigma`

It also declares:

- Private `init()` method
- Default constructor
- Parameterized constructor
- Destructor
- Getter methods for all option parameters

### `Option.cpp`

Implements the `Option` class.

The default constructor calls `init()` to initialize all option parameters to zero.

The parameterized constructor initializes an option using user-supplied values for:

```text
K, S, r, T, sigma
```

### `pricing_method.h`

Defines the abstract `Pricing_Method` class.

It declares the pure virtual functions:

```cpp
BSM_Pricer()
Binomial_Pricer()
```

Both functions return a `PricingResult` containing:

```text
price
delta
```

### `Option_Price.h`

Declares the `Option_Price` class.

`Option_Price` inherits from both:

```text
Option
Pricing_Method
```

It contains the public `flag` member representing whether the option is a call or put.

### `Option_Price.cpp`

Implements the two pricing methods.

#### Black-Scholes-Merton

The Black-Scholes-Merton implementation calculates:

```text
d1
d2
option price
Delta
```

For a European call:

```text
C = S*N(d1) - K*exp(-rT)*N(d2)
Delta = N(d1)
```

For a European put:

```text
P = K*exp(-rT)*N(-d2) - S*N(-d1)
Delta = N(d1) - 1
```

#### Binomial Lattice

The binomial model uses a Cox-Ross-Rubinstein lattice with 1000 time steps.

The implementation calculates:

```text
dt
up factor
down factor
risk-neutral probability
terminal payoffs
backward induction
option price
Delta
```

The binomial Delta is calculated using the option values at the first up and down nodes.

### `test.h`

Contains the unit tests for the project.

The test suite checks:

1. Black-Scholes call price
2. Black-Scholes call Delta
3. Black-Scholes put price
4. Black-Scholes put Delta
5. Uppercase call flag
6. Uppercase put flag
7. Binomial call consistency with BSM
8. Binomial put consistency with BSM
9. Put-call parity
10. Invalid option flag
11. Invalid volatility
12. Invalid maturity
13. Default constructor initialization
14. Parameterized constructor, getters, and flag
15. Invalid flag handling in the binomial model

The current implementation passes:

```text
Unit tests passed: 15/15
```

### `main.cpp`

The main program:

1. Runs the unit tests.
2. Prompts the user for option parameters.
3. Constructs an `Option_Price` object.
4. Computes the Black-Scholes-Merton price and Delta.
5. Computes the binomial price and Delta.
6. Displays the differences between the two methods.

## User Input

The program prompts for:

```text
Strike price K
Current underlying price S
Risk-free rate r
Time to maturity T
Volatility sigma
Option flag
```

Example:

```text
K = 100
S = 100
r = 0.05
T = 1
sigma = 0.20
flag = C
```

## Example Results

For:

```text
S = 100
K = 100
r = 0.05
T = 1
sigma = 0.20
Call option
```

the program produces approximately:

```text
Black-Scholes-Merton:
Price: 10.450584
Delta: 0.636831

Binomial Lattice:
Price: 10.448584
Delta: 0.636799

Price difference: -0.001999
Delta difference: -0.000032
```

The very small differences indicate that the binomial lattice result is consistent with the analytical Black-Scholes-Merton result.

## Compilation

From inside the `Black_Scholes` directory:

```bash
g++ -Wall -Wextra -pedantic -std=c++11 \
Option.cpp Option_Price.cpp main.cpp -o main
```

## Running the Program

Run:

```bash
./main
```

## Program Execution

The final execution screenshot is stored at:

```text
screenshots/program_run.png
```

## Author

Varun Jhaveri  
Georgia Institute of Technology  
ISyE 6767 — Systems for Computational Finance