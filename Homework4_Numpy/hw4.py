import numpy as np
import matplotlib.pyplot as plt


# ============================================================
# TASK 1: TRADING STRATEGY
# ============================================================

def generate_trading_results(prices):
    n = len(prices)
    if n == 0:
        return (
            np.array([], dtype=int),
            np.array([], dtype=int),
            np.array([], dtype=float),
            0.0
        )

    signals = np.zeros(n, dtype=int)
    positions = np.zeros(n, dtype=int)
    account_values = np.zeros(n, dtype=float)

    initial_cash = 10000.0
    cash = initial_cash

    current_shares = 0
    sh = 10

    for i in range(n):

        # Rule 3: On the final day, sell everything
        if i == n - 1:
            if current_shares > 0:
                signals[i] = -1

                cash += current_shares * prices[i]
                current_shares = 0

        # Rule 1: Price rises for 3 consecutive days
        elif (
            i >= 3
            and prices[i - 3] < prices[i - 2]
            and prices[i - 2] < prices[i - 1]
            and prices[i - 1] < prices[i]
        ):
            if current_shares < 2 * sh:
                signals[i] = 1

                cash -= sh * prices[i]
                current_shares += sh

        # Rule 2: Price falls for 2 consecutive days
        elif (
            i >= 2
            and prices[i - 2] > prices[i - 1]
            and prices[i - 1] > prices[i]
        ):
            if current_shares > 0:
                signals[i] = -1

                cash += current_shares * prices[i]
                current_shares = 0

        positions[i] = current_shares

        account_values[i] = (
            cash + current_shares * prices[i]
        )

    cumulative_pnl = account_values[-1] - initial_cash

    return signals, positions, account_values, cumulative_pnl


def trading_strategy(csv_file):
    # Read the CSV file using NumPy
    data = np.genfromtxt(
        csv_file,
        delimiter=",",
        names=True,
        dtype=None,
        encoding="utf-8"
    )

    # Extract dates and closing prices
    dates = data["Date"]
    prices = data["Close"]

    # Apply the trading strategy
    signals, positions, account_values, cumulative_pnl = (
        generate_trading_results(prices)
    )

    # Save required arrays to trading_results.csv
    results = np.column_stack((
        dates,
        signals,
        positions,
        account_values
    ))

    np.savetxt(
        "trading_results.csv",
        results,
        delimiter=",",
        fmt="%s",
        header="Date,Signal,Position,Account_Value",
        comments=""
    )

        # Cumulative PNL for each trading day
    cumulative_pnl_series = account_values - 10000.0

    # Convert dates for plotting
    plot_dates = dates.astype("datetime64[D]")

    # Plot cumulative PNL
    plt.figure(figsize=(10, 6))
    plt.plot(plot_dates, cumulative_pnl_series)

    plt.title("AAPL Trading Strategy - Cumulative PNL")
    plt.xlabel("Date")
    plt.ylabel("Cumulative PNL ($)")
    plt.grid(True)
    plt.tight_layout()

    plt.savefig("cumulative_pnl.png")
    plt.close()

    # Screen output of cumulative trading PNL
    print(f"Cumulative Trading PNL: ${cumulative_pnl:.2f}")

    return signals, positions, account_values, cumulative_pnl

def test_custom_trading_example():
    prices = np.array([
        10, 11, 12, 13, 14, 13,
        12, 11, 12, 13, 14, 15
    ])

    expected_signals = np.array([
        0, 0, 0, 1, 1, 0,
        -1, 0, 0, 0, 1, -1
    ])

    expected_positions = np.array([
        0, 0, 0, 10, 20, 20,
        0, 0, 0, 0, 10, 0
    ])

    signals, positions, account_values, cumulative_pnl = (
        generate_trading_results(prices)
    )

    assert np.array_equal(signals, expected_signals)
    assert np.array_equal(positions, expected_positions)
    assert cumulative_pnl == -20.0

    print("Custom trading strategy test passed.")

# ============================================================
# TASK 2: ENERGY MAZE
# ============================================================

def min_initial_energy(maze):
    rows, cols = maze.shape

    # dp[i][j] = minimum energy needed when entering cell (i, j)
    dp = np.zeros((rows, cols), dtype=int)

    # Bottom-right cell
    dp[rows - 1][cols - 1] = max(
        1,
        1 - maze[rows - 1][cols - 1]
    )

    # Last row: can only move right
    for j in range(cols - 2, -1, -1):
        dp[rows - 1][j] = max(
            1,
            dp[rows - 1][j + 1] - maze[rows - 1][j]
        )

    # Last column: can only move down
    for i in range(rows - 2, -1, -1):
        dp[i][cols - 1] = max(
            1,
            dp[i + 1][cols - 1] - maze[i][cols - 1]
        )

    # Remaining cells
    for i in range(rows - 2, -1, -1):
        for j in range(cols - 2, -1, -1):

            minimum_next_energy = min(
                dp[i + 1][j],
                dp[i][j + 1]
            )

            dp[i][j] = max(
                1,
                minimum_next_energy - maze[i][j]
            )

    return dp[0][0]

def test_custom_energy_maze():
    custom_maze = np.array([
        [1, -4,  5],
        [-2, -3, -1]
    ])

    result = min_initial_energy(custom_maze)

    assert result == 4

    # Edge case: single positive cell
    assert min_initial_energy(np.array([[5]])) == 1

    # Edge case: single negative cell
    assert min_initial_energy(np.array([[-5]])) == 6

    print("Custom energy maze tests passed.")

# ============================================================
# MAIN / TESTS
# ============================================================
if __name__ == "__main__":

    # --------------------------------------------------------
    # Task 1: Professor's example
    # --------------------------------------------------------

    example_prices = np.array([
        98, 100, 102, 104, 103, 101, 99,
        100, 102, 104, 106, 107, 105
    ])

    signals, positions, account_values, cumulative_pnl = (
        generate_trading_results(example_prices)
    )

    print("Professor's Example")
    print("-------------------")

    print("Signals:")
    print(signals)

    print("\nPositions:")
    print(positions)

    print("\nAccount Values:")
    print(account_values)

    print("\nCumulative PNL:")
    print(cumulative_pnl)

    print("\nCustom Task 1 Unit Test")
    print("-----------------------")

    test_custom_trading_example()

    # --------------------------------------------------------
    # Task 1: AAPL data
    # --------------------------------------------------------

    print("\nAAPL Trading Strategy")
    print("---------------------")

    trading_strategy("aapl.csv")

        # --------------------------------------------------------
    # Task 2: Professor's example
    # --------------------------------------------------------

    print("\nEnergy Maze - Professor's Example")
    print("---------------------------------")

    maze = np.array([
        [-2, -3,  3],
        [-5, -10, 1],
        [10, 30, -5]
    ])

    initial_energy = min_initial_energy(maze)

    print("Maze:")
    print(maze)

    print("\nMinimum Initial Energy:")
    print(initial_energy)

    print("\nCustom Task 2 Unit Test")
    print("-----------------------")

    test_custom_energy_maze()