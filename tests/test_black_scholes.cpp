
#include "black_scholes.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <initializer_list>
#include <limits>
#include <stdexcept>
#include <string>

namespace {

int failures = 0;

void expect_near(
    const std::string& test_name,
    double actual,
    double expected,
    double tolerance = 0.0001
) {
    if (std::abs(actual - expected) > tolerance) {
        std::cerr << "[FAIL] " << test_name
                  << " | expected: " << expected
                  << ", actual: " << actual << '\n';
        ++failures;
    } else {
        std::cout << "[PASS] " << test_name << '\n';
    }
}

void expect_true(const std::string& test_name, bool condition) {
    if (!condition) {
        std::cerr << "[FAIL] " << test_name << '\n';
        ++failures;
    } else {
        std::cout << "[PASS] " << test_name << '\n';
    }
}

template <typename Function>
void expect_invalid_argument(
    const std::string& test_name,
    Function function
) {
    try {
        function();
        std::cerr << "[FAIL] " << test_name
                  << " | expected std::invalid_argument\n";
        ++failures;
    } catch (const std::invalid_argument&) {
        std::cout << "[PASS] " << test_name << '\n';
    } catch (...) {
        std::cerr << "[FAIL] " << test_name
                  << " | unexpected exception type\n";
        ++failures;
    }
}

// Test NaN, positive infinity and negative infinity
// for every option parameter, for both call and put pricing.
void test_non_finite_inputs() {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();

    const quant::OptionParameters base{
        100.0, 100.0, 1.0, 0.05, 0.20
    };

    for (int parameter = 0; parameter < 5; ++parameter) {
        for (double invalid_value : {nan, infinity, -infinity}) {
            auto option = base;

            switch (parameter) {
                case 0:
                    option.spot = invalid_value;
                    break;
                case 1:
                    option.strike = invalid_value;
                    break;
                case 2:
                    option.maturity = invalid_value;
                    break;
                case 3:
                    option.risk_free_rate = invalid_value;
                    break;
                case 4:
                    option.volatility = invalid_value;
                    break;
            }

            const std::string name =
                "Reject non-finite parameter " +
                std::to_string(parameter);

            expect_invalid_argument(name + " (call)", [&] {
                quant::call_price(option);
            });

            expect_invalid_argument(name + " (put)", [&] {
                quant::put_price(option);
            });
        }
    }
}

} // namespace

int main() {
    const quant::OptionParameters option{
        100.0, 100.0, 1.0, 0.05, 0.20
    };

    const double call = quant::call_price(option);
    const double put = quant::put_price(option);
    const double discounted_strike =
        option.strike *
        std::exp(-option.risk_free_rate * option.maturity);

    // 1. Known reference prices
    expect_near("Black-Scholes call price", call, 10.4506);
    expect_near("Black-Scholes put price", put, 5.5735);

    // 2. Put-call parity: C - P = S - K * exp(-rT)
    expect_near(
        "Put-call parity",
        call - put,
        option.spot - discounted_strike
    );

    // 3. European option price bounds (non-negative interest rate)
    expect_true(
        "Call price is non-negative",
        call >= 0.0
    );

    expect_true(
        "Call price does not exceed spot",
        call <= option.spot
    );

    expect_true(
        "Call price respects its lower bound",
        call >= std::max(option.spot - discounted_strike, 0.0) - 0.0001
    );

    expect_true(
        "Put price is non-negative",
        put >= 0.0
    );

    expect_true(
        "Put price does not exceed discounted strike",
        put <= discounted_strike
    );

    expect_true(
        "Put price respects its lower bound",
        put >= std::max(discounted_strike - option.spot, 0.0) - 0.0001
    );

    // 4. Invalid inputs: zero and negative values
    auto invalid = option;

    invalid.spot = 0.0;
    expect_invalid_argument("Reject zero spot", [&] {
        quant::call_price(invalid);
    });

    invalid = option;
    invalid.spot = -100.0;
    expect_invalid_argument("Reject negative spot", [&] {
        quant::call_price(invalid);
    });

    invalid = option;
    invalid.strike = 0.0;
    expect_invalid_argument("Reject zero strike", [&] {
        quant::put_price(invalid);
    });

    invalid = option;
    invalid.strike = -100.0;
    expect_invalid_argument("Reject negative strike", [&] {
        quant::put_price(invalid);
    });

    invalid = option;
    invalid.maturity = 0.0;
    expect_invalid_argument("Reject zero maturity", [&] {
        quant::call_price(invalid);
    });

    invalid = option;
    invalid.maturity = -1.0;
    expect_invalid_argument("Reject negative maturity", [&] {
        quant::put_price(invalid);
    });

    invalid = option;
    invalid.volatility = 0.0;
    expect_invalid_argument("Reject zero volatility", [&] {
        quant::call_price(invalid);
    });

    invalid = option;
    invalid.volatility = -0.20;
    expect_invalid_argument("Reject negative volatility", [&] {
        quant::put_price(invalid);
    });

    // 5. Option Delta calculations
    const double call_delta = quant::call_delta(option);
    const double put_delta = quant::put_delta(option);

    expect_near(
        "Call Delta reference value",
        call_delta,
        0.636830651
    );

    expect_near(
        "Put Delta reference value",
        put_delta,
        -0.363169349
    );

    expect_near(
        "Call-put Delta relationship",
        call_delta - put_delta,
        1.0
    );

    expect_true(
        "Call Delta is between zero and one",
        call_delta > 0.0 && call_delta < 1.0
    );

    expect_true(
        "Put Delta is between minus one and zero",
        put_delta > -1.0 && put_delta < 0.0
    );

    invalid = option;
    invalid.volatility = 0.0;
    expect_invalid_argument("Call Delta rejects zero volatility", [&] {
        quant::call_delta(invalid);
    });

    invalid = option;
    invalid.spot = std::numeric_limits<double>::infinity();
    expect_invalid_argument("Put Delta rejects infinite spot", [&] {
        quant::put_delta(invalid);
    });

    // 6. Non-finite inputs: NaN, +infinity and -infinity


    // 5. Non-finite inputs: NaN, +infinity and -infinity
    test_non_finite_inputs();

    // Final test result
    if (failures > 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
