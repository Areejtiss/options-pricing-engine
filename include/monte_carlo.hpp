#pragma once

#include "black_scholes.hpp"
#include "option.hpp"
#include "payoff.hpp"

#include <cstddef>
#include <cstdint>

// Monte Carlo pricing of European payoffs under Black-Scholes dynamics.
//
// The spot at expiry is sampled exactly from geometric Brownian motion:
//   S_T = S * exp((r - sigma^2 / 2) * T + sigma * sqrt(T) * Z),  Z ~ N(0, 1)
// and the price is the discounted mean payoff exp(-rT) * E[payoff(S_T)].
//
// Results are reproducible for a given seed on a given standard library;
// different standard libraries may produce different random sequences.

namespace quant {

struct MonteCarloSettings {
    // Total number of simulated paths. Must be at least 2 without
    // antithetic variates, or even and at least 4 with them.
    std::size_t num_paths = 100000;
    std::uint64_t seed = 42;
    // Pair each draw Z with -Z to reduce variance.
    bool antithetic = true;
};

struct MonteCarloResult {
    double price;
    double standard_error;
    std::size_t num_paths;

    // 95% confidence interval for the price.
    double confidence_low() const { return price - 1.96 * standard_error; }
    double confidence_high() const { return price + 1.96 * standard_error; }
};

// Throws std::invalid_argument on invalid parameters or settings.
MonteCarloResult monte_carlo_price(
    const Payoff& payoff,
    const OptionParameters& parameters,
    const MonteCarloSettings& settings = {}
);

// Prices the option's vanilla payoff with its own parameters.
MonteCarloResult monte_carlo_price(
    const EuropeanOption& option,
    const MonteCarloSettings& settings = {}
);

} // namespace quant
