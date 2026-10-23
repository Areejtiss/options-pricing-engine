#include "payoff.hpp"
#include "test_helpers.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
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
    const quant::CallPayoff call(100.0);
    const quant::PutPayoff put(100.0);
    const quant::DigitalCallPayoff digital_call(100.0);
    const quant::DigitalPutPayoff digital_put(100.0);

    // 1. Vanilla payoffs in, at and out of the money
    expect_near("Call payoff in the money", call(120.0), 20.0);
    expect_near("Call payoff at the money", call(100.0), 0.0);
    expect_near("Call payoff out of the money", call(80.0), 0.0);
    expect_near("Call payoff at zero spot", call(0.0), 0.0);

    expect_near("Put payoff in the money", put(80.0), 20.0);
    expect_near("Put payoff at the money", put(100.0), 0.0);
    expect_near("Put payoff out of the money", put(120.0), 0.0);
    expect_near("Put payoff at zero spot", put(0.0), 100.0);

    // Call minus put is a forward: S - K, for any spot
    for (double spot : {0.0, 50.0, 100.0, 150.0}) {
        expect_near(
            "Call minus put equals S - K at spot " + std::to_string(spot),
            call(spot) - put(spot),
            spot - 100.0
        );
    }

    // 2. Digital payoffs, which pay nothing exactly at the strike
    expect_near("Digital call pays one above strike", digital_call(100.01), 1.0);
    expect_near("Digital call pays zero at strike", digital_call(100.0), 0.0);
    expect_near("Digital call pays zero below strike", digital_call(99.99), 0.0);

    expect_near("Digital put pays one below strike", digital_put(99.99), 1.0);
    expect_near("Digital put pays zero at strike", digital_put(100.0), 0.0);
    expect_near("Digital put pays zero above strike", digital_put(100.01), 0.0);

    // 3. Strike accessor, clone and factory
    expect_near("Strike accessor", call.strike(), 100.0);

    const std::unique_ptr<quant::Payoff> cloned = call.clone();
    expect_near("Cloned payoff matches original", (*cloned)(130.0), 30.0);

    const auto vanilla_call =
        quant::make_vanilla_payoff(quant::OptionType::Call, 100.0);
    const auto vanilla_put =
        quant::make_vanilla_payoff(quant::OptionType::Put, 100.0);
    expect_near("Factory builds a call payoff", (*vanilla_call)(110.0), 10.0);
    expect_near("Factory builds a put payoff", (*vanilla_put)(90.0), 10.0);

    expect_true(
        "Factory returns a CallPayoff for calls",
        dynamic_cast<const quant::CallPayoff*>(vanilla_call.get()) != nullptr
    );

    // 4. Invalid strikes
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();

    expect_invalid_argument("Reject zero strike", [] {
        quant::CallPayoff payoff(0.0);
    });
    expect_invalid_argument("Reject negative strike", [] {
        quant::PutPayoff payoff(-100.0);
    });
    expect_invalid_argument("Reject NaN strike", [&] {
        quant::DigitalCallPayoff payoff(nan);
    });
    expect_invalid_argument("Reject infinite strike", [&] {
        quant::DigitalPutPayoff payoff(infinity);
    });
    expect_invalid_argument("Factory rejects invalid strike", [] {
        quant::make_vanilla_payoff(quant::OptionType::Call, -1.0);
    });

    // 5. Invalid spot prices
    expect_invalid_argument("Call rejects negative spot", [&] {
        call(-1.0);
    });
    expect_invalid_argument("Put rejects NaN spot", [&] {
        put(nan);
    });
    expect_invalid_argument("Digital call rejects infinite spot", [&] {
        digital_call(infinity);
    });
    expect_invalid_argument("Digital put rejects negative spot", [&] {
        digital_put(-0.01);
    });

    return test::finish();
}
