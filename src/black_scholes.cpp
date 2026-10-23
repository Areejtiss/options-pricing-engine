#include "black_scholes.hpp"

#include "bs_common.hpp"

namespace quant {

double call_price(const OptionParameters& option) {
    detail::validate(option);

    return option.spot * detail::normal_cdf(detail::d1(option)) -
           option.strike * detail::discount_factor(option) *
               detail::normal_cdf(detail::d2(option));
}

double put_price(const OptionParameters& option) {
    detail::validate(option);

    return option.strike * detail::discount_factor(option) *
               detail::normal_cdf(-detail::d2(option)) -
           option.spot * detail::normal_cdf(-detail::d1(option));
}

} // namespace quant
