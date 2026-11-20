
from pathlib import Path

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

from sklearn.pipeline import Pipeline
from sklearn.impute import SimpleImputer
from sklearn.preprocessing import RobustScaler
from sklearn.linear_model import LogisticRegression
from sklearn.ensemble import HistGradientBoostingClassifier
from sklearn.metrics import (
    accuracy_score,
    balanced_accuracy_score,
    roc_auc_score,
    log_loss
)

from part1_baselines import load_data
from part1_features import build_features


OUTPUT = Path("outputs")
FIGURES = Path("figures")


def get_models():
    """Define the two classification models."""

    logistic = Pipeline([
        ("imputer", SimpleImputer(strategy="median")),
        ("scaler", RobustScaler()),
        ("model", LogisticRegression(
            max_iter=3000,
            random_state=42
        ))
    ])

    boosting = Pipeline([
        ("imputer", SimpleImputer(strategy="median")),
        ("model", HistGradientBoostingClassifier(
            max_iter=150,
            learning_rate=0.05,
            max_leaf_nodes=15,
            l2_regularization=1.0,
            random_state=42
        ))
    ])

    return {
        "logistic_regression": logistic,
        "gradient_boosting": boosting
    }


def evaluate_strategy(name, y_true, prediction, probability,
                      spread, dates):
    """Calculate classification and trading metrics."""

    action = np.where(prediction == 1, 1, -1)

    # Trade 1 MWh per hour
    profit_c0 = action * spread
    profit_c1 = profit_c0 - 1.0

    result = {
        "strategy": name,
        "accuracy": accuracy_score(y_true, prediction),
        "balanced_accuracy": balanced_accuracy_score(
            y_true, prediction
        ),
        "roc_auc": (
            roc_auc_score(y_true, probability)
            if probability is not None else np.nan
        ),
        "log_loss": (
            log_loss(y_true, probability, labels=[0, 1])
            if probability is not None else np.nan
        ),
        "total_mwh": len(prediction),
        "profit_c0": profit_c0.sum(),
        "profit_c1": profit_c1.sum(),
        "profit_per_mwh_c0": profit_c0.mean(),
        "profit_per_mwh_c1": profit_c1.mean()
    }

    trades = pd.DataFrame({
        "delivery_date": dates,
        "strategy": name,
        "prediction": prediction,
        "probability": (
            probability if probability is not None
            else np.full(len(prediction), np.nan)
        ),
        "action": action,
        "volume": 1.0,
        "spread": spread,
        "profit_c0": profit_c0,
        "profit_c1": profit_c1
    })

    return result, trades


def main():
    OUTPUT.mkdir(exist_ok=True)
    FIGURES.mkdir(exist_ok=True)

    # --------------------------------------------------
    # 1. Load data, reserving 2025 for final testing
    # --------------------------------------------------

    df = load_data()

    df = df.loc[
        df["delivery_date"].dt.year <= 2024
    ].copy().reset_index(drop=True)

    features = build_features(df)

    train_mask = df["delivery_date"].dt.year == 2023
    val_mask = df["delivery_date"].dt.year == 2024

    X_train = features.loc[train_mask]
    X_val = features.loc[val_mask]

    y_train = df.loc[train_mask, "y"].astype(int)
    y_val = df.loc[val_mask, "y"].astype(int).to_numpy()

    val = df.loc[val_mask].copy()

    spread = val["spread"].to_numpy()
    dates = val["delivery_date"].to_numpy()

    print("Training observations:", len(X_train))
    print("Validation observations:", len(X_val))
    print("Number of features:", X_train.shape[1])

    results = []
    all_trades = []

    # --------------------------------------------------
    # 2. Train learned classifiers
    # --------------------------------------------------

    models = get_models()

    for name, model in models.items():
        print(f"\nTraining {name}...")

        # The entire pipeline is fitted on 2023 only.
        # Imputation and scaling do not see 2024 data.
        model.fit(X_train, y_train)

        probability = model.predict_proba(X_val)[:, 1]
        prediction = (probability >= 0.5).astype(int)

        result, trades = evaluate_strategy(
            name, y_val, prediction, probability,
            spread, dates
        )

        results.append(result)
        all_trades.append(trades)

    # --------------------------------------------------
    # 3. Alternative framing: heuristic rule
    # --------------------------------------------------

    # A positive historical mean predicts INC.
    # Missing means fall back to the 2023 majority.
    training_majority = int(y_train.mean() >= 0.5)

    historical_mean = X_val["spread_mean28"]

    heuristic_prediction = np.where(
        historical_mean.isna(),
        training_majority,
        (historical_mean > 0).astype(int)
    )

    result, trades = evaluate_strategy(
        "rolling_mean_heuristic",
        y_val,
        heuristic_prediction,
        None,
        spread,
        dates
    )

    results.append(result)
    all_trades.append(trades)

    # --------------------------------------------------
    # 4. Save validation metrics
    # --------------------------------------------------

    results_df = pd.DataFrame(results)

    results_df.to_csv(
        OUTPUT / "part1_classification_2024.csv",
        index=False
    )

    print("\n2024 CLASSIFICATION RESULTS")
    print(results_df.round(3).to_string(index=False))

    # --------------------------------------------------
    # 5. Compare against existing baselines
    # --------------------------------------------------

    baseline_file = OUTPUT / "part1_baselines_2024.csv"

    if baseline_file.exists():
        baselines = pd.read_csv(baseline_file)

        columns = [
            "strategy",
            "accuracy",
            "profit_c0",
            "profit_c1",
            "profit_per_mwh_c1"
        ]

        comparison = pd.concat(
            [
                baselines[columns],
                results_df[columns]
            ],
            ignore_index=True
        )

        comparison = comparison.sort_values(
            "profit_c1", ascending=False
        )

        print("\nALL STRATEGIES: 2024 VALIDATION")
        print(comparison.round(3).to_string(index=False))

        comparison.to_csv(
            OUTPUT / "part1_all_strategies_2024.csv",
            index=False
        )

    # --------------------------------------------------
    # 6. Save trades and cumulative daily profit
    # --------------------------------------------------

    trades_df = pd.concat(all_trades, ignore_index=True)

    trades_df.to_csv(
        OUTPUT / "part1_validation_trades_2024.csv",
        index=False
    )

    daily = (
        trades_df.groupby(
            ["delivery_date", "strategy"]
        )["profit_c1"]
        .sum()
        .unstack()
        .sort_index()
    )

    cumulative = daily.cumsum()

    cumulative.to_csv(
        OUTPUT / "part1_classification_cumulative_2024.csv"
    )

    plt.figure(figsize=(12, 6))

    for name in cumulative.columns:
        plt.plot(
            cumulative.index,
            cumulative[name],
            label=name
        )

    plt.axhline(0, color="black", linestyle="--")
    plt.title("Part 1: Cumulative Validation Profit (2024)")
    plt.xlabel("Date")
    plt.ylabel("Cumulative Profit ($), c = $1/MWh")
    plt.legend()
    plt.grid(alpha=0.3)
    plt.tight_layout()

    plt.savefig(
        FIGURES / "part1_classification_profit_2024.png",
        dpi=200
    )

    plt.close()

    print("\nResults and figure saved successfully")


if __name__ == "__main__":
    main()
