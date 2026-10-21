#pragma once

namespace quant {

struct OptionParameters {
    double spot;
    double strike;
    double maturity;
    double risk_free_rate;
    double volatility;
};

double call_price(const OptionParameters& option);
double put_price(const OptionParameters& option);

} // namespace quant