
#include "black_scholes.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
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

} // namespace

int main() {
    const quant::OptionParameters option{
        100.0, 100.0, 1.0, 0.05, 0.20
    };

    const double call = quant::call_price(option);
    const double put = quant::put_price(option);
    const double discounted_strike =
        option.strike * std::exp(-option.risk_free_rate * option.maturity);

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

    // 4. Invalid inputs must throw std::invalid_argument
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

    if (failures > 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
