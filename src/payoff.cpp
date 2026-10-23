#include "payoff.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace quant {
namespace {

void validate_spot(double spot) {
    if (!std::isfinite(spot) || spot < 0.0) {
        throw std::invalid_argument(
            "Spot at expiry must be finite and non-negative."
        );
    }
}

} // namespace

StrikePayoff::StrikePayoff(double strike) : strike_(strike) {
    if (!std::isfinite(strike) || strike <= 0.0) {
        throw std::invalid_argument(
            "Strike must be finite and positive."
        );
    }
}

double CallPayoff::operator()(double spot) const {
    validate_spot(spot);
    return std::max(spot - strike_, 0.0);
}

std::unique_ptr<Payoff> CallPayoff::clone() const {
    return std::make_unique<CallPayoff>(*this);
}

double PutPayoff::operator()(double spot) const {
    validate_spot(spot);
    return std::max(strike_ - spot, 0.0);
}

std::unique_ptr<Payoff> PutPayoff::clone() const {
    return std::make_unique<PutPayoff>(*this);
}

double DigitalCallPayoff::operator()(double spot) const {
    validate_spot(spot);
    return spot > strike_ ? 1.0 : 0.0;
}

std::unique_ptr<Payoff> DigitalCallPayoff::clone() const {
    return std::make_unique<DigitalCallPayoff>(*this);
}

double DigitalPutPayoff::operator()(double spot) const {
    validate_spot(spot);
    return spot < strike_ ? 1.0 : 0.0;
}

std::unique_ptr<Payoff> DigitalPutPayoff::clone() const {
    return std::make_unique<DigitalPutPayoff>(*this);
}

std::unique_ptr<Payoff> make_vanilla_payoff(OptionType type, double strike) {
    switch (type) {
        case OptionType::Call:
            return std::make_unique<CallPayoff>(strike);
        case OptionType::Put:
            return std::make_unique<PutPayoff>(strike);
    }
    throw std::invalid_argument("Unknown option type.");
}

} // namespace quant
