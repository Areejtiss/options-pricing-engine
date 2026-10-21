
#include "black_scholes.hpp"

#include <cmath>
#include <iostream>
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

} // namespace

int main() {
    const quant::OptionParameters option{
        100.0, 100.0, 1.0, 0.05, 0.20
    };

    expect_near(
        "Black-Scholes call price",
        quant::call_price(option),
        10.4506
    );

    expect_near(
        "Black-Scholes put price",
        quant::put_price(option),
        5.5735
    );

    if (failures > 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
