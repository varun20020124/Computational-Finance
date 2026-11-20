
from pathlib import Path

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

from sklearn.metrics import (
    accuracy_score,
    balanced_accuracy_score,
    roc_auc_score,
    log_loss,
    mean_absolute_error,
    mean_squared_error,
)

from part1_baselines import load_data
from part1_features import build_features
from part1_classification import get_models as get_classifiers
from part2_regression import get_models as get_regressors
from part2_sizing import evaluate_sizing
from part3_allocation import (
    BUDGET, COST, TAU, RISK_LAMBDAS, HOURLY_CAP,
    allocate_scaled, allocate_knapsack,
    allocate_risk_aware, estimate_covariance, max_drawdown,
)
from check_lookahead import assert_no_lookahead


OUTPUT = Path("outputs")
FIGURES = Path("figures")

# Illustrative capital assumption, not provided by ERCOT.
ASSUMED_CAPITAL = 100_000.0


def main():
    OUTPUT.mkdir(exist_ok=True)
    FIGURES.mkdir(exist_ok=True)

    df = load_data()
    features = build_features(df)

    # Verify the price-feature decision-time restriction.
    assert_no_lookahead(
        build_features,
        df,
        days=("2025-03-09", "2025-07-15", "2025-11-03")
    )

    train_mask = df.delivery_date.dt.year == 2023
    test_mask = df.delivery_date.dt.year == 2025

    train = df.loc[train_mask].copy()
    test = df.loc[test_mask].copy().reset_index(drop=True)

    X_train = features.loc[train_mask]
    X_test = features.loc[test_mask].reset_index(drop=True)

    assert len(train) == 8760
    assert len(test) == 8760

    y_train = train.y.astype(int)
    y_test = test.y.astype(int).to_numpy()

    s_train = train.spread.to_numpy()
    s_test = test.spread.to_numpy()

    dates = test.delivery_date.to_numpy()
    times = test.interval_start_utc.to_numpy()
    risk = X_test.spread_std28.to_numpy()

    assert np.isfinite(risk).all()

    majority = int(y_train.mean() >= 0.5)

    all_trades = []
    class_metrics = []
    regression_metrics = []

    # --------------------------------------------
    # Common evaluation helpers
    # --------------------------------------------

    def add_strategy(
        name, prediction, action, volume, probability=None
    ):
        prediction = np.asarray(prediction)
        action = np.asarray(action, dtype=int)
        volume = np.asarray(volume, dtype=float)

        assert all(
            len(a) == len(test)
            for a in (prediction, action, volume)
        )

        assert set(np.unique(action)).issubset({-1, 0, 1})
        assert np.all(np.isfinite(volume))
        assert np.all(volume >= 0)
        assert np.all(action[volume == 0] == 0)
        assert np.all(action[volume > 0] != 0)

        profit_c0 = volume * action * s_test
        profit_c1 = profit_c0 - COST * volume

        probs = (
            np.full(len(test), np.nan)
            if probability is None
            else np.asarray(probability)
        )

        all_trades.append(pd.DataFrame({
            "strategy": name,
            "delivery_date": dates,
            "interval_start_utc": times,
            "prediction": prediction,
            "prob_positive": probs,
            "action": action,
            "volume": volume,
            "spread": s_test,
            "profit_c0": profit_c0,
            "profit_c1": profit_c1
        }))

    def add_direction(name, pred, probability=None):
        pred = np.asarray(pred, dtype=int)

        add_strategy(
            name, pred, 2 * pred - 1,
            np.ones(len(test)), probability
        )

        if probability is not None:
            class_metrics.append({
                "strategy": name,
                "accuracy": accuracy_score(y_test, pred),
                "balanced_accuracy": balanced_accuracy_score(
                    y_test, pred
                ),
                "roc_auc": roc_auc_score(y_test, probability),
                "log_loss": log_loss(
                    y_test, probability, labels=[0, 1]
                )
            })

    # --------------------------------------------
    # Part 1: Baselines
    # --------------------------------------------

    add_direction(
        "baseline_always_inc",
        np.ones(len(test), dtype=int)
    )

    add_direction(
        "baseline_always_dec",
        np.zeros(len(test), dtype=int)
    )

    for lag in (2, 7):
        history = X_test[f"spread_lag{lag}"]

        pred = np.where(
            history.isna(),
            majority,
            history.to_numpy() > 0
        ).astype(int)

        add_direction(
            f"baseline_persistence_d{lag}", pred
        )

    seasonal = (
        train.groupby(["weekday", "hour_ending"]).y
        .mean().ge(0.5).astype(int)
    )

    index = pd.MultiIndex.from_frame(
        test[["weekday", "hour_ending"]]
    )

    seasonal_pred = (
        seasonal.reindex(index)
        .fillna(majority)
        .astype(int)
        .to_numpy()
    )

    add_direction(
        "baseline_seasonal_majority", seasonal_pred
    )

    # --------------------------------------------
    # Part 1: Classification
    # --------------------------------------------

    class_prob = None

    for name, model in get_classifiers().items():
        print("Fitting classifier:", name)

        model.fit(X_train, y_train)

        prob = model.predict_proba(X_test)[:, 1]
        pred = (prob >= 0.5).astype(int)

        add_direction(f"classifier_{name}", pred, prob)

        if name == "gradient_boosting":
            class_prob = prob

    rolling = X_test.spread_mean28

    heuristic = np.where(
        rolling.isna(),
        majority,
        rolling.to_numpy() > 0
    ).astype(int)

    add_direction("heuristic_rolling_mean", heuristic)

    # --------------------------------------------
    # Part 2: Signed regression
    # --------------------------------------------

    # Thresholds selected using 2024 validation.
    selected_tau = {
        "ridge_squared": 10,
        "boost_squared": 2,
        "boost_absolute": 5
    }

    signed_predictions = {}

    for name, model in get_regressors().items():
        print("Fitting signed regression:", name)

        model.fit(X_train, s_train)
        pred = model.predict(X_test)

        signed_predictions[name] = pred

        regression_metrics.append({
            "view": "signed",
            "model": name,
            "mae": mean_absolute_error(s_test, pred),
            "rmse": np.sqrt(
                mean_squared_error(s_test, pred)
            )
        })

        threshold = COST + selected_tau[name]

        action = np.where(
            pred > threshold, 1,
            np.where(pred < -threshold, -1, 0)
        )

        add_strategy(
            f"regression_{name}_tau{selected_tau[name]}",
            pred,
            action,
            (action != 0).astype(float)
        )

    # --------------------------------------------
    # Part 2: Unsigned magnitude
    # --------------------------------------------

    magnitude_predictions = {}

    for name, model in get_regressors().items():
        print("Fitting unsigned magnitude:", name)

        model.fit(X_train, np.abs(s_train))

        pred = np.maximum(
            model.predict(X_test), 0
        )

        magnitude_predictions[name] = pred

        regression_metrics.append({
            "view": "magnitude",
            "model": name,
            "mae": mean_absolute_error(
                np.abs(s_test), pred
            ),
            "rmse": np.sqrt(
                mean_squared_error(np.abs(s_test), pred)
            )
        })

    # --------------------------------------------
    # Part 2: Trading-volume sizing
    # --------------------------------------------

    size_signals = {
        "signed_regression":
            signed_predictions["boost_absolute"],

        "classifier_magnitude":
            (2 * class_prob - 1)
            * magnitude_predictions["boost_absolute"]
    }

    for view, signal in size_signals.items():

        _, sizing_trades = evaluate_sizing(
            view, signal, s_test, risk, dates, times
        )

        for frame in sizing_trades:
            method = frame["sizing"].iloc[0]

            add_strategy(
                f"sizing_{view}_{method}",
                signal,
                frame["action"].to_numpy(),
                frame["volume"].to_numpy()
            )

    # --------------------------------------------
    # Part 3: Daily budget allocation
    # --------------------------------------------

    mu = signed_predictions["boost_absolute"]

    # Covariance estimated using 2023 only.
    covariance = estimate_covariance(train)

    alloc_names = [
        "scaled_equal",
        "scaled_edge",
        "scaled_risk",
        "knapsack",
        *[
            f"risk_lambda_{lam:g}"
            for lam in RISK_LAMBDAS
        ]
    ]

    allocated = {
        name: np.zeros(len(test))
        for name in alloc_names
    }

    for _, group in test.groupby(
        "delivery_date", sort=True
    ):
        ix = group.index.to_numpy()

        m = mu[ix]
        action = np.where(m >= 0, 1, -1)
        edge = np.abs(m) - COST

        r = np.maximum(risk[ix], 5)
        hours = group.hour_ending.to_numpy()

        daily_alloc = {
            "scaled_equal": allocate_scaled(
                np.ones(len(ix))
            ),
            "scaled_edge": allocate_scaled(
                np.maximum(edge, 0.01)
            ),
            "scaled_risk": allocate_scaled(
                np.maximum(edge, 0.01) / r**2
            ),
            "knapsack": allocate_knapsack(edge)
        }

        for lam in RISK_LAMBDAS:
            daily_alloc[f"risk_lambda_{lam:g}"] = (
                allocate_risk_aware(
                    edge, action, hours, covariance, lam
                )
            )

        for name, volume in daily_alloc.items():
            volume = np.asarray(volume, dtype=float)

            assert np.all(volume >= -1e-8)
            assert volume.max() <= HOURLY_CAP + 1e-6
            assert volume.sum() <= BUDGET + 1e-5

            if name.startswith("scaled_"):
                assert np.isclose(
                    volume.sum(), BUDGET, atol=1e-5
                )

            if name == "knapsack":
                assert np.allclose(volume, np.rint(volume))

            allocated[name][ix] = volume

    for name, volume in allocated.items():
        volume[volume < 1e-8] = 0.0

        actions = np.where(
            volume > 0,
            np.where(mu >= 0, 1, -1),
            0
        )

        add_strategy(
            f"allocation_{name}",
            mu, actions, volume
        )

    # --------------------------------------------
    # Export all 2025 test trades
    # --------------------------------------------

    trades = pd.concat(
        all_trades, ignore_index=True
    )

    assert set(trades.delivery_date.dt.year) == {2025}

    trades.to_csv(
        OUTPUT / "project2_test_trades_2025.csv",
        index=False
    )

    # --------------------------------------------
    # Calculate final evaluation metrics
    # --------------------------------------------

    metrics = []
    worst_frames = []
    cumulative = {}

    for name, group in trades.groupby(
        "strategy", sort=True
    ):
        active = group.volume > 0

        daily = (
            group.groupby(
                "delivery_date", sort=True
            ).profit_c1.sum()
        )

        pnl = daily.to_numpy()

        total_mwh = group.volume.sum()
        total_profit = group.profit_c1.sum()

        daily_std = pnl.std(ddof=1)

        worst = daily.nsmallest(5)

        worst_frames.append(pd.DataFrame({
            "strategy": name,
            "delivery_date": worst.index,
            "profit_c1": worst.to_numpy()
        }))

        metrics.append({
            "strategy": name,
            "profit_c0": group.profit_c0.sum(),
            "profit_c1": total_profit,
            "mwh": total_mwh,

            "profit_per_mwh_c1": (
                total_profit / total_mwh
                if total_mwh else np.nan
            ),

            "n_trades": int(active.sum()),

            "win_rate_c1": (
                (group.loc[active, "profit_c1"] > 0).mean()
                if active.any() else np.nan
            ),

            "budget_used_pct": (
                100 * total_mwh / (BUDGET * len(daily))
                if name.startswith("allocation_")
                else np.nan
            ),

            "annualized_return_on_assumed_capital": (
                (total_profit / ASSUMED_CAPITAL)
                * 365 / len(daily)
            ),

            "annualized_pnl_sharpe": (
                np.sqrt(365) * pnl.mean() / daily_std
                if daily_std > 0 else np.nan
            ),

            "max_drawdown": max_drawdown(pnl),

            "top5_profit_share": (
                np.sort(pnl)[-5:].sum() / total_profit
                if total_profit > 0 else np.nan
            )
        })

        cumulative[name] = daily.cumsum()

    summary = (
        pd.DataFrame(metrics)
        .sort_values("profit_c1", ascending=False)
    )

    summary.to_csv(
        OUTPUT / "project2_test_summary_2025.csv",
        index=False
    )

    pd.concat(
        worst_frames, ignore_index=True
    ).to_csv(
        OUTPUT / "project2_test_worst_days_2025.csv",
        index=False
    )

    pd.DataFrame(class_metrics).to_csv(
        OUTPUT / "project2_test_classification_2025.csv",
        index=False
    )

    pd.DataFrame(regression_metrics).to_csv(
        OUTPUT / "project2_test_regression_2025.csv",
        index=False
    )

    # --------------------------------------------
    # Cumulative net profit comparison
    # --------------------------------------------

    selected = [
        "baseline_always_inc",
        "classifier_gradient_boosting",
        "regression_boost_absolute_tau5",
        "allocation_scaled_edge",
        "allocation_knapsack",
        "allocation_risk_lambda_5"
    ]

    plt.figure(figsize=(12, 6))

    for name in selected:
        plt.plot(
            cumulative[name].index,
            cumulative[name].values,
            label=name
        )

    plt.axhline(
        0, color="black", linestyle="--"
    )

    plt.title(
        "2025 Test: Cumulative Daily Net Profit"
    )
    plt.xlabel("Delivery Date")
    plt.ylabel("Cumulative Profit ($), c = $1/MWh")
    plt.legend(fontsize=8)
    plt.grid(alpha=0.3)
    plt.tight_layout()

    plt.savefig(
        FIGURES / "project2_test_cumulative_2025.png",
        dpi=200
    )

    plt.close()

    print("\n2025 FINAL TEST — ALL STRATEGIES")
    print(summary.round(3).to_string(index=False))

    print("\nFinal test files saved successfully.")
    print("Do not retune models using 2025 results.")


if __name__ == "__main__":
    main()
