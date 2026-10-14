#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>

#include "MarketData.h"
#include "Option.h"
#include "BlackScholesModel.h"
#include "Simulation.h"
#include "DeltaHedger.h"
#include "test.h"


int main()
{
    /*
        --------------------------------------------------
        Unit Tests
        --------------------------------------------------
    */

    std::cout
        << "Running Interim Project 1 unit tests:"
        << std::endl;


    runBlackScholesTests();
    runSimulationTests();
    runDeltaHedgerTests();
    runMarketDataTests();


    /*
        --------------------------------------------------
        Task 1:
        Simulate stock-price paths
        --------------------------------------------------

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
        --------------------------------------------------
        Task 1:
        Generate Black-Scholes option-price paths
        --------------------------------------------------

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
        --------------------------------------------------
        Task 1:
        Dynamic Delta hedge
        --------------------------------------------------

        dt = T / N
           = 0.4 / 100
           = 0.004
    */

    std::cout << std::endl;

    std::cout
        << "Running dynamic Delta hedging..."
        << std::endl;

    const double T = 0.4;
    const int N = 100;

    const double dt =
        T / static_cast<double>(N);

    const double risk_free_rate =
        0.025;

    const double volatility =
        0.24;

    DeltaHedger hedger(dt);

    std::vector<double>
        terminal_hedging_errors;

    terminal_hedging_errors.reserve(
        stock_paths.size()
    );


    /*
        Apply the Delta hedge independently
        to each simulated path.
    */

    for (std::size_t path = 0;
         path < stock_paths.size();
         path++)
    {
        std::vector<double> deltas;

        deltas.reserve(N);

        /*
            Delta is required at t0 through t99.

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
        Save terminal cumulative hedging error
        for all 1000 simulated paths.
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


    /*
        --------------------------------------------------
        Task 2:
        Real GOOG market data
        --------------------------------------------------

        t0 = 2011-07-05
        tN = 2011-07-29
        T  = 2011-09-17
        K  = 500

        For each trading date:

        1. Read stock price S_i.
        2. Read option market value V_i.
        3. Read risk-free rate r_i.
        4. Calculate time to maturity.
        5. Calculate implied volatility.
        6. Calculate Delta.
        7. Calculate unhedged PNL.
        8. Calculate hedged PNL.
    */

    std::cout << std::endl;

    std::cout
        << "Running Task 2 using GOOG market data..."
        << std::endl;


    MarketData market_data(
        "interest.csv",
        "sec_GOOG.csv",
        "op_GOOG.csv"
    );


    /*
        Extract the required GOOG call contract.
    */

    std::vector<MarketDataPoint> goog_data =
        market_data.getOptionSeries(
            "2011-07-05",
            "2011-07-29",
            "2011-09-17",
            500.0,
            'C'
        );


    std::cout
        << "Matching GOOG observations: "
        << goog_data.size()
        << std::endl;


    /*
        Represent the GOOG option contract.
    */

    Option goog_call(
        500.0,
        "2011-09-17",
        'C'
    );

    BlackScholesModel goog_model(
        goog_call
    );


    /*
        Containers for Task 2 results.
    */

    std::vector<double>
        goog_stock_prices;

    std::vector<double>
        goog_option_prices;

    std::vector<double>
        goog_rates;

    std::vector<double>
        goog_time_to_maturity;

    std::vector<double>
        goog_implied_vols;

    std::vector<double>
        goog_deltas;

    std::vector<double>
        goog_pnl;


    /*
        Calculate implied volatility and Delta
        for every market date.
    */

    for (std::size_t i = 0;
         i < goog_data.size();
         i++)
    {
        double S =
            goog_data[i].stock_price;

        double V =
            goog_data[i].option_price;

        double r =
            goog_data[i].risk_free_rate;

        double tau =
            market_data.timeToMaturity(
                goog_data[i].date,
                "2011-09-17"
            );


        /*
            Infer implied volatility from
            the observed market option price.
        */

        double implied_vol =
            goog_model.impliedVolatility(
                S,
                r,
                tau,
                V
            );


        /*
            Calculate Delta using the IV
            corresponding to this date.
        */

        double current_delta =
            goog_model.delta(
                S,
                r,
                tau,
                implied_vol
            );


        goog_stock_prices.push_back(
            S
        );

        goog_option_prices.push_back(
            V
        );

        goog_rates.push_back(
            r
        );

        goog_time_to_maturity.push_back(
            tau
        );

        goog_implied_vols.push_back(
            implied_vol
        );

        goog_deltas.push_back(
            current_delta
        );
    }


    /*
        --------------------------------------------------
        Unhedged PNL

        PNL_i = V0 - Vi
        --------------------------------------------------
    */

    double initial_option_price =
        goog_option_prices[0];

    for (std::size_t i = 0;
         i < goog_option_prices.size();
         i++)
    {
        goog_pnl.push_back(
            initial_option_price
            - goog_option_prices[i]
        );
    }


    /*
        --------------------------------------------------
        Hedged PNL

        PNL with hedge = cumulative hedging error

        Real market hedge is rebalanced once
        per business day:

        dt = 1 / 252
        --------------------------------------------------
    */

    DeltaHedger real_data_hedger(
        1.0 / 252.0
    );

    std::vector<double> goog_hedged_pnl =
        real_data_hedger.calculateHedgingErrors(
            goog_stock_prices,
            goog_option_prices,
            goog_deltas,
            goog_rates
        );


    /*
        --------------------------------------------------
        Write outputs/result.csv
        --------------------------------------------------
    */

    std::ofstream result_file(
        "outputs/result.csv"
    );

    if (!result_file.is_open())
    {
        std::cerr
            << "Could not create outputs/result.csv"
            << std::endl;

        return 1;
    }


    /*
        Required Task 2 columns.
    */

    result_file
        << "date,"
        << "stock_price,"
        << "option_price,"
        << "implied_volatility,"
        << "delta,"
        << "PNL,"
        << "PNL_with_hedge"
        << "\n";


    result_file
        << std::fixed
        << std::setprecision(8);


    for (std::size_t i = 0;
         i < goog_data.size();
         i++)
    {
        result_file
            << goog_data[i].date
            << ","
            << goog_stock_prices[i]
            << ","
            << goog_option_prices[i]
            << ","
            << goog_implied_vols[i]
            << ","
            << goog_deltas[i]
            << ","
            << goog_pnl[i]
            << ","
            << goog_hedged_pnl[i]
            << "\n";
    }


    result_file.close();


    /*
        --------------------------------------------------
        Task 2 summary
        --------------------------------------------------
    */

    std::cout
        << "Calculated IV and Delta for "
        << goog_data.size()
        << " observations."
        << std::endl;

    std::cout
        << "Initial time to maturity: "
        << goog_time_to_maturity.front()
        << std::endl;

    std::cout
        << "Initial implied volatility: "
        << goog_implied_vols.front()
        << std::endl;

    std::cout
        << "Initial Delta: "
        << goog_deltas.front()
        << std::endl;

    std::cout
        << "Final unhedged PNL: "
        << goog_pnl.back()
        << std::endl;

    std::cout
        << "Final hedged PNL: "
        << goog_hedged_pnl.back()
        << std::endl;

    std::cout
        << "Saved to outputs/result.csv"
        << std::endl;


    return 0;
}
