#include "black_scholes.hpp"

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
    std::cout << "Call delta: " << quant::call_delta(option) << '\n';
    std::cout << "Put delta:  " << quant::put_delta(option) << '\n';

    return 0;
}