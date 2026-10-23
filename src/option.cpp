#include "option.hpp"

#include "bs_common.hpp"
#include "greeks.hpp"

namespace quant {

EuropeanOption::EuropeanOption(
    OptionType type,
    const OptionParameters& parameters
) : type_(type), parameters_(parameters) {
    detail::validate(parameters_);
}

double EuropeanOption::price() const {
    return type_ == OptionType::Call ? call_price(parameters_)
                                     : put_price(parameters_);
}

double EuropeanOption::delta() const {
    return type_ == OptionType::Call ? call_delta(parameters_)
                                     : put_delta(parameters_);
}

double EuropeanOption::gamma() const {
    return quant::gamma(parameters_);
}

double EuropeanOption::vega() const {
    return quant::vega(parameters_);
}

double EuropeanOption::theta() const {
    return type_ == OptionType::Call ? call_theta(parameters_)
                                     : put_theta(parameters_);
}

double EuropeanOption::rho() const {
    return type_ == OptionType::Call ? call_rho(parameters_)
                                     : put_rho(parameters_);
}

Greeks EuropeanOption::greeks() const {
    return {delta(), gamma(), vega(), theta(), rho()};
}

double EuropeanOption::payoff(double spot_at_expiry) const {
    if (type_ == OptionType::Call) {
        return CallPayoff(parameters_.strike)(spot_at_expiry);
    }
    return PutPayoff(parameters_.strike)(spot_at_expiry);
}

} // namespace quant
