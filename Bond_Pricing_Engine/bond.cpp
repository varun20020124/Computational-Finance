#include "bond.h"

#include <cmath>
#include <sstream>
#include <stdexcept>

namespace
{
    struct Date
    {
        int month;
        int day;
        int year;
    };

    bool parseDate(const std::string& date_string, Date& date)
    {
        std::stringstream ss(date_string);

        char slash1;
        char slash2;

        if (!(ss >> date.month >> slash1 >> date.day >> slash2 >> date.year))
        {
            return false;
        }

        if (slash1 != '/' || slash2 != '/')
        {
            return false;
        }

        if (date.month < 1 || date.month > 12)
        {
            return false;
        }

        if (date.day < 1 || date.day > 31)
        {
            return false;
        }

        return true;
    }

    int compareDates(const Date& first, const Date& second)
    {
        if (first.year != second.year)
        {
            return first.year < second.year ? -1 : 1;
        }

        if (first.month != second.month)
        {
            return first.month < second.month ? -1 : 1;
        }

        if (first.day != second.day)
        {
            return first.day < second.day ? -1 : 1;
        }

        return 0;
    }

    double yearFraction(const Date& start, const Date& end)
    {
        int month_difference =
            (end.year - start.year) * 12 +
            (end.month - start.month);

        double years = month_difference / 12.0;

        years += (end.day - start.day) / 365.0;

        return years;
    }

    double interpolateRate(
        double time,
        const std::vector<double>& maturities,
        const std::vector<double>& interest_rates)
    {
        if (maturities.empty() ||
            maturities.size() != interest_rates.size())
        {
            throw std::invalid_argument(
                "Invalid interest-rate curve.");
        }

        const double tolerance = 1e-10;

        // First check whether the exact maturity exists.
        for (std::size_t i = 0; i < maturities.size(); i++)
        {
            if (std::fabs(maturities[i] - time) < tolerance)
            {
                return interest_rates[i];
            }
        }

        bool lower_found = false;
        bool upper_found = false;

        double lower_time = 0.0;
        double upper_time = 0.0;

        double lower_rate = 0.0;
        double upper_rate = 0.0;

        for (std::size_t i = 0; i < maturities.size(); i++)
        {
            if (maturities[i] < time)
            {
                if (!lower_found || maturities[i] > lower_time)
                {
                    lower_time = maturities[i];
                    lower_rate = interest_rates[i];
                    lower_found = true;
                }
            }

            if (maturities[i] > time)
            {
                if (!upper_found || maturities[i] < upper_time)
                {
                    upper_time = maturities[i];
                    upper_rate = interest_rates[i];
                    upper_found = true;
                }
            }
        }

        // If the requested maturity is outside the curve,
        // use the nearest available rate.
        if (!lower_found && upper_found)
        {
            return upper_rate;
        }

        if (lower_found && !upper_found)
        {
            return lower_rate;
        }

        if (!lower_found && !upper_found)
        {
            throw std::invalid_argument(
                "Unable to interpolate interest rate.");
        }

        // Linear interpolation.
        double weight =
            (time - lower_time) /
            (upper_time - lower_time);

        return lower_rate +
               weight * (upper_rate - lower_rate);
    }
}

Bond::Bond()
    : expiration_date("0"),
      frequency(0.0),
      coupon_rate(0.0)
{
}

Bond::~Bond()
{
}

Bond::Bond(const Bond& other)
    : expiration_date(other.expiration_date),
      frequency(other.frequency),
      coupon_rate(other.coupon_rate)
{
}

Bond::Bond(
    std::string expiration_date,
    double frequency,
    double coupon_rate)
    : expiration_date(expiration_date),
      frequency(frequency),
      coupon_rate(coupon_rate)
{
}

std::string Bond::ToString() const
{
    std::ostringstream output;

    output << "Bond ("
           << expiration_date << ", "
           << frequency << ", "
           << coupon_rate << ")";

    return output.str();
}

double Bond::Price(
    double face_value,
    std::string valuation_date,
    const std::vector<double>& maturities,
    const std::vector<double>& interest_rates) const
{
    if (face_value <= 0.0)
    {
        throw std::invalid_argument(
            "Face value must be positive.");
    }

    if (frequency <= 0.0)
    {
        throw std::invalid_argument(
            "Frequency must be positive.");
    }

    Date valuation;
    Date maturity;

    if (!parseDate(valuation_date, valuation))
    {
        throw std::invalid_argument(
            "Invalid valuation date.");
    }

    if (!parseDate(expiration_date, maturity))
    {
        throw std::invalid_argument(
            "Invalid expiration date.");
    }

    int date_comparison =
        compareDates(valuation, maturity);

    // Homework convention:
    // price at maturity equals par value.
    if (date_comparison == 0)
    {
        return face_value;
    }

    // The bond has already matured.
    if (date_comparison > 0)
    {
        return 0.0;
    }

    double time_to_maturity =
        yearFraction(valuation, maturity);

    const double tolerance = 1e-10;

    /*
        Find the length of the first coupon period.

        Example:
            time to maturity = 4.2
            frequency        = 0.5

        remainder = 0.2

        Payment times:
            0.2, 0.7, 1.2, ..., 4.2
    */

    double first_period =
        std::fmod(time_to_maturity, frequency);

    if (first_period < tolerance)
    {
        first_period = frequency;
    }

    double price = 0.0;
    double payment_time = first_period;

    bool first_payment = true;

    while (payment_time < time_to_maturity - tolerance)
    {
        double coupon_period;

        if (first_payment)
        {
            coupon_period = first_period;
        }
        else
        {
            coupon_period = frequency;
        }

        double coupon =
            face_value *
            coupon_rate *
            coupon_period;

        double yield =
            interpolateRate(
                payment_time,
                maturities,
                interest_rates);

        double discount_factor =
            std::exp(
                -yield * payment_time);

        price += coupon * discount_factor;

        first_payment = false;

        payment_time += frequency;
    }

    /*
        Final payment includes:

        final coupon + face value
    */

    double final_coupon_period;

    if (first_payment)
    {
        final_coupon_period = first_period;
    }
    else
    {
        final_coupon_period = frequency;
    }

    double final_coupon =
        face_value *
        coupon_rate *
        final_coupon_period;

    double final_yield =
        interpolateRate(
            time_to_maturity,
            maturities,
            interest_rates);

    double final_discount_factor =
        std::exp(
            -final_yield * time_to_maturity);

    price +=
        (face_value + final_coupon) *
        final_discount_factor;

    return price;
}