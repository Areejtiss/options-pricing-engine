#pragma once

// Minimal assertion helpers shared by the test executables.

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

namespace test {

inline int failures = 0;

inline void expect_near(
    const std::string& test_name,
    double actual,
    double expected,
    double tolerance = 0.0001
) {
    if (!(std::abs(actual - expected) <= tolerance)) {
        std::cerr << "[FAIL] " << test_name
                  << " | expected: " << expected
                  << ", actual: " << actual << '\n';
        ++failures;
    } else {
        std::cout << "[PASS] " << test_name << '\n';
    }
}

inline void expect_true(const std::string& test_name, bool condition) {
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

// Prints the summary and returns the process exit code.
inline int finish() {
    if (failures > 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}

} // namespace test
