# ERCOT North Hub — Virtual Bidding Strategy

**ISyE 6767: Systems for Computational Finance — Interim Project II**

## Overview

This project develops a virtual bidding strategy for ERCOT's North Hub using historical day-ahead (DA) and real-time (RT) electricity prices, load forecasts, and renewable generation forecasts.

The implementation covers three components:

1. **Direction prediction:** Classify whether the DA–RT spread will be positive or negative using baseline strategies, logistic regression, gradient boosting, and a heuristic.
2. **Spread magnitude forecasting:** Predict signed and unsigned spreads using linear and tree-based regression, then evaluate trading thresholds and position-sizing rules.
3. **Trading budget allocation:** Allocate a maximum of 24 MWh per day, with a maximum of 4 MWh per hour, using scaled allocations, integer knapsack, and risk-aware quadratic optimization.

## Repository Structure

```text
Virtual_Bidding_ERCOT/
├── lab2_student.ipynb
├── lab2_data/                  # Raw data (not tracked by Git)
├── ercot_hb_north_hourly.csv
├── src/
│   ├── part1_baselines.py
│   ├── check_lookahead.py
│   ├── part1_features.py
│   ├── part1_classification.py
│   ├── part2_regression.py
│   ├── part2_sizing.py
│   ├── part3_allocation.py
│   ├── feature_ablation.py
│   ├── weighted_knapsack_example.py
│   └── final_evaluation.py
├── outputs/                    # Evaluation results and trades
├── figures/                    # Generated plots
├── report.md
├── report.pdf
├── requirements.txt
└── README.md
```

## Data

The project uses ERCOT historical market data supplied with Lab 2:

- Day-ahead hourly settlement point prices
- Real-time 15-minute settlement point prices
- System and North-zone load forecasts
- System-wide wind and solar forecasts
- Forecast publication timestamps

The raw data is not included in the Git repository. Obtain `lab2_data.zip` from the course materials and extract it into the project root.

The original `lab2_student.ipynb` is the reproducible data-cleaning pipeline. Run its cells sequentially to generate `ercot_hb_north_hourly.csv` from the raw files.

The dataset contains 27,048 hourly observations for December 2022–December 2025.

## Installation

From the project directory:

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements.txt
```

For the Lab 2 notebook, select the project's Python virtual environment as the Jupyter kernel.

## Running the Project

After generating `ercot_hb_north_hourly.csv` with the Lab 2 notebook, run the following commands from the project root:

```bash
# Verify historical price features
python src/check_lookahead.py

# Build and verify all 22 model features
python src/part1_features.py

# Part 1: Classification
python src/part1_baselines.py
python src/part1_classification.py

# Part 2: Spread forecasting and sizing
python src/part2_regression.py
python src/part2_sizing.py

# Part 3: Allocation
python src/part3_allocation.py
python src/weighted_knapsack_example.py

# Feature ablation
python src/feature_ablation.py

# Final holdout evaluation
python src/final_evaluation.py
```

The final evaluation regenerates the 2025 trading results, summary statistics, and cumulative P&L figure using the fixed model specifications.

## Experimental Design

| Period | Purpose |
|---|---|
| December 2022 | Historical price lags |
| 2023 | Training |
| 2024 | Validation and model selection |
| 2025 | Final out-of-sample testing |

Trading decisions are assumed to be made at 9:00 AM Central Time on the day before delivery.

Price-derived features use observations from D−2 or earlier. Forecasts are restricted to those published before the decision deadline.

The full feature matrix contains 22 variables, including calendar effects, historical price statistics, forecast load, renewable generation forecasts, net load, and net-load ramps.

## Results

Selected 2025 test results at a transaction cost of $1/MWh:

| Strategy | Net profit | Profit/MWh |
|---|---:|---:|
| Gradient-boosted classification | −$1,262.92 | −$0.14 |
| Signed regression, threshold 5 | $4,472.88 | $1.74 |
| Knapsack allocation | $13,419.21 | $1.89 |
| Scaled-edge allocation | $13,401.41 | $1.53 |
| Risk-aware allocation, λ = 5 | $5,087.99 | $4.06 |

Part 2 sizing strategies and Part 3 allocation strategies use different budget constraints and should not be compared as though their capital exposure were identical.

The five best knapsack trading days accounted for 142.3% of total annual net profit, demonstrating the significant influence of extreme price-spread events.

## Outputs

Important results are saved in `outputs/`:

- `project2_test_trades_2025.csv` — complete hourly test trades for all evaluated strategies
- `project2_test_summary_2025.csv` — profitability, trading volumes, risk metrics, and drawdown
- `project2_test_classification_2025.csv` — classification metrics
- `project2_test_regression_2025.csv` — regression MAE and RMSE
- `project2_test_worst_days_2025.csv` — five worst days per strategy
- `project2_test_all_cumulative_2025.csv` — cumulative daily net profit for all strategies
- `feature_ablation_2024.csv` — feature-group ablation results

Figures are saved in `figures/`.

## Limitations

Electricity price spreads exhibit extreme spikes and heavy-tailed distributions. Historical relationships may change, and profitable backtests may depend on a small number of unusually favorable trading days.

The simulation does not model market impact, bid acceptance uncertainty, or actual ERCOT collateral requirements.

Annualized returns in the report use an illustrative $100,000 capital assumption, not verified invested capital.

## Report

See [report.md](report.md) or `report.pdf` for detailed methodology, results, figures, comparisons, and discussion of limitations.
