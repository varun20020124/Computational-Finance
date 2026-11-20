
from pathlib import Path

import numpy as np
import pandas as pd

from sklearn.metrics import (
    accuracy_score,
    roc_auc_score,
    mean_absolute_error,
    mean_squared_error
)

from part1_baselines import load_data
from part1_features import build_features
from part1_classification import get_models as get_classifiers
from part2_regression import get_models as get_regressors


OUTPUT = Path("outputs")
COST = 1.0
TAU = 5.0


def evaluate_models(X_train, X_val, y_train,
                    y_val, train_spread, val_spread):

    # Classifier: gradient boosting
    classifier = get_classifiers()["gradient_boosting"]
    classifier.fit(X_train, y_train)

    probability = classifier.predict_proba(X_val)[:, 1]
    prediction = (probability >= 0.5).astype(int)

    action = 2 * prediction - 1
    classifier_profit = np.sum(
        action * val_spread - COST
    )

    # Regression: signed spread, absolute-error boosting
    regressor = get_regressors()["boost_absolute"]
    regressor.fit(X_train, train_spread)

    predicted_spread = regressor.predict(X_val)

    threshold = COST + TAU
    trade_action = np.where(
        predicted_spread > threshold, 1,
        np.where(predicted_spread < -threshold, -1, 0)
    )

    active = trade_action != 0
    regression_profit = np.sum(
        trade_action * val_spread - COST * active
    )

    return {
        "classifier_accuracy": accuracy_score(
            y_val, prediction
        ),
        "classifier_auc": roc_auc_score(
            y_val, probability
        ),
        "classifier_profit": classifier_profit,
        "regression_mae": mean_absolute_error(
            val_spread, predicted_spread
        ),
        "regression_rmse": np.sqrt(
            mean_squared_error(
                val_spread, predicted_spread
            )
        ),
        "regression_trades": int(active.sum()),
        "regression_profit": regression_profit
    }


def main():
    OUTPUT.mkdir(exist_ok=True)

    df = load_data()
    df = df.loc[
        df["delivery_date"].dt.year <= 2024
    ].reset_index(drop=True)

    features = build_features(df)

    train = df["delivery_date"].dt.year == 2023
    val = df["delivery_date"].dt.year == 2024

    y_train = df.loc[train, "y"].astype(int)
    y_val = df.loc[val, "y"].astype(int)

    train_spread = df.loc[train, "spread"].to_numpy()
    val_spread = df.loc[val, "spread"].to_numpy()

    calendar_cols = [
        "hour_of_day", "day_of_week", "month",
        "is_weekend", "is_holiday", "is_repeated_hour"
    ]

    forecast_cols = [
        "load_fcst_system_mw", "load_fcst_north_mw",
        "wind_fcst_mw", "solar_fcst_mw",
        "net_load_mw", "net_load_ramp_mw"
    ]

    price_cols = [
        c for c in features.columns
        if c.startswith(("da_lag", "rt_lag", "spread_"))
    ]

    experiments = {
        "full_model": [],
        "no_price_history": price_cols,
        "no_forecasts": forecast_cols,
        "no_calendar": calendar_cols
    }

    results = []

    for name, removed in experiments.items():
        columns = [
            c for c in features.columns if c not in removed
        ]

        X_train = features.loc[train, columns]
        X_val = features.loc[val, columns]

        print(f"Running {name}: {len(columns)} features")

        metrics = evaluate_models(
            X_train, X_val, y_train, y_val,
            train_spread, val_spread
        )

        results.append({
            "experiment": name,
            "n_features": len(columns),
            **metrics
        })

    results_df = pd.DataFrame(results)

    results_df.to_csv(
        OUTPUT / "feature_ablation_2024.csv",
        index=False
    )

    print("\n2024 FEATURE ABLATION RESULTS")
    print(results_df.round(3).to_string(index=False))

    print("\nFeature ablation completed successfully")


if __name__ == "__main__":
    main()
