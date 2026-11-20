
def solve_knapsack(weights, profits, budget):
    """
    Solve a 0/1 knapsack problem using dynamic programming.

    Each trade has:
      - a collateral/risk requirement (weight)
      - an expected profit (value)
    """
    n = len(weights)

    dp = [
        [0] * (budget + 1)
        for _ in range(n + 1)
    ]

    for i in range(1, n + 1):
        weight = weights[i - 1]
        profit = profits[i - 1]

        for capacity in range(budget + 1):
            # Option 1: skip the trade
            dp[i][capacity] = dp[i - 1][capacity]

            # Option 2: take the trade
            if weight <= capacity:
                dp[i][capacity] = max(
                    dp[i][capacity],
                    profit + dp[i - 1][capacity - weight]
                )

    # Recover the selected trades
    chosen = []
    capacity = budget

    for i in range(n, 0, -1):
        if dp[i][capacity] != dp[i - 1][capacity]:
            chosen.append(i)
            capacity -= weights[i - 1]

    chosen.reverse()

    return dp[n][budget], chosen


def main():
    # Hypothetical trading opportunities
    weights = [2, 3, 4]   # Risk-budget units
    profits = [3, 4, 5]   # Expected profit in dollars

    budget = 6

    optimal_profit, chosen = solve_knapsack(
        weights, profits, budget
    )

    print("Risk budget:", budget)
    print("Selected trades:", chosen)
    print("Risk budget used:",
          sum(weights[i - 1] for i in chosen))
    print("Optimal expected profit:", optimal_profit)

    assert chosen == [1, 3]
    assert optimal_profit == 8

    print("Weighted knapsack test passed")


if __name__ == "__main__":
    main()
