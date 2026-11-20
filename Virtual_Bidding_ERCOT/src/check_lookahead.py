
import numpy as np
import pandas as pd

from part1_baselines import load_data, add_price_lag


def price_features(df):
    data = add_price_lag(df, 2)
    data = add_price_lag(data, 7)

    features = data[["spread_lag2", "spread_lag7"]].copy()
    features.index = df.index

    return features


def assert_no_lookahead(
    feature_fn,
    df,
    days=("2023-07-15", "2024-03-10", "2025-11-03")
):
    # Professor's Lab 2 look-ahead test
    clean = feature_fn(df)

    for d in pd.to_datetime(list(days)):
        poisoned = df.copy()

        late = poisoned.delivery_date >= (
            d - pd.Timedelta(days=1)
        )

        rng = np.random.default_rng(0)

        for c in ["da_price", "rt_price", "spread"]:
            poisoned.loc[late, c] = rng.normal(
                1e4, 1e3, late.sum()
            )

        poisoned["y"] = (poisoned.spread > 0).astype(int)

        dirty = feature_fn(poisoned)

        rows = df.delivery_date == d

        if not clean.loc[rows].equals(dirty.loc[rows]):
            bad = [
                c for c in clean
                if not clean.loc[rows, c].equals(
                    dirty.loc[rows, c]
                )
            ]

            raise AssertionError(
                f"look-ahead on {d.date()} in {bad}"
            )

    print("no look-ahead in", list(clean.columns))


def main():
    df = load_data()

    features = price_features(df)

    assert len(features) == len(df)

    print("Dataset rows:", len(df))
    print("Features:", list(features.columns))

    assert_no_lookahead(price_features, df)

    print("Feature verification passed")


if __name__ == "__main__":
    main()
