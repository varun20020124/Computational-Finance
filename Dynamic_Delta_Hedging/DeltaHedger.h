#ifndef DELTA_HEDGER_H
#define DELTA_HEDGER_H

#include <string>
#include <vector>

class DeltaHedger
{
private:
    double dt;

public:
    DeltaHedger(double dt);

    std::vector<double> calculateHedgingErrors(
        const std::vector<double>& stock_prices,
        const std::vector<double>& option_prices,
        const std::vector<double>& deltas,
        double risk_free_rate
    ) const;

    std::vector<double> calculateBankBalances(
        const std::vector<double>& stock_prices,
        const std::vector<double>& option_prices,
        const std::vector<double>& deltas,
        double risk_free_rate
    ) const;

    void writeTerminalErrorsCSV(
        const std::vector<double>& terminal_errors,
        const std::string& filename
    ) const;
};

#endif