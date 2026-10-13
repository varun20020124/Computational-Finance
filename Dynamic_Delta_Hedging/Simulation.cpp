#include "Simulation.h"

#include <cmath>
#include <fstream>
#include <random>
#include <stdexcept>

Simulation::Simulation(
    double S0,
    double T,
    double mu,
    double sigma,
    int steps,
    int number_of_paths)
    : S0(S0),
      T(T),
      mu(mu),
      sigma(sigma),
      steps(steps),
      number_of_paths(number_of_paths)
{
    if (S0 <= 0.0 ||
        T <= 0.0 ||
        sigma < 0.0 ||
        steps <= 0 ||
        number_of_paths <= 0)
    {
        throw std::invalid_argument(
            "Invalid simulation parameters.");
    }
}

std::vector<std::vector<double> >
Simulation::generateStockPaths(
    unsigned int seed) const
{
    double dt =
        T / static_cast<double>(steps);

    std::mt19937 generator(seed);

    std::normal_distribution<double>
        normal_distribution(0.0, 1.0);

    std::vector<std::vector<double> > paths(
        number_of_paths,
        std::vector<double>(steps + 1, 0.0));

    for (int path = 0;
         path < number_of_paths;
         path++)
    {
        paths[path][0] = S0;

        for (int step = 1;
             step <= steps;
             step++)
        {
            double previous_price =
                paths[path][step - 1];

            double Z =
                normal_distribution(generator);

            double next_price =
                previous_price
                + mu * previous_price * dt
                + sigma
                    * previous_price
                    * std::sqrt(dt)
                    * Z;

            paths[path][step] =
                next_price;
        }
    }

    return paths;
}

void Simulation::writeStockPathsCSV(
    const std::vector<std::vector<double> >& paths,
    const std::string& filename) const
{
    std::ofstream outfile(filename.c_str());

    if (!outfile.is_open())
    {
        throw std::runtime_error(
            "Could not open simulation output file.");
    }

    outfile << "path";

    for (int step = 0;
         step <= steps;
         step++)
    {
        outfile << ",t" << step;
    }

    outfile << "\n";

    for (std::size_t path = 0;
         path < paths.size();
         path++)
    {
        outfile << path;

        for (std::size_t step = 0;
             step < paths[path].size();
             step++)
        {
            outfile
                << ","
                << paths[path][step];
        }

        outfile << "\n";
    }

    outfile.close();
}

std::vector<std::vector<double> >
Simulation::generateOptionPricePaths(
    const std::vector<std::vector<double> >& stock_paths,
    const BlackScholesModel& model,
    double risk_free_rate) const
{
    double dt =
        T / static_cast<double>(steps);

    std::vector<std::vector<double> > option_paths(
        stock_paths.size(),
        std::vector<double>(steps + 1, 0.0));

    for (std::size_t path = 0;
         path < stock_paths.size();
         path++)
    {
        for (int step = 0;
             step <= steps;
             step++)
        {
            double tau =
                T - step * dt;

            if (tau < 0.0)
            {
                tau = 0.0;
            }

            option_paths[path][step] =
                model.price(
                    stock_paths[path][step],
                    risk_free_rate,
                    tau,
                    sigma);
        }
    }

    return option_paths;
}


void Simulation::writeOptionPathsCSV(
    const std::vector<std::vector<double> >& paths,
    const std::string& filename) const
{
    std::ofstream outfile(filename.c_str());

    if (!outfile.is_open())
    {
        throw std::runtime_error(
            "Could not open option-path output file.");
    }

    outfile << "path";

    for (int step = 0;
         step <= steps;
         step++)
    {
        outfile << ",t" << step;
    }

    outfile << "\n";

    for (std::size_t path = 0;
         path < paths.size();
         path++)
    {
        outfile << path;

        for (std::size_t step = 0;
             step < paths[path].size();
             step++)
        {
            outfile
                << ","
                << paths[path][step];
        }

        outfile << "\n";
    }

    outfile.close();
}