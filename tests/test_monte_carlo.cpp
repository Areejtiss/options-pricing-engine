#include "black_scholes.hpp"
#include "monte_carlo.hpp"
#include "option.hpp"
#include "payoff.hpp"
#include "test_helpers.hpp"

#include <cmath>
#include <limits>
#include <string>

namespace {

using test::expect_invalid_argument;
using test::expect_near;
using test::expect_true;

// Monte Carlo estimates are random, so compare against exact values
// within a number of standard errors. With fixed seeds the tests are
// deterministic; 4 standard errors keeps them robust across the
// different random sequences produced by different standard libraries.
constexpr double z_tolerance = 4.0;

void expect_within_standard_errors(
    const std::string& test_name,
    const quant::MonteCarloResult& result,
    double expected
) {
    expect_near(
        test_name,
        result.price,
        expected,
        z_tolerance * result.standard_error
    );
}

double digital_call_price(const quant::OptionParameters& option) {
    const double d2 =
        (std::log(option.spot / option.strike) +
         (option.risk_free_rate -
          0.5 * option.volatility * option.volatility) *
             option.maturity) /
        (option.volatility * std::sqrt(option.maturity));
    return std::exp(-option.risk_free_rate * option.maturity) *
           0.5 * std::erfc(-d2 / std::sqrt(2.0));
}

void test_convergence(const quant::OptionParameters& parameters,
                      const std::string& label) {
    const quant::MonteCarloSettings settings{200000, 7, true};

    const auto call = quant::monte_carlo_price(
        quant::CallPayoff(parameters.strike), parameters, settings);
    const auto put = quant::monte_carlo_price(
        quant::PutPayoff(parameters.strike), parameters, settings);
    const auto digital = quant::monte_carlo_price(
        quant::DigitalCallPayoff(parameters.strike), parameters, settings);

    expect_within_standard_errors(
        "Call converges to Black-Scholes (" + label + ")",
        call, quant::call_price(parameters));
    expect_within_standard_errors(
        "Put converges to Black-Scholes (" + label + ")",
        put, quant::put_price(parameters));
    expect_within_standard_errors(
        "Digital call converges to analytic price (" + label + ")",
        digital, digital_call_price(parameters));

    // Same seed means the same paths, so call - put is the
    // simulated forward S - K * exp(-rT).
    const double forward =
        parameters.spot -
        parameters.strike *
            std::exp(-parameters.risk_free_rate * parameters.maturity);
    expect_near(
        "Monte Carlo put-call parity (" + label + ")",
        call.price - put.price,
        forward,
        z_tolerance * (call.standard_error + put.standard_error)
    );
}

} // namespace

int main() {
    const quant::OptionParameters parameters{
        100.0, 100.0, 1.0, 0.05, 0.20
    };
    const quant::CallPayoff call_payoff(parameters.strike);

    // 1. Convergence to exact prices: at, in and out of the money
    test_convergence(parameters, "ATM");
    test_convergence({120.0, 100.0, 0.5, 0.03, 0.25}, "ITM");
    test_convergence({80.0, 100.0, 2.0, 0.01, 0.35}, "OTM");

    // 2. Result fields and confidence interval
    const auto result = quant::monte_carlo_price(call_payoff, parameters);
    expect_true("Path count is reported",
                result.num_paths ==
                    quant::MonteCarloSettings{}.num_paths);
    expect_true("Standard error is positive",
                result.standard_error > 0.0);
    expect_true("Standard error is small for 100k paths",
                result.standard_error < 0.05);
    expect_near("95% interval is centred on the price",
                0.5 * (result.confidence_low() + result.confidence_high()),
                result.price,
                1e-12);
    expect_near("95% interval is 1.96 standard errors wide",
                result.confidence_high() - result.confidence_low(),
                2.0 * 1.96 * result.standard_error,
                1e-12);

    // 3. Variance reduction and convergence rate
    const auto plain = quant::monte_carlo_price(
        call_payoff, parameters, {100000, 42, false});
    const auto antithetic = quant::monte_carlo_price(
        call_payoff, parameters, {100000, 42, true});
    expect_true("Antithetic variates reduce the standard error",
                antithetic.standard_error < plain.standard_error);

    const auto small = quant::monte_carlo_price(
        call_payoff, parameters, {50000, 1, false});
    const auto large = quant::monte_carlo_price(
        call_payoff, parameters, {200000, 1, false});
    expect_near("Four times the paths halves the standard error",
                large.standard_error / small.standard_error,
                0.5,
                0.05);

    // 4. Reproducibility
    const auto first = quant::monte_carlo_price(
        call_payoff, parameters, {10000, 123, true});
    const auto second = quant::monte_carlo_price(
        call_payoff, parameters, {10000, 123, true});
    const auto other_seed = quant::monte_carlo_price(
        call_payoff, parameters, {10000, 124, true});
    expect_true("Same seed gives identical results",
                first.price == second.price &&
                    first.standard_error == second.standard_error);
    expect_true("Different seed gives a different estimate",
                first.price != other_seed.price);

    // 5. EuropeanOption overload uses the option's payoff
    const quant::EuropeanOption put_option(
        quant::OptionType::Put, parameters);
    const auto via_option = quant::monte_carlo_price(put_option);
    const auto via_payoff = quant::monte_carlo_price(
        quant::PutPayoff(parameters.strike), parameters);
    expect_true("EuropeanOption overload matches payoff overload",
                via_option.price == via_payoff.price &&
                    via_option.standard_error ==
                        via_payoff.standard_error);

    // 6. Invalid settings and parameters
    expect_invalid_argument("Reject zero paths", [&] {
        quant::monte_carlo_price(call_payoff, parameters, {0, 42, false});
    });
    expect_invalid_argument("Reject a single path", [&] {
        quant::monte_carlo_price(call_payoff, parameters, {1, 42, false});
    });
    expect_invalid_argument("Reject a single antithetic pair", [&] {
        quant::monte_carlo_price(call_payoff, parameters, {2, 42, true});
    });
    expect_invalid_argument("Reject odd paths with antithetic", [&] {
        quant::monte_carlo_price(call_payoff, parameters, {1001, 42, true});
    });

    auto invalid = parameters;
    invalid.volatility = 0.0;
    expect_invalid_argument("Reject zero volatility", [&] {
        quant::monte_carlo_price(call_payoff, invalid);
    });

    invalid = parameters;
    invalid.spot = std::numeric_limits<double>::quiet_NaN();
    expect_invalid_argument("Reject NaN spot", [&] {
        quant::monte_carlo_price(call_payoff, invalid);
    });

    return test::finish();
}
