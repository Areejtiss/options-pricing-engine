#include "black_scholes.hpp"

#include <cmath>
#include <stdexcept>

namespace quant {
namespace {

double normal_cdf(double x) {
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

void validate(const OptionParameters& option) {
    if (option.spot <= 0.0 ||
        option.strike <= 0.0 ||
        option.maturity <= 0.0 ||
        option.volatility <= 0.0) {
        throw std::invalid_argument(
            "Spot, strike, maturity and volatility must be positive."
        );
    }
}

double d1(const OptionParameters& option) {
    return (
        std::log(option.spot / option.strike) +
        (option.risk_free_rate +
         0.5 * option.volatility * option.volatility) *
            option.maturity
    ) / (option.volatility * std::sqrt(option.maturity));
}

double d2(const OptionParameters& option) {
    return d1(option) -
           option.volatility * std::sqrt(option.maturity);
}

} // namespace

double call_price(const OptionParameters& option) {
    validate(option);

    return option.spot * normal_cdf(d1(option)) -
           option.strike *
               std::exp(-option.risk_free_rate * option.maturity) *
               normal_cdf(d2(option));
}

double put_price(const OptionParameters& option) {
    validate(option);

    return option.strike *
               std::exp(-option.risk_free_rate * option.maturity) *
               normal_cdf(-d2(option)) -
           option.spot * normal_cdf(-d1(option));
}

} // namespace quant