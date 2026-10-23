#include "black_scholes.hpp"
#include "greeks.hpp"

#include <iomanip>
#include <iostream>

int main() {
    const quant::OptionParameters option{
        100.0,  // Spot price
        100.0,  // Strike price
        1.0,    // One year to maturity
        0.05,   // 5% risk-free rate
        0.20    // 20% annual volatility
    };

    std::cout << std::fixed << std::setprecision(4);

    std::cout << "Black-Scholes European Option Pricer\n";
    std::cout << "Call price: " << quant::call_price(option) << '\n';
    std::cout << "Put price:  " << quant::put_price(option) << '\n';

    std::cout << "\nGreeks\n";
    std::cout << "Call delta: " << quant::call_delta(option) << '\n';
    std::cout << "Put delta:  " << quant::put_delta(option) << '\n';
    std::cout << "Gamma:      " << quant::gamma(option) << '\n';
    std::cout << "Vega:       " << quant::vega(option) << '\n';
    std::cout << "Call theta: " << quant::call_theta(option) << '\n';
    std::cout << "Put theta:  " << quant::put_theta(option) << '\n';
    std::cout << "Call rho:   " << quant::call_rho(option) << '\n';
    std::cout << "Put rho:    " << quant::put_rho(option) << '\n';

    return 0;
}