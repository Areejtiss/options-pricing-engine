#include "black_scholes.hpp"
#include "greeks.hpp"
#include "test_helpers.hpp"

#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using test::expect_invalid_argument;
using test::expect_near;
using test::expect_true;

using Pricer = double (*)(const quant::OptionParameters&);
using Bump = std::function<void(quant::OptionParameters&, double)>;

// Central finite difference of a price with respect to one parameter.
double finite_difference(
    Pricer price,
    const quant::OptionParameters& option,
    const Bump& bump,
    double h
) {
    auto up = option;
    auto down = option;
    bump(up, h);
    bump(down, -h);
    return (price(up) - price(down)) / (2.0 * h);
}

// Second-order central difference of a price with respect to spot.
double second_difference_spot(
    Pricer price,
    const quant::OptionParameters& option,
    double h
) {
    auto up = option;
    auto down = option;
    up.spot += h;
    down.spot -= h;
    return (price(up) - 2.0 * price(option) + price(down)) / (h * h);
}

void test_reference_values(const quant::OptionParameters& option) {
    // S = 100, K = 100, T = 1, r = 5%, sigma = 20%  =>  d1 = 0.35, d2 = 0.15
    expect_near("Call Delta reference value",
                quant::call_delta(option), 0.636831);
    expect_near("Put Delta reference value",
                quant::put_delta(option), -0.363169);
    expect_near("Gamma reference value",
                quant::gamma(option), 0.018762);
    expect_near("Vega reference value",
                quant::vega(option), 37.5240, 0.001);
    expect_near("Call Theta reference value",
                quant::call_theta(option), -6.41403, 0.001);
    expect_near("Put Theta reference value",
                quant::put_theta(option), -1.65788, 0.001);
    expect_near("Call Rho reference value",
                quant::call_rho(option), 53.2325, 0.001);
    expect_near("Put Rho reference value",
                quant::put_rho(option), -41.8905, 0.001);
}

void test_relationships(const quant::OptionParameters& option) {
    const double discounted_strike =
        option.strike *
        std::exp(-option.risk_free_rate * option.maturity);

    expect_near(
        "Call-put Delta relationship",
        quant::call_delta(option) - quant::put_delta(option),
        1.0
    );

    // From put-call parity: d(C - P)/dr = K * T * exp(-rT)
    expect_near(
        "Call-put Rho relationship",
        quant::call_rho(option) - quant::put_rho(option),
        option.strike * option.maturity *
            std::exp(-option.risk_free_rate * option.maturity)
    );

    // From put-call parity: theta_C - theta_P = -r * K * exp(-rT)
    expect_near(
        "Call-put Theta relationship",
        quant::call_theta(option) - quant::put_theta(option),
        -option.risk_free_rate * discounted_strike
    );

    expect_true("Call Delta is between zero and one",
                quant::call_delta(option) > 0.0 &&
                    quant::call_delta(option) < 1.0);
    expect_true("Put Delta is between minus one and zero",
                quant::put_delta(option) > -1.0 &&
                    quant::put_delta(option) < 0.0);
    expect_true("Gamma is positive", quant::gamma(option) > 0.0);
    expect_true("Vega is positive", quant::vega(option) > 0.0);
    expect_true("Call Rho is positive", quant::call_rho(option) > 0.0);
    expect_true("Put Rho is negative", quant::put_rho(option) < 0.0);
}

// Check each analytic Greek against a numerical derivative of the price.
void test_finite_differences(const quant::OptionParameters& option) {
    const Bump bump_spot = [](quant::OptionParameters& o, double h) {
        o.spot += h;
    };
    const Bump bump_volatility = [](quant::OptionParameters& o, double h) {
        o.volatility += h;
    };
    const Bump bump_rate = [](quant::OptionParameters& o, double h) {
        o.risk_free_rate += h;
    };
    // Theta is the derivative with respect to calendar time,
    // which is minus the derivative with respect to maturity.
    const Bump bump_time = [](quant::OptionParameters& o, double h) {
        o.maturity -= h;
    };

    expect_near("Call Delta matches finite difference",
                quant::call_delta(option),
                finite_difference(quant::call_price, option, bump_spot, 0.01));
    expect_near("Put Delta matches finite difference",
                quant::put_delta(option),
                finite_difference(quant::put_price, option, bump_spot, 0.01));
    expect_near("Gamma matches finite difference (call)",
                quant::gamma(option),
                second_difference_spot(quant::call_price, option, 0.01));
    expect_near("Gamma matches finite difference (put)",
                quant::gamma(option),
                second_difference_spot(quant::put_price, option, 0.01));
    expect_near("Vega matches finite difference",
                quant::vega(option),
                finite_difference(
                    quant::call_price, option, bump_volatility, 1e-5),
                0.001);
    expect_near("Call Theta matches finite difference",
                quant::call_theta(option),
                finite_difference(quant::call_price, option, bump_time, 1e-5),
                0.001);
    expect_near("Put Theta matches finite difference",
                quant::put_theta(option),
                finite_difference(quant::put_price, option, bump_time, 1e-5),
                0.001);
    expect_near("Call Rho matches finite difference",
                quant::call_rho(option),
                finite_difference(quant::call_price, option, bump_rate, 1e-5),
                0.001);
    expect_near("Put Rho matches finite difference",
                quant::put_rho(option),
                finite_difference(quant::put_price, option, bump_rate, 1e-5),
                0.001);
}

void test_invalid_inputs(const quant::OptionParameters& option) {
    const std::vector<std::pair<std::string, Pricer>> greeks{
        {"Call Delta", quant::call_delta},
        {"Put Delta", quant::put_delta},
        {"Gamma", quant::gamma},
        {"Vega", quant::vega},
        {"Call Theta", quant::call_theta},
        {"Put Theta", quant::put_theta},
        {"Call Rho", quant::call_rho},
        {"Put Rho", quant::put_rho},
    };

    for (const auto& [name, greek] : greeks) {
        auto invalid = option;
        invalid.volatility = 0.0;
        expect_invalid_argument(name + " rejects zero volatility", [&] {
            greek(invalid);
        });

        invalid = option;
        invalid.maturity = -1.0;
        expect_invalid_argument(name + " rejects negative maturity", [&] {
            greek(invalid);
        });

        invalid = option;
        invalid.spot = std::numeric_limits<double>::infinity();
        expect_invalid_argument(name + " rejects infinite spot", [&] {
            greek(invalid);
        });

        invalid = option;
        invalid.risk_free_rate = std::numeric_limits<double>::quiet_NaN();
        expect_invalid_argument(name + " rejects NaN rate", [&] {
            greek(invalid);
        });
    }
}

} // namespace

int main() {
    const quant::OptionParameters at_the_money{
        100.0, 100.0, 1.0, 0.05, 0.20
    };

    // 1. Known reference values
    test_reference_values(at_the_money);

    // 2. Relationships and signs
    test_relationships(at_the_money);

    // 3. Analytic Greeks against numerical derivatives,
    //    for at-the-money, in-the-money and out-of-the-money calls
    test_finite_differences(at_the_money);
    test_finite_differences({120.0, 100.0, 0.5, 0.03, 0.25});
    test_finite_differences({80.0, 100.0, 2.0, 0.01, 0.35});

    // 4. Invalid inputs
    test_invalid_inputs(at_the_money);

    return test::finish();
}
