# Homework 4 — NumPy Exercise

ISyE 6767 — Systems and Computation for Finance  
Fall 2026

This project contains the implementation for Homework 4, consisting of two NumPy-based problems:

1. A rule-based stock trading strategy applied to historical AAPL data.
2. A dynamic-programming solution for the Energy Maze problem.

## Project Structure

```text
Homework4_Numpy/
├── hw4.py
├── aapl.csv
├── trading_results.csv
├── cumulative_pnl.png
├── README.md
├── report.md
└── report.pdf
```

## Requirements

The implementation uses Python 3 with:

```text
numpy
matplotlib
```

If needed, install the dependencies with:

```bash
pip3 install numpy matplotlib
```

## Running the Program

From inside the `Homework4_Numpy` directory, run:

```bash
python3 hw4.py
```

The program executes:

- the provided Task 1 trading-strategy example;
- a custom Task 1 unit test;
- the trading strategy on `aapl.csv`;
- the provided Task 2 Energy Maze example;
- custom and boundary tests for Task 2.

## Task 1 — Trading Strategy

The strategy begins with an account value of $10,000 and trades 10 shares at a time.

The rules are:

- Buy 10 shares when the stock price has increased for three consecutive days, provided the current position is less than 20 shares.
- Sell the entire position when the stock price has decreased for two consecutive days.
- Liquidate any remaining position on the final trading day.
- Otherwise, take no action.

The program generates:

```text
trading_results.csv
```

with columns:

```text
Date,Signal,Position,Account_Value
```

It also generates:

```text
cumulative_pnl.png
```

showing the cumulative trading P&L over the supplied AAPL historical period.

For the supplied AAPL dataset, the final cumulative trading P&L is:

```text
$1377.38
```

## Task 2 — Energy Maze

`min_initial_energy(maze)` calculates the minimum initial energy required to move from the top-left to the bottom-right of a maze while:

- moving only right or down;
- never allowing energy to fall below 1.

The solution uses dynamic programming with a NumPy array.

For the provided example:

```text
[[-2,  -3,  3],
 [-5, -10,  1],
 [10,  30, -5]]
```

the minimum initial energy is:

```text
7
```

## Tests

The implementation checks:

- exact reproduction of the provided trading-strategy example;
- a custom trading-strategy example;
- the supplied AAPL dataset;
- exact reproduction of the provided Energy Maze example;
- a custom Energy Maze example;
- positive and negative single-cell maze edge cases.

All implemented tests pass successfully.

## Output Files

`trading_results.csv` contains the daily trading results for the AAPL dataset.

`cumulative_pnl.png` contains the cumulative P&L visualization.

`report.pdf` contains the brief project report and results.