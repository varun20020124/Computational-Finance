
from pathlib import Path

import numpy as np
import pandas as pd

from part1_baselines import load_data
from part1_features import build_features
from part1_classification import get_models as get_classifiers
from part2_regression import get_models as get_regressors


OUTPUT = Path("outputs")

COST = 1.0
TAU = 5.0
MAX_HOURLY_VOLUME = 4.0
RISK_FLOOR = 5.0


def allocate_volume(weights, total_mwh, cap=4.0):
    """
    Allocate a fixed total volume proportional to weights,
    subject to an hourly cap.

    Sum of allocated volumes equals total_mwh.
    """
    weights = np.asarray(weights, dtype=float)

    if len(weights) == 0:
        return weights.copy()

    assert np.all(np.isfinite(weights))
    assert np.all(weights > 0)
    assert total_mwh <= cap * len(weights)

    low, high = 0.0, 1.0

    while np.minimum(cap, high * weights).sum() < total_mwh:
        high *= 2

    for _ in range(70):
        mid = (low + high) / 2

        volume = np.minimum(cap, mid * weights)

        if volume.sum() < total_mwh:
            low = mid
        else:
            high = mid

    return np.minimum(cap, high * weights)


def evaluate_sizing(view, signal, spread, risk, dates, times):
    """
    Compare sizing rules on identical active hours
    and identical total MWh within each view.
    """
    action = np.sign(signal).astype(int)

    predicted_edge = np.abs(signal) - COST

    # Trade only when predicted edge exceeds tau
    active = (predicted_edge > TAU) & (action != 0)

    n_active = int(active.sum())

    print(f"\n{view}: {n_active} eligible hours")

    if n_active == 0:
        return [], []

    edge_weights = predicted_edge[active]

    # Rolling 28-day spread volatility is known by D-2.
    safe_risk = np.maximum(risk[active], RISK_FLOOR)

    risk_weights = edge_weights / safe_risk**2

    volumes = {
        "fixed": np.ones(n_active),

        "edge": allocate_volume(
            edge_weights,
            n_active,
            MAX_HOURLY_VOLUME
        ),

        "risk_adjusted": allocate_volume(
            risk_weights,
            n_active,
            MAX_HOURLY_VOLUME
        )
    }

    results = []
    trade_frames = []

    for method, active_volume in volumes.items():

        volume = np.zeros(len(spread))
        volume[active] = active_volume

        # Same total volume for all sizing approaches
        assert np.isclose(volume.sum(), n_active)

        # Enforce maximum volume per hour
        assert volume.max() <= MAX_HOURLY_VOLUME + 1e-8

        # Actual realized profit
        profit_c0 = volume * action * spread
        profit_c1 = volume * (action * spread - COST)

        total_mwh = volume.sum()
        total_profit = profit_c1.sum()

        results.append({
            "view": view,
            "sizing": method,
            "n_trades": n_active,
            "total_mwh": total_mwh,
            "profit_c0": profit_c0.sum(),
            "profit_c1": total_profit,
            "profit_per_mwh_c1": total_profit / total_mwh,
            "max_hourly_volume": volume.max(),
            "win_rate": (profit_c1[active] > 0).mean()
        })

        trades = pd.DataFrame({
            "delivery_date": dates,
            "interval_start_utc": times,
            "view": view,
            "sizing": method,
            "prediction": signal,
            "action": np.where(active, action, 0),
            "volume": volume,
            "spread": spread,
            "profit_c0": profit_c0,
            "profit_c1": profit_c1
        })

        trade_frames.append(trades)

    return results, trade_frames


def main():
    OUTPUT.mkdir(exist_ok=True)

    df = load_data()

    # Never use 2025 during model selection
    df = df.loc[
        df["delivery_date"].dt.year <= 2024
    ].reset_index(drop=True)

    X = build_features(df)

    train = df["delivery_date"].dt.year == 2023
    val = df["delivery_date"].dt.year == 2024

    X_train = X.loc[train]
    X_val = X.loc[val]

    train_spread = df.loc[train, "spread"].to_numpy()
    val_spread = df.loc[val, "spread"].to_numpy()

    dates = df.loc[val, "delivery_date"].to_numpy()
    times = df.loc[val, "interval_start_utc"].to_numpy()

    risk = X_val["spread_std28"].to_numpy()

    assert np.isfinite(risk).all()

    # ---------------------------------------------
    # View B: signed spread regression
    # ---------------------------------------------

    signed_model = get_regressors()["boost_absolute"]

    signed_model.fit(X_train, train_spread)

    signed_prediction = signed_model.predict(X_val)

    # ---------------------------------------------
    # View A: classifier + unsigned magnitude
    # ---------------------------------------------

    classifier = get_classifiers()["gradient_boosting"]

    classifier.fit(
        X_train,
        df.loc[train, "y"].astype(int)
    )

    probability = classifier.predict_proba(X_val)[:, 1]

    magnitude_model = get_regressors()["boost_absolute"]

    magnitude_model.fit(
        X_train,
        np.abs(train_spread)
    )

    predicted_magnitude = np.maximum(
        magnitude_model.predict(X_val),
        0.0
    )

    # Probability-magnitude signal proxy.
    # Assumes conditional sign/magnitude independence
    # if interpreted as an expected signed spread.
    combined_signal = (
        (2 * probability - 1) * predicted_magnitude
    )

    # ---------------------------------------------
    # Compare three sizing rules for each view
    # ---------------------------------------------

    all_results = []
    all_trades = []

    views = {
        "signed_regression": signed_prediction,
        "classifier_magnitude": combined_signal
    }

    for name, signal in views.items():

        results, trades = evaluate_sizing(
            name,
            signal,
            val_spread,
            risk,
            dates,
            times
        )

        all_results.extend(results)
        all_trades.extend(trades)

    # ---------------------------------------------
    # Save results
    # ---------------------------------------------

    results_df = pd.DataFrame(all_results)

    print("\n2024 SIZING COMPARISON")
    print(results_df.round(3).to_string(index=False))

    results_df.to_csv(
        OUTPUT / "part2_sizing_2024.csv",
        index=False
    )

    trades_df = pd.concat(all_trades, ignore_index=True)

    trades_df.to_csv(
        OUTPUT / "part2_sizing_trades_2024.csv",
        index=False
    )

    print("\nSizing comparison completed successfully")


if __name__ == "__main__":
    main()
