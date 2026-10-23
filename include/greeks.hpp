#pragma once

#include "black_scholes.hpp"

// Black-Scholes Greeks for European options.
//
// Units:
//   Delta, Gamma - per 1.00 change in spot
//   Vega         - per 1.00 change in volatility (divide by 100 for per 1%)
//   Theta        - per year of calendar time (divide by 365 for per day)
//   Rho          - per 1.00 change in rate (divide by 100 for per 1%)
//
// All functions throw std::invalid_argument on invalid parameters.

namespace quant {

double call_delta(const OptionParameters& option);
double put_delta(const OptionParameters& option);

// Gamma and Vega are identical for calls and puts.
double gamma(const OptionParameters& option);
double vega(const OptionParameters& option);

double call_theta(const OptionParameters& option);
double put_theta(const OptionParameters& option);

double call_rho(const OptionParameters& option);
double put_rho(const OptionParameters& option);

} // namespace quant
