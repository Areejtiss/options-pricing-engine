#include "black_scholes.hpp"
#include "monte_carlo.hpp"
#include "option.hpp"

#include <initializer_list>
#include <iomanip>
#include <iostream>

namespace {

void print_option(const char* name, const quant::EuropeanOption& option) {
    const quant::Greeks greeks = option.greeks();

    std::cout << name << '\n';
    std::cout << "  Price: " << option.price() << '\n';
    std::cout << "  Delta: " << greeks.delta << '\n';
    std::cout << "  Gamma: " << greeks.gamma << '\n';
    std::cout << "  Vega:  " << greeks.vega << '\n';
    std::cout << "  Theta: " << greeks.theta << '\n';
    std::cout << "  Rho:   " << greeks.rho << '\n';
}

} // namespace

int main() {
    const quant::OptionParameters parameters{
        100.0,  // Spot price
        100.0,  // Strike price
        1.0,    // One year to maturity
        0.05,   // 5% risk-free rate
        0.20    // 20% annual volatility
    };

    const quant::EuropeanOption call(quant::OptionType::Call, parameters);
    const quant::EuropeanOption put(quant::OptionType::Put, parameters);

    std::cout << std::fixed << std::setprecision(4);

    std::cout << "Black-Scholes European Option Pricer\n\n";
    print_option("Call", call);
    print_option("Put", put);

    std::cout << "\nMonte Carlo (100,000 paths, antithetic)\n";
    for (const auto* option : {&call, &put}) {
        const quant::MonteCarloResult result =
            quant::monte_carlo_price(*option);
        std::cout << (option->type() == quant::OptionType::Call
                          ? "  Call: "
                          : "  Put:  ")
                  << result.price << " +/- " << result.standard_error
                  << "  (Black-Scholes " << option->price() << ")\n";
    }

    return 0;
}
