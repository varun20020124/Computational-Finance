
import numpy as np
import pandas as pd

from pandas.tseries.holiday import USFederalHolidayCalendar

from part1_baselines import load_data
from check_lookahead import assert_no_lookahead


def build_features(df):
    data = df.copy()

    assert data["interval_start_utc"].is_monotonic_increasing

    data["delivery_date"] = pd.to_datetime(
        data["delivery_date"]
    ).dt.normalize()

    data["repeated"] = (
        data["repeated"].astype(str).str.lower()
        .isin(["true", "y", "1"])
    )

    x = pd.DataFrame(index=data.index)

    # ----------------------------------------
    # 1. Calendar features
    # ----------------------------------------

    x["hour_of_day"] = data["hour_ending"] - 1
    x["day_of_week"] = data["delivery_date"].dt.day_of_week
    x["month"] = data["delivery_date"].dt.month

    x["is_weekend"] = (x["day_of_week"] >= 5).astype(int)

    holidays = USFederalHolidayCalendar().holidays(
        start=data["delivery_date"].min(),
        end=data["delivery_date"].max()
    )

    x["is_holiday"] = (
        data["delivery_date"].isin(holidays).astype(int)
    )

    x["is_repeated_hour"] = data["repeated"].astype(int)

    # ----------------------------------------
    # 2. Forecast features
    # ----------------------------------------

    forecast_cols = [
        "load_fcst_system_mw",
        "load_fcst_north_mw",
        "wind_fcst_mw",
        "solar_fcst_mw"
    ]

    for col in forecast_cols:
        x[col] = data[col]

    # ----------------------------------------
    # 3. Net load and ramps
    # ----------------------------------------

    x["net_load_mw"] = (
        x["load_fcst_system_mw"]
        - x["wind_fcst_mw"]
        - x["solar_fcst_mw"]
    )

    # Difference between consecutive forecast hours
    # within the same delivery day
    x["net_load_ramp_mw"] = (
        x["net_load_mw"]
        .groupby(data["delivery_date"])
        .diff()
        .fillna(0.0)
    )

    # ----------------------------------------
    # 4. Historical price grid
    # ----------------------------------------

    # Explicit calendar grid handles 23/25-hour days.
    calendar = pd.date_range(
        data["delivery_date"].min(),
        data["delivery_date"].max(),
        freq="D"
    )

    grid = pd.MultiIndex.from_product(
        [calendar, range(1, 25)],
        names=["delivery_date", "hour_ending"]
    )

    # First copy of each repeated hour, as in Lab 2
    history = (
        data.loc[
            ~data["repeated"],
            [
                "delivery_date",
                "hour_ending",
                "da_price",
                "rt_price",
                "spread"
            ]
        ]
        .set_index(["delivery_date", "hour_ending"])
        .reindex(grid)
    )

    price = pd.DataFrame(index=grid)
    by_hour = history.groupby(level="hour_ending")

    # ----------------------------------------
    # 5. D-2 and D-7 price lags
    # ----------------------------------------

    for days in [2, 7]:
        for old, new in [
            ("da_price", "da"),
            ("rt_price", "rt"),
            ("spread", "spread")
        ]:
            price[f"{new}_lag{days}"] = (
                by_hour[old].shift(days)
            )

    # ----------------------------------------
    # 6. Rolling same-hour statistics
    # ----------------------------------------

    # First shift by 2 calendar days.
    # Thus all windows end at D-2.
    safe_spread = by_hour["spread"].shift(2)

    safe_groups = safe_spread.groupby(
        level="hour_ending"
    )

    price["spread_mean7"] = safe_groups.transform(
        lambda s: s.rolling(7, min_periods=1).mean()
    )

    price["spread_mean28"] = safe_groups.transform(
        lambda s: s.rolling(28, min_periods=1).mean()
    )

    price["spread_std28"] = safe_groups.transform(
        lambda s: s.rolling(28, min_periods=2).std()
    )

    # Positive spread frequency, preserving missing hours
    historical_positive = (
        history["spread"].gt(0).astype(float)
        .where(history["spread"].notna())
    )

    safe_positive = (
        historical_positive
        .groupby(level="hour_ending")
        .shift(2)
    )

    price["spread_frac_positive28"] = (
        safe_positive
        .groupby(level="hour_ending")
        .transform(
            lambda s: s.rolling(
                28, min_periods=1
            ).mean()
        )
    )

    # ----------------------------------------
    # 7. Match features to actual trading hours
    # ----------------------------------------

    keys = pd.MultiIndex.from_frame(
        data[["delivery_date", "hour_ending"]]
    )

    price = price.reindex(keys).reset_index(drop=True)

    return pd.concat([x, price], axis=1)


def main():
    df = load_data()
    features = build_features(df)

    print("Dataset shape:", df.shape)
    print("Feature matrix shape:", features.shape)

    print("\nFEATURE COLUMNS")
    print(features.columns.tolist())

    # Required price-feature leakage test
    assert_no_lookahead(build_features, df)

    # Inspect missing values in training and validation
    for year in [2023, 2024]:
        rows = df["delivery_date"].dt.year == year

        print(f"\n{year} missing feature values:")
        print(
            features.loc[rows]
            .isna()
            .sum()
            .loc[lambda s: s > 0]
        )

    print("\nFeature engineering checks passed")


if __name__ == "__main__":
    main()
