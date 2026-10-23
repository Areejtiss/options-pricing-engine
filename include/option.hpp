#pragma once

#include "black_scholes.hpp"
#include "payoff.hpp"

// A European call or put priced with Black-Scholes.
// Greeks use the units documented in greeks.hpp.

namespace quant {

struct Greeks {
    double delta;
    double gamma;
    double vega;
    double theta;
    double rho;
};

class EuropeanOption {
public:
    // Throws std::invalid_argument if the parameters are invalid.
    EuropeanOption(OptionType type, const OptionParameters& parameters);

    OptionType type() const { return type_; }
    const OptionParameters& parameters() const { return parameters_; }

    double price() const;

    double delta() const;
    double gamma() const;
    double vega() const;
    double theta() const;
    double rho() const;
    Greeks greeks() const;

    // Value at expiry for the given spot price.
    double payoff(double spot_at_expiry) const;

private:
    OptionType type_;
    OptionParameters parameters_;
};

} // namespace quant
