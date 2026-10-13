#include <iostream>
#include <vector>

#include "Option.h"
#include "BlackScholesModel.h"
#include "Simulation.h"
#include "DeltaHedger.h"
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
    runDeltaHedgerTests();


    /*
        Task 1:
        Generate simulated stock-price paths.

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
        Generate Black-Scholes option-price paths.

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
        Task 1:
        Construct the dynamic Delta hedge.

        dt = T / N = 0.4 / 100 = 0.004
    */

    std::cout << std::endl;

    std::cout
        << "Running dynamic Delta hedging..."
        << std::endl;

    const double T = 0.4;
    const int N = 100;
    const double dt =
        T / static_cast<double>(N);

    const double risk_free_rate = 0.025;
    const double volatility = 0.24;

    DeltaHedger hedger(dt);

    std::vector<double> terminal_hedging_errors;

    terminal_hedging_errors.reserve(
        stock_paths.size()
    );


    /*
        Apply the Delta hedge independently
        to each simulated stock path.
    */

    for (std::size_t path = 0;
         path < stock_paths.size();
         path++)
    {
        std::vector<double> deltas;

        deltas.reserve(N);

        /*
            We need Delta at t0 through t99.

            At t100 the option has matured,
            so no new hedge position is needed.
        */

        for (int step = 0;
             step < N;
             step++)
        {
            double tau =
                T - step * dt;

            double current_delta =
                simulated_model.delta(
                    stock_paths[path][step],
                    risk_free_rate,
                    tau,
                    volatility
                );

            deltas.push_back(
                current_delta
            );
        }

        std::vector<double> hedging_errors =
            hedger.calculateHedgingErrors(
                stock_paths[path],
                option_paths[path],
                deltas,
                risk_free_rate
            );

        terminal_hedging_errors.push_back(
            hedging_errors.back()
        );
    }


    /*
        Save the terminal cumulative hedging
        error from every simulated path.
    */

    hedger.writeTerminalErrorsCSV(
        terminal_hedging_errors,
        "outputs/hedging_errors.csv"
    );

    std::cout
        << "Calculated terminal hedging errors for "
        << terminal_hedging_errors.size()
        << " paths."
        << std::endl;

    std::cout
        << "Saved to outputs/hedging_errors.csv"
        << std::endl;


    return 0;
}