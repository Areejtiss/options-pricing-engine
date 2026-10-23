#include "greeks.hpp"

#include "bs_common.hpp"

#include <cmath>

namespace quant {

double call_delta(const OptionParameters& option) {
    detail::validate(option);
    return detail::normal_cdf(detail::d1(option));
}

double put_delta(const OptionParameters& option) {
    detail::validate(option);
    return detail::normal_cdf(detail::d1(option)) - 1.0;
}

double gamma(const OptionParameters& option) {
    detail::validate(option);
    return detail::normal_pdf(detail::d1(option)) /
           (option.spot * option.volatility *
            std::sqrt(option.maturity));
}

double vega(const OptionParameters& option) {
    detail::validate(option);
    return option.spot * detail::normal_pdf(detail::d1(option)) *
           std::sqrt(option.maturity);
}

double call_theta(const OptionParameters& option) {
    detail::validate(option);

    const double time_decay =
        -option.spot * detail::normal_pdf(detail::d1(option)) *
        option.volatility / (2.0 * std::sqrt(option.maturity));

    return time_decay -
           option.risk_free_rate * option.strike *
               detail::discount_factor(option) *
               detail::normal_cdf(detail::d2(option));
}

double put_theta(const OptionParameters& option) {
    detail::validate(option);

    const double time_decay =
        -option.spot * detail::normal_pdf(detail::d1(option)) *
        option.volatility / (2.0 * std::sqrt(option.maturity));

    return time_decay +
           option.risk_free_rate * option.strike *
               detail::discount_factor(option) *
               detail::normal_cdf(-detail::d2(option));
}

double call_rho(const OptionParameters& option) {
    detail::validate(option);
    return option.strike * option.maturity *
           detail::discount_factor(option) *
           detail::normal_cdf(detail::d2(option));
}

double put_rho(const OptionParameters& option) {
    detail::validate(option);
    return -option.strike * option.maturity *
           detail::discount_factor(option) *
           detail::normal_cdf(-detail::d2(option));
}

} // namespace quant
