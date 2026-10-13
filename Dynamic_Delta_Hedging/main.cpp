#include <iostream>
#include <vector>

#include "Option.h"
#include "BlackScholesModel.h"
#include "Simulation.h"
#include "test.h"

int main()
{
    /*
        Run unit tests.
    */

    std::cout
        << "Running Interim Project 1 unit tests:"
        << std::endl;

    runBlackScholesTests();
    runSimulationTests();


    /*
        Task 1:
        Generate simulated stock-price paths.

        Parameters required by the project:

        S0    = 100
        T     = 0.4
        mu    = 0.05
        sigma = 0.24
        N     = 100
        paths = 1000
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


    /*
        Task 1:
        Generate the corresponding
        Black-Scholes option-price paths.

        European Call:
        K     = 105
        T     = 0.4
        r     = 0.025
        sigma = 0.24
    */

    std::cout << std::endl;

    std::cout
        << "Generating option price paths..."
        << std::endl;

    Option simulated_call(
        105.0,
        "simulation_expiry",
        'C'
    );

    BlackScholesModel simulated_model(
        simulated_call
    );

    std::vector<std::vector<double> > option_paths =
        simulation.generateOptionPricePaths(
            stock_paths,
            simulated_model,
            0.025
        );

    simulation.writeOptionPathsCSV(
        option_paths,
        "outputs/simulated_option_paths.csv"
    );

    std::cout
        << "Generated "
        << option_paths.size()
        << " option price paths."
        << std::endl;

    std::cout
        << "Observations per option path: "
        << option_paths[0].size()
        << std::endl;

    std::cout
        << "Saved to outputs/simulated_option_paths.csv"
        << std::endl;


    /*
        Program ends only after
        all Task 1 outputs are generated.
    */

    return 0;
}