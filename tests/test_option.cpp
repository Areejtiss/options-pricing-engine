#include "black_scholes.hpp"
#include "greeks.hpp"
#include "option.hpp"
#include "test_helpers.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

namespace {

using test::expect_invalid_argument;
using test::expect_true;

// These values are computed exactly, so default to a tight tolerance.
void expect_near(
    const std::string& test_name,
    double actual,
    double expected,
    double tolerance = 1e-12
) {
    test::expect_near(test_name, actual, expected, tolerance);
}

} // namespace

int main() {
    const quant::OptionParameters parameters{
        100.0, 100.0, 1.0, 0.05, 0.20
    };

    const quant::EuropeanOption call(quant::OptionType::Call, parameters);
    const quant::EuropeanOption put(quant::OptionType::Put, parameters);

    // 1. Accessors
    expect_true("Call reports its type",
                call.type() == quant::OptionType::Call);
    expect_true("Put reports its type",
                put.type() == quant::OptionType::Put);
    expect_near("Parameters are stored",
                call.parameters().strike, parameters.strike);

    // 2. Methods match the free functions exactly
    expect_near("Call price", call.price(), quant::call_price(parameters));
    expect_near("Call delta", call.delta(), quant::call_delta(parameters));
    expect_near("Call gamma", call.gamma(), quant::gamma(parameters));
    expect_near("Call vega", call.vega(), quant::vega(parameters));
    expect_near("Call theta", call.theta(), quant::call_theta(parameters));
    expect_near("Call rho", call.rho(), quant::call_rho(parameters));

    expect_near("Put price", put.price(), quant::put_price(parameters));
    expect_near("Put delta", put.delta(), quant::put_delta(parameters));
    expect_near("Put gamma", put.gamma(), quant::gamma(parameters));
    expect_near("Put vega", put.vega(), quant::vega(parameters));
    expect_near("Put theta", put.theta(), quant::put_theta(parameters));
    expect_near("Put rho", put.rho(), quant::put_rho(parameters));

    // 3. greeks() bundles the individual Greeks
    const quant::Greeks call_greeks = call.greeks();
    expect_near("Call greeks().delta", call_greeks.delta, call.delta());
    expect_near("Call greeks().gamma", call_greeks.gamma, call.gamma());
    expect_near("Call greeks().vega", call_greeks.vega, call.vega());
    expect_near("Call greeks().theta", call_greeks.theta, call.theta());
    expect_near("Call greeks().rho", call_greeks.rho, call.rho());

    const quant::Greeks put_greeks = put.greeks();
    expect_near("Put greeks().delta", put_greeks.delta, put.delta());
    expect_near("Put greeks().theta", put_greeks.theta, put.theta());
    expect_near("Put greeks().rho", put_greeks.rho, put.rho());

    // 4. Payoff at expiry uses the option's strike and type
    expect_near("Call payoff at expiry", call.payoff(125.0), 25.0);
    expect_near("Call payoff out of the money", call.payoff(90.0), 0.0);
    expect_near("Put payoff at expiry", put.payoff(75.0), 25.0);
    expect_near("Put payoff out of the money", put.payoff(110.0), 0.0);

    // 5. Invalid parameters are rejected at construction
    auto invalid = parameters;
    invalid.volatility = 0.0;
    expect_invalid_argument("Reject zero volatility", [&] {
        quant::EuropeanOption option(quant::OptionType::Call, invalid);
    });

    invalid = parameters;
    invalid.maturity = -1.0;
    expect_invalid_argument("Reject negative maturity", [&] {
        quant::EuropeanOption option(quant::OptionType::Put, invalid);
    });

    invalid = parameters;
    invalid.risk_free_rate = std::numeric_limits<double>::quiet_NaN();
    expect_invalid_argument("Reject NaN rate", [&] {
        quant::EuropeanOption option(quant::OptionType::Call, invalid);
    });

    expect_invalid_argument("Payoff rejects negative spot", [&] {
        call.payoff(-1.0);
    });

    return test::finish();
}
