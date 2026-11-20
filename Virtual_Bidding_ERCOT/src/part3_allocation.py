
from pathlib import Path

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

import osqp
from scipy import sparse

from part1_baselines import load_data
from part1_features import build_features
from part2_regression import get_models


OUTPUT = Path("outputs")
FIGURES = Path("figures")

BUDGET = 24.0
HOURLY_CAP = 4.0
COST = 1.0
TAU = 5.0

RISK_LAMBDAS = [0.1, 1.0, 5.0]


def allocate_scaled(weights):
    """Use the full daily budget with an hourly cap."""
    w = np.maximum(np.asarray(weights, dtype=float), 1e-8)

    if len(w) * HOURLY_CAP < BUDGET:
        raise ValueError("Insufficient hourly capacity")

    low, high = 0.0, 1.0

    while np.minimum(HOURLY_CAP, high * w).sum() < BUDGET:
        high *= 2

    for _ in range(70):
        mid = (low + high) / 2
        volume = np.minimum(HOURLY_CAP, mid * w)

        if volume.sum() < BUDGET:
            low = mid
        else:
            high = mid

    return np.minimum(HOURLY_CAP, high * w)


def allocate_knapsack(edge):
    """
    Integer MWh allocation.
    Equal item weights make greedy sorting optimal.
    """
    volumes = np.zeros(len(edge))
    remaining = int(BUDGET)

    order = np.argsort(-edge, kind="stable")

    for i in order:
        if edge[i] <= TAU or remaining == 0:
            break

        units = min(int(HOURLY_CAP), remaining)
        volumes[i] = units
        remaining -= units

    return volumes


def estimate_covariance(train):
    """Estimate hourly spread covariance using 2023 only."""
    repeated = (
        train["repeated"].astype(str).str.lower()
        .isin(["true", "y", "1"])
    )

    history = train.loc[~repeated]

    panel = history.pivot(
        index="delivery_date",
        columns="hour_ending",
        values="spread"
    ).reindex(columns=range(1, 25))

    # Fill the missing DST hour for covariance estimation.
    panel = panel.fillna(panel.median())

    cov = np.cov(panel.to_numpy(), rowvar=False)

    # Shrink off-diagonal correlations for stability.
    cov = 0.5 * cov + 0.5 * np.diag(np.diag(cov))

    # Normalize the risk penalty scale.
    scale = np.median(np.diag(cov))
    assert scale > 0

    return cov / scale


def project_daily_budget(values):
    """
    Project volumes onto the feasible trading region:

        0 <= volume[h] <= 4
        sum(volume) <= 24
    """
    values = np.asarray(values, dtype=float)

    projected = np.clip(values, 0.0, HOURLY_CAP)

    if projected.sum() <= BUDGET:
        return projected

    # Find a common adjustment that satisfies
    # the daily budget using binary search.
    low = 0.0
    high = float(np.max(values))

    for _ in range(50):
        mid = (low + high) / 2.0

        projected = np.clip(
            values - mid, 0.0, HOURLY_CAP
        )

        if projected.sum() > BUDGET:
            low = mid
        else:
            high = mid

    return np.clip(
        values - high, 0.0, HOURLY_CAP
    )



def allocate_risk_aware(edge, action, hours, covariance, lam):
    """
    Maximize risk-adjusted predicted profit:

        edge @ v - 0.5 * lam * v.T @ Sigma @ v

    Subject to:
        0 <= v[h] <= 4
        sum(v) <= 24

    Solved as a convex quadratic program using OSQP.
    """

    eligible = np.flatnonzero(edge > TAU)
    volumes = np.zeros(len(edge))

    if len(eligible) == 0:
        return volumes

    e = np.asarray(edge[eligible], dtype=float)
    a = np.asarray(action[eligible], dtype=float)
    h = np.asarray(hours[eligible], dtype=int) - 1

    n = len(eligible)

    # Direction-adjusted covariance matrix
    sigma = covariance[np.ix_(h, h)]
    sigma = sigma * np.outer(a, a)
    sigma = 0.5 * (sigma + sigma.T)

    # Small numerical regularization
    sigma += 1e-8 * np.eye(n)

    # OSQP minimizes:
    #     0.5 * v.T @ P @ v + q.T @ v
    #
    # Equivalent to maximizing our original objective.
    P = sparse.csc_matrix(lam * sigma)
    q = -e

    # Constraints:
    # 0 <= v <= 4
    # sum(v) <= 24
    A = sparse.vstack(
        [
            sparse.eye(n, format="csc"),
            sparse.csc_matrix(np.ones((1, n)))
        ],
        format="csc"
    )

    lower = np.concatenate([
        np.zeros(n),
        [-np.inf]
    ])

    upper = np.concatenate([
        np.full(n, HOURLY_CAP),
        [BUDGET]
    ])

    # Initialize quadratic programming solver
    solver = osqp.OSQP()

    solver.setup(
        P=P,
        q=q,
        A=A,
        l=lower,
        u=upper,
        verbose=False,
        eps_abs=1e-6,
        eps_rel=1e-6,
        max_iter=20000,
        polishing=False
    )

    result = solver.solve()

    # Verify successful optimization
    if result.info.status.lower() != "solved":
        raise RuntimeError(
            f"OSQP failed: {result.info.status}, "
            f"lambda={lam}, eligible_hours={n}"
        )

    solution = np.asarray(result.x)

    # Allow only small numerical constraint errors
    if (
        solution.min() < -1e-4
        or solution.max() > HOURLY_CAP + 1e-4
        or solution.sum() > BUDGET + 1e-4
    ):
        raise RuntimeError(
            "OSQP returned an infeasible allocation"
        )

    # Correct floating-point deviations
    solution = project_daily_budget(solution)

    volumes[eligible] = solution

    # Final hard checks
    assert np.all(volumes >= -1e-8)
    assert volumes.max() <= HOURLY_CAP + 1e-8
    assert volumes.sum() <= BUDGET + 1e-8
    assert np.all(np.isfinite(volumes))

    return volumes

def max_drawdown(daily_profit):
    """Maximum peak-to-trough loss in cumulative P&L."""
    cumulative = np.r_[0.0, np.cumsum(daily_profit)]
    running_peak = np.maximum.accumulate(cumulative)

    return (running_peak - cumulative).max()


def main():
    OUTPUT.mkdir(exist_ok=True)
    FIGURES.mkdir(exist_ok=True)

    df = load_data()

    # Reserve 2025 for the final out-of-sample test.
    df = df.loc[
        df["delivery_date"].dt.year <= 2024
    ].reset_index(drop=True)

    X = build_features(df)

    train_mask = df["delivery_date"].dt.year == 2023
    val_mask = df["delivery_date"].dt.year == 2024

    X_train = X.loc[train_mask]
    X_val = X.loc[val_mask].reset_index(drop=True)

    train = df.loc[train_mask].copy()
    val = df.loc[val_mask].copy().reset_index(drop=True)

    # Train only on 2023.
    model = get_models()["boost_absolute"]
    model.fit(X_train, train["spread"])

    val["predicted_spread"] = model.predict(X_val)
    val["volatility"] = X_val["spread_std28"].to_numpy()

    assert np.isfinite(val["predicted_spread"]).all()
    assert np.isfinite(val["volatility"]).all()

    covariance = estimate_covariance(train)

    trade_frames = []

    # Each delivery day is a separate budget problem.
    for day, group in val.groupby("delivery_date", sort=True):
        mu = group["predicted_spread"].to_numpy()
        spread = group["spread"].to_numpy()
        risk = np.maximum(group["volatility"].to_numpy(), 5.0)
        hours = group["hour_ending"].to_numpy()

        action = np.where(mu >= 0, 1, -1)
        edge = np.abs(mu) - COST

        # Approach A: full-budget scaled allocations.
        allocations = {
            "scaled_equal": allocate_scaled(
                np.ones(len(group))
            ),
            "scaled_edge": allocate_scaled(
                np.maximum(edge, 0.01)
            ),
            "scaled_risk": allocate_scaled(
                np.maximum(edge, 0.01) / risk**2
            ),

            # Approach B: integer knapsack.
            "knapsack": allocate_knapsack(edge)
        }

        # Approach C: risk-penalized optimization.
        for lam in RISK_LAMBDAS:
            name = f"risk_lambda_{lam:g}"

            allocations[name] = allocate_risk_aware(
                edge, action, hours, covariance, lam
            )

        for name, volume in allocations.items():
            volume = np.asarray(volume).copy()
            volume[volume < 1e-8] = 0.0

            # Hard trading constraints.
            assert (volume >= 0).all()
            assert volume.max() <= HOURLY_CAP + 1e-6
            assert volume.sum() <= BUDGET + 1e-5

            if name.startswith("scaled_"):
                assert np.isclose(
                    volume.sum(), BUDGET, atol=1e-5
                )

            if name == "knapsack":
                assert np.allclose(volume, np.rint(volume))

            actual_action = np.where(volume > 0, action, 0)

            profit_c0 = volume * actual_action * spread
            profit_c1 = profit_c0 - COST * volume

            trades = pd.DataFrame({
                "delivery_date": group["delivery_date"].to_numpy(),
                "interval_start_utc":
                    group["interval_start_utc"].to_numpy(),
                "strategy": name,
                "predicted_spread": mu,
                "expected_edge_proxy": edge,
                "action": actual_action,
                "volume": volume,
                "spread": spread,
                "profit_c0": profit_c0,
                "profit_c1": profit_c1
            })

            trade_frames.append(trades)

    trades = pd.concat(trade_frames, ignore_index=True)

    trades.to_csv(
        OUTPUT / "part3_trades_2024.csv", index=False
    )

    # Daily realized outcomes.
    daily = (
        trades.groupby(["strategy", "delivery_date"])
        .agg(
            profit_c0=("profit_c0", "sum"),
            profit_c1=("profit_c1", "sum"),
            mwh=("volume", "sum")
        )
        .reset_index()
    )

    results = []

    for name, group in trades.groupby("strategy"):
        active = group["volume"] > 1e-8

        daily_group = (
            daily[daily["strategy"] == name]
            .sort_values("delivery_date")
        )

        pnl = daily_group["profit_c1"].to_numpy()
        total_mwh = group["volume"].sum()
        total_profit = group["profit_c1"].sum()

        daily_std = pnl.std(ddof=1)

        sharpe = (
            np.sqrt(365) * pnl.mean() / daily_std
            if daily_std > 0 else np.nan
        )

        # Concentration of P&L in the five best days.
        best_five = np.sort(pnl)[-5:].sum()

        results.append({
            "strategy": name,
            "total_profit_c0": group["profit_c0"].sum(),
            "total_profit_c1": total_profit,
            "total_mwh": total_mwh,
            "profit_per_mwh": (
                total_profit / total_mwh
                if total_mwh > 0 else np.nan
            ),
            "n_trades": int(active.sum()),
            "win_rate": (
                (group.loc[active, "profit_c1"] > 0).mean()
                if active.any() else np.nan
            ),
            "budget_used_pct": (
                100 * total_mwh /
                (BUDGET * len(daily_group))
            ),
            "max_drawdown": max_drawdown(pnl),
            "annualized_pnl_sharpe": sharpe,
            "top5_profit_share": (
                best_five / total_profit
                if total_profit > 0 else np.nan
            )
        })

    summary = pd.DataFrame(results)

    summary.to_csv(
        OUTPUT / "part3_comparison_2024.csv",
        index=False
    )

    print("\n2024 DAILY ALLOCATION COMPARISON")
    print(summary.round(3).to_string(index=False))

    # Five worst trading days for each strategy.
    worst = (
        daily.sort_values(["strategy", "profit_c1"])
        .groupby("strategy", group_keys=False)
        .head(5)
    )

    worst.to_csv(
        OUTPUT / "part3_worst_days_2024.csv",
        index=False
    )

    print("\nFIVE WORST DAYS PER STRATEGY")
    print(worst.to_string(index=False))

    # Cumulative daily net P&L.
    cumulative = (
        daily.pivot(
            index="delivery_date",
            columns="strategy",
            values="profit_c1"
        )
        .sort_index()
        .cumsum()
    )

    cumulative.to_csv(
        OUTPUT / "part3_cumulative_2024.csv"
    )

    plt.figure(figsize=(12, 6))

    for name in cumulative.columns:
        plt.plot(cumulative.index, cumulative[name], label=name)

    plt.axhline(0, color="black", linestyle="--")
    plt.title("2024 Validation: Budget-Constrained Trading")
    plt.xlabel("Date")
    plt.ylabel("Cumulative Net Profit ($)")
    plt.legend()
    plt.grid(alpha=0.3)
    plt.tight_layout()

    plt.savefig(
        FIGURES / "part3_cumulative_2024.png",
        dpi=200
    )

    plt.close()

    print("\nPart 3 validation completed successfully")


if __name__ == "__main__":
    main()
