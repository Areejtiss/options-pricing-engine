#include "monte_carlo.hpp"

#include "bs_common.hpp"

#include <cmath>
#include <random>
#include <stdexcept>

namespace quant {
namespace {

void validate_settings(const MonteCarloSettings& settings) {
    if (settings.antithetic && settings.num_paths % 2 != 0) {
        throw std::invalid_argument(
            "Antithetic sampling requires an even number of paths."
        );
    }

    // At least two independent samples are needed for a standard error.
    const std::size_t samples =
        settings.antithetic ? settings.num_paths / 2 : settings.num_paths;
    if (samples < 2) {
        throw std::invalid_argument(
            "Too few paths: need at least two independent samples."
        );
    }
}

// Running mean and variance using Welford's algorithm.
class RunningStats {
public:
    void add(double value) {
        ++count_;
        const double delta = value - mean_;
        mean_ += delta / static_cast<double>(count_);
        sum_squared_deviations_ += delta * (value - mean_);
    }

    double mean() const { return mean_; }

    double standard_error() const {
        const double n = static_cast<double>(count_);
        const double sample_variance = sum_squared_deviations_ / (n - 1.0);
        return std::sqrt(sample_variance / n);
    }

private:
    std::size_t count_ = 0;
    double mean_ = 0.0;
    double sum_squared_deviations_ = 0.0;
};

} // namespace

MonteCarloResult monte_carlo_price(
    const Payoff& payoff,
    const OptionParameters& parameters,
    const MonteCarloSettings& settings
) {
    detail::validate(parameters);
    validate_settings(settings);

    const double drift =
        (parameters.risk_free_rate -
         0.5 * parameters.volatility * parameters.volatility) *
        parameters.maturity;
    const double diffusion =
        parameters.volatility * std::sqrt(parameters.maturity);

    const auto terminal_payoff = [&](double z) {
        return payoff(parameters.spot * std::exp(drift + diffusion * z));
    };

    std::mt19937_64 generator(settings.seed);
    std::normal_distribution<double> standard_normal(0.0, 1.0);

    // With antithetic variates each sample is the average of a pair,
    // so the standard error is computed over independent pairs.
    RunningStats stats;
    if (settings.antithetic) {
        for (std::size_t i = 0; i < settings.num_paths / 2; ++i) {
            const double z = standard_normal(generator);
            stats.add(0.5 * (terminal_payoff(z) + terminal_payoff(-z)));
        }
    } else {
        for (std::size_t i = 0; i < settings.num_paths; ++i) {
            stats.add(terminal_payoff(standard_normal(generator)));
        }
    }

    const double discount = detail::discount_factor(parameters);

    return {
        discount * stats.mean(),
        discount * stats.standard_error(),
        settings.num_paths
    };
}

MonteCarloResult monte_carlo_price(
    const EuropeanOption& option,
    const MonteCarloSettings& settings
) {
    const auto payoff =
        make_vanilla_payoff(option.type(), option.parameters().strike);
    return monte_carlo_price(*payoff, option.parameters(), settings);
}

} // namespace quant
