#pragma once

// Internal helpers shared by the Black-Scholes pricing and Greeks code.
// Not part of the public API.

#include "black_scholes.hpp"

#include <cmath>
#include <stdexcept>

namespace quant {
namespace detail {

inline double normal_cdf(double x) {
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

inline double normal_pdf(double x) {
    // 1 / sqrt(2 * pi)
    constexpr double inv_sqrt_2pi = 0.39894228040143267794;
    return inv_sqrt_2pi * std::exp(-0.5 * x * x);
}

inline void validate(const OptionParameters& option) {
    if (!std::isfinite(option.spot) ||
        !std::isfinite(option.strike) ||
        !std::isfinite(option.maturity) ||
        !std::isfinite(option.risk_free_rate) ||
        !std::isfinite(option.volatility)) {
        throw std::invalid_argument(
            "All option parameters must be finite."
        );
    }

    if (option.spot <= 0.0 ||
        option.strike <= 0.0 ||
        option.maturity <= 0.0 ||
        option.volatility <= 0.0) {
        throw std::invalid_argument(
            "Spot, strike, maturity and volatility must be positive."
        );
    }
}

inline double d1(const OptionParameters& option) {
    return (
        std::log(option.spot / option.strike) +
        (option.risk_free_rate +
         0.5 * option.volatility * option.volatility) *
            option.maturity
    ) / (option.volatility * std::sqrt(option.maturity));
}

inline double d2(const OptionParameters& option) {
    return d1(option) -
           option.volatility * std::sqrt(option.maturity);
}

inline double discount_factor(const OptionParameters& option) {
    return std::exp(-option.risk_free_rate * option.maturity);
}

} // namespace detail
} // namespace quant
