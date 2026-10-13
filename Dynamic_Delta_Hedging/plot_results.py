import csv
import matplotlib.pyplot as plt


def read_path_csv(filename):
    paths = []

    with open(filename, "r") as file:
        reader = csv.reader(file)

        # Skip header
        next(reader)

        for row in reader:
            # First column is the path number.
            values = [float(x) for x in row[1:]]
            paths.append(values)

    return paths


def read_hedging_errors(filename):
    errors = []

    with open(filename, "r") as file:
        reader = csv.reader(file)

        # Skip header
        next(reader)

        for row in reader:
            errors.append(float(row[1]))

    return errors


# --------------------------------------------------
# Read C++ output
# --------------------------------------------------

stock_paths = read_path_csv(
    "outputs/simulated_stock_paths.csv"
)

option_paths = read_path_csv(
    "outputs/simulated_option_paths.csv"
)

hedging_errors = read_hedging_errors(
    "outputs/hedging_errors.csv"
)


# --------------------------------------------------
# Time axis
#
# T = 0.4
# N = 100
# --------------------------------------------------

T = 0.4
N = 100

times = [
    i * T / N
    for i in range(N + 1)
]


# --------------------------------------------------
# Figure 1:
# First 100 simulated stock paths
# --------------------------------------------------

plt.figure(figsize=(10, 6))

for path in stock_paths[:100]:
    plt.plot(times, path, linewidth=0.8)

plt.xlabel("Time (years)")
plt.ylabel("Stock Price")
plt.title("100 Simulated Stock Price Paths")

plt.tight_layout()

plt.savefig(
    "plots/simulated_stock_paths.png",
    dpi=300
)

plt.close()


# --------------------------------------------------
# Figure 2:
# First 100 option-price paths
# --------------------------------------------------

plt.figure(figsize=(10, 6))

for path in option_paths[:100]:
    plt.plot(times, path, linewidth=0.8)

plt.xlabel("Time (years)")
plt.ylabel("Call Option Price")
plt.title("100 Simulated Black-Scholes Option Price Paths")

plt.tight_layout()

plt.savefig(
    "plots/simulated_option_paths.png",
    dpi=300
)

plt.close()


# --------------------------------------------------
# Figure 3:
# Distribution of terminal hedging errors
# --------------------------------------------------

plt.figure(figsize=(10, 6))

plt.hist(
    hedging_errors,
    bins=30,
    edgecolor="black"
)

plt.xlabel("Terminal Hedging Error")
plt.ylabel("Frequency")
plt.title("Distribution of Terminal Delta-Hedging Errors")

plt.tight_layout()

plt.savefig(
    "plots/hedging_error_distribution.png",
    dpi=300
)

plt.close()


# --------------------------------------------------
# Summary statistics
# --------------------------------------------------

mean_error = (
    sum(hedging_errors)
    / len(hedging_errors)
)

variance = (
    sum(
        (x - mean_error) ** 2
        for x in hedging_errors
    )
    / len(hedging_errors)
)

std_error = variance ** 0.5

print("Plots generated successfully.")
print()
print("Terminal hedging error statistics:")
print("Number of paths:", len(hedging_errors))
print("Mean:", mean_error)
print("Standard deviation:", std_error)
print("Minimum:", min(hedging_errors))
print("Maximum:", max(hedging_errors))