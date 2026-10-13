#include <iostream>
#include <vector>

#include "test.h"
#include "Simulation.h"

int main()
{
    /*
        Run Black-Scholes / implied volatility tests.
    */

    std::cout
        << "Running Interim Project 1 unit tests:"
        << std::endl;

    runBlackScholesTests();
    runSimulationTests();

    /*
        Task 1:
        Generate simulated stock-price paths.
    */

    std::cout << std::endl;
    std::cout
        << "Generating stock paths..."
        << std::endl;

    Simulation simulation(
        100.0,   // S0
        0.4,     // T
        0.05,    // mu
        0.24,    // sigma
        100,     // N
        1000     // number of paths
    );

    std::vector<std::vector<double> > stock_paths =
        simulation.generateStockPaths();

    simulation.writeStockPathsCSV(
        stock_paths,
        "outputs/simulated_stock_paths.csv"
    );

    std::cout
        << "Generated "
        << stock_paths.size()
        << " paths."
        << std::endl;

    std::cout
        << "Observations per path: "
        << stock_paths[0].size()
        << std::endl;

    std::cout
        << "Saved to outputs/simulated_stock_paths.csv"
        << std::endl;

    return 0;
}