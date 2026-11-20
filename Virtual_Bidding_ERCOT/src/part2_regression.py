
from pathlib import Path

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

from sklearn.pipeline import Pipeline
from sklearn.impute import SimpleImputer
from sklearn.preprocessing import RobustScaler
from sklearn.linear_model import Ridge
from sklearn.ensemble import HistGradientBoostingRegressor
from sklearn.metrics import mean_absolute_error, mean_squared_error

from part1_baselines import load_data
from part1_features import build_features


OUTPUT = Path("outputs")
FIGURES = Path("figures")


def get_models():
    """Compare linear and tree models with different losses."""

    return {
        "ridge_squared": Pipeline([
            ("imputer", SimpleImputer(strategy="median")),
            ("scaler", RobustScaler()),
            ("model", Ridge(alpha=10.0))
        ]),

        "boost_squared": Pipeline([
            ("imputer", SimpleImputer(strategy="median")),
            ("model", HistGradientBoostingRegressor(
                loss="squared_error",
                max_iter=150,
                learning_rate=0.05,
                max_leaf_nodes=15,
                l2_regularization=1.0,
                random_state=42
            ))
        ]),

        "boost_absolute": Pipeline([
            ("imputer", SimpleImputer(strategy="median")),
            ("model", HistGradientBoostingRegressor(
                loss="absolute_error",
                max_iter=150,
                learning_rate=0.05,
                max_leaf_nodes=15,
                l2_regularization=1.0,
                random_state=42
            ))
        ])
    }


def evaluate_regression(actual, predicted):
    """Calculate regression errors."""

    return {
        "mae": mean_absolute_error(actual, predicted),
        "rmse": np.sqrt(mean_squared_error(actual, predicted))
    }


def trading_actions(predicted_spread, cost, tau):
    """Trade only when the predicted edge exceeds costs."""

    threshold = cost + tau

    return np.where(
        predicted_spread > threshold,
        1,
        np.where(predicted_spread < -threshold, -1, 0)
    )


def evaluate_trades(actual_spread, actions, cost):
    """Evaluate 1 MWh per active trade."""

    active = actions != 0
    volume = int(active.sum())

    profit = actions * actual_spread - cost * active

    return {
        "n_trades": volume,
        "total_mwh": volume,
        "total_profit": profit.sum(),
        "profit_per_mwh": (
            profit.sum() / volume if volume > 0 else np.nan
        )
    }


def main():
    OUTPUT.mkdir(exist_ok=True)
    FIGURES.mkdir(exist_ok=True)

    df = load_data()

    # Keep 2025 completely out of model development.
    df = df.loc[
        df["delivery_date"].dt.year <= 2024
    ].copy().reset_index(drop=True)

    features = build_features(df)

    train_mask = df["delivery_date"].dt.year == 2023
    val_mask = df["delivery_date"].dt.year == 2024

    X_train = features.loc[train_mask]
    X_val = features.loc[val_mask]

    train_spread = df.loc[train_mask, "spread"].to_numpy()
    val_spread = df.loc[val_mask, "spread"].to_numpy()

    val = df.loc[val_mask].copy()

    print("Training observations:", len(X_train))
    print("Validation observations:", len(X_val))
    print("Features:", X_train.shape[1])

    regression_results = []
    threshold_results = []

    predictions = pd.DataFrame({
        "delivery_date": val["delivery_date"].to_numpy(),
        "interval_start_utc": val["interval_start_utc"].to_numpy(),
        "actual_spread": val_spread
    })

    # --------------------------------------------------
    # 1. View A: unsigned magnitude
    #    View B: signed spread
    # --------------------------------------------------

    targets = {
        "magnitude": (
            np.abs(train_spread),
            np.abs(val_spread)
        ),
        "signed": (
            train_spread,
            val_spread
        )
    }

    for view, (y_train, y_val) in targets.items():

        for name, model in get_models().items():
            print(f"\nTraining {view}: {name}")

            # Pipelines fit preprocessing on 2023 only.
            model.fit(X_train, y_train)

            predicted = model.predict(X_val)

            # A magnitude cannot be negative.
            if view == "magnitude":
                predicted = np.maximum(predicted, 0.0)

            assert np.isfinite(predicted).all()

            errors = evaluate_regression(y_val, predicted)

            regression_results.append({
                "view": view,
                "model": name,
                **errors
            })

            predictions[f"{view}_{name}"] = predicted

            # --------------------------------------------------
            # 2. Signed forecasts become trading decisions
            # --------------------------------------------------

            if view == "signed":

                for cost in [0.0, 1.0]:
                    for tau in [0, 1, 2, 5, 10]:

                        actions = trading_actions(
                            predicted, cost, tau
                        )

                        trading = evaluate_trades(
                            val_spread, actions, cost
                        )

                        threshold_results.append({
                            "model": name,
                            "cost": cost,
                            "tau": tau,
                            **trading
                        })

    # --------------------------------------------------
    # 3. Regression comparison
    # --------------------------------------------------

    regression_df = pd.DataFrame(regression_results)

    regression_df.to_csv(
        OUTPUT / "part2_regression_2024.csv",
        index=False
    )

    print("\n2024 REGRESSION RESULTS")
    print(
        regression_df.sort_values(
            ["view", "mae"]
        ).round(3).to_string(index=False)
    )

    # --------------------------------------------------
    # 4. Trading threshold comparison
    # --------------------------------------------------

    threshold_df = pd.DataFrame(threshold_results)

    threshold_df.to_csv(
        OUTPUT / "part2_thresholds_2024.csv",
        index=False
    )

    print("\n2024 TRADING THRESHOLDS: c = $1/MWh")

    selected = threshold_df[
        threshold_df["cost"] == 1.0
    ]

    print(
        selected[
            [
                "model",
                "tau",
                "n_trades",
                "total_profit",
                "profit_per_mwh"
            ]
        ].round(3).to_string(index=False)
    )

    # --------------------------------------------------
    # 5. Save predictions
    # --------------------------------------------------

    predictions.to_csv(
        OUTPUT / "part2_predictions_2024.csv",
        index=False
    )

    # --------------------------------------------------
    # 6. Visualize profit versus threshold
    # --------------------------------------------------

    plt.figure(figsize=(10, 6))

    for name, group in selected.groupby("model"):
        group = group.sort_values("tau")

        plt.plot(
            group["tau"],
            group["total_profit"],
            marker="o",
            label=name
        )

    plt.axhline(0, color="black", linestyle="--")
    plt.title("Trading Threshold vs Validation Profit (2024)")
    plt.xlabel("Additional Threshold (tau), $/MWh")
    plt.ylabel("Total Net Profit ($)")
    plt.legend()
    plt.grid(alpha=0.3)
    plt.tight_layout()

    plt.savefig(
        FIGURES / "part2_threshold_profit_2024.png",
        dpi=200
    )

    plt.close()

    print("\nRegression results and predictions saved.")


if __name__ == "__main__":
    main()
