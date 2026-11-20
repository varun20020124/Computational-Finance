
from pathlib import Path

import numpy as np
import pandas as pd


DATA_FILE = Path("ercot_hb_north_hourly.csv")


def load_data():
    df = pd.read_csv(
        DATA_FILE,
        parse_dates=["delivery_date", "interval_start_utc"]
    )

    df = df.sort_values("interval_start_utc").reset_index(drop=True)
    df["weekday"] = df["delivery_date"].dt.day_of_week

    return df


def add_price_lag(df, days):
    """
    Get the spread for the same local hour on D-days.
    No prices from D-1 or D are used.
    """
    history = df.loc[
        df["repeated"].astype(str).str.lower().eq("false"),
        ["delivery_date", "hour_ending", "spread"]
    ].copy()

    history["delivery_date"] += pd.Timedelta(days=days)

    col = f"spread_lag{days}"
    history = history.rename(columns={"spread": col})

    return df.merge(
        history,
        on=["delivery_date", "hour_ending"],
        how="left",
        validate="many_to_one",
        sort=False
    )


def main():
    df = load_data()

    # Create both historical price features
    df = add_price_lag(df, 2)
    df = add_price_lag(df, 7)

    # Chronological train/validation split
    train = df[df["delivery_date"].dt.year == 2023].copy()
    val = df[df["delivery_date"].dt.year == 2024].copy()

    print("Training hours:", len(train))
    print("Validation hours:", len(val))

    # Baseline 1 and 2
    val["always_inc"] = 1
    val["always_dec"] = 0

    # Baseline 3 and 4
    # Missing lag values fall back to the 2023 majority (INC)
    for days in [2, 7]:
        lag = val[f"spread_lag{days}"]
        val[f"persistence_d{days}"] = np.where(
            lag.isna(), 1, (lag > 0).astype(int)
        )

    # Baseline 5: majority direction for each hour of the week
    seasonal = (
        train.groupby(["weekday", "hour_ending"])["y"]
        .mean()
        .ge(0.5)
        .astype(int)
    )

    keys = pd.MultiIndex.from_frame(
        val[["weekday", "hour_ending"]]
    )

    val["seasonal_majority"] = (
        seasonal.reindex(keys)
        .fillna(1)
        .astype(int)
        .to_numpy()
    )

    methods = [
        "always_inc",
        "always_dec",
        "persistence_d2",
        "persistence_d7",
        "seasonal_majority"
    ]

    results = []
    daily_profits = pd.DataFrame(index=sorted(val.delivery_date.unique()))

    for method in methods:
        predictions = val[method]

        # +1 for INC, -1 for DEC
        actions = 2 * predictions - 1

        # Each baseline trades 1 MWh per delivery hour
        gross_profit = actions * val["spread"]
        net_profit = gross_profit - 1.0

        results.append({
            "strategy": method,
            "accuracy": (predictions == val["y"]).mean(),
            "total_mwh": len(val),
            "profit_c0": gross_profit.sum(),
            "profit_c1": net_profit.sum(),
            "profit_per_mwh_c0": gross_profit.mean(),
            "profit_per_mwh_c1": net_profit.mean()
        })

        daily_profits[method] = (
            gross_profit.groupby(val["delivery_date"])
            .sum()
            .reindex(daily_profits.index)
            .to_numpy()
        )

    results = pd.DataFrame(results)

    Path("outputs").mkdir(exist_ok=True)
    results.to_csv("outputs/part1_baselines_2024.csv", index=False)
    daily_profits.cumsum().to_csv(
        "outputs/part1_cumulative_daily_profit_2024.csv",
        index_label="delivery_date"
    )

    print("\n2024 VALIDATION RESULTS")
    print(results.round(3).to_string(index=False))


if __name__ == "__main__":
    main()
