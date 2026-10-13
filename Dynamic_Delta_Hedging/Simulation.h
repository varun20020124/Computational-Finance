#ifndef SIMULATION_H
#define SIMULATION_H

#include <string>
#include <vector>
#include "BlackScholesModel.h"

class Simulation
{
private:
    double S0;
    double T;
    double mu;
    double sigma;
    int steps;
    int number_of_paths;

public:
    Simulation(
        double S0,
        double T,
        double mu,
        double sigma,
        int steps,
        int number_of_paths
    );

    std::vector<std::vector<double> >
    generateStockPaths(unsigned int seed = 42) const;

    void writeStockPathsCSV(
        const std::vector<std::vector<double> >& paths,
        const std::string& filename
    ) const;
    std::vector<std::vector<double> >
    generateOptionPricePaths(
        const std::vector<std::vector<double> >& stock_paths,
        const BlackScholesModel& model,
        double risk_free_rate
    ) const;

    void writeOptionPathsCSV(
        const std::vector<std::vector<double> >& paths,
        const std::string& filename
    ) const;
};

#endif