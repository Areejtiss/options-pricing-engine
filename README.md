# Options Pricing Engine

A small C++17 library for pricing European options. It provides
closed-form Black-Scholes prices and Greeks, plus a Monte Carlo pricer
that can value any payoff and is checked against the exact prices.

## Features

- **Black-Scholes prices** for European calls and puts
- **Greeks**: Delta, Gamma, Vega, Theta and Rho
- **Payoffs**: call, put, digital (cash-or-nothing) call and digital put,
  behind a common `Payoff` interface
- **`EuropeanOption`**: one object for an option's price, Greeks and payoff
- **Monte Carlo pricer** with a seeded generator, antithetic variates,
  a standard error and a 95% confidence interval
- **Input validation**: invalid or non-finite inputs throw
  `std::invalid_argument`
- **Tests** run with CTest on every push and pull request (GitHub Actions)

## Quick start

### Linux / macOS

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/options_pricer
```

### Windows (Visual Studio)

```sh
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
build\Release\options_pricer.exe
```

### Docker

The image builds the project, runs the tests (the build fails if any
test fails), and then runs the demo:

```sh
docker build -t options-pricing-engine .
docker run --rm options-pricing-engine
```

Requirements: CMake 3.20+ and a C++17 compiler (GCC, Clang or MSVC).

## Example

```cpp
#include "monte_carlo.hpp"
#include "option.hpp"

#include <iostream>

int main() {
    // Spot, strike, maturity (years), risk-free rate, volatility
    const quant::OptionParameters parameters{100.0, 100.0, 1.0, 0.05, 0.20};

    const quant::EuropeanOption call(quant::OptionType::Call, parameters);

    std::cout << "Price: " << call.price() << '\n';  // 10.4506
    std::cout << "Delta: " << call.delta() << '\n';  // 0.636831

    const quant::MonteCarloResult mc =
        quant::monte_carlo_price(call, {200000, 42, true});
    std::cout << "Monte Carlo: " << mc.price
              << " +/- " << mc.standard_error << '\n';
}
```

Any payoff can be priced by Monte Carlo, including payoffs that have no
`EuropeanOption` wrapper:

```cpp
const quant::DigitalCallPayoff digital(100.0);
const auto result = quant::monte_carlo_price(digital, parameters);
```

The free functions (`quant::call_price`, `quant::gamma`,
`quant::put_rho`, ...) are also available directly.

## Demo output

`options_pricer` prices an at-the-money option with S = K = 100,
T = 1 year, r = 5% and σ = 20%:

```
Call
  Price: 10.4506
  Delta: 0.6368
  Gamma: 0.0188
  Vega:  37.5240
  Theta: -6.4140
  Rho:   53.2325
Put
  Price: 5.5735
  Delta: -0.3632
  Gamma: 0.0188
  Vega:  37.5240
  Theta: -1.6579
  Rho:   -41.8905

Monte Carlo (100,000 paths, antithetic)
  Call: 10.4520 +/- 0.0328  (Black-Scholes 10.4506)
  Put:  5.5770 +/- 0.0209  (Black-Scholes 5.5735)
```

The Monte Carlo digits can differ slightly between compilers, because
standard libraries generate different normal random numbers from the
same seed.

## Model

Under Black-Scholes, with spot S, strike K, maturity T, risk-free
rate r and volatility σ:

```
d1 = (ln(S/K) + (r + σ²/2)·T) / (σ·√T)
d2 = d1 − σ·√T

Call = S·N(d1) − K·e^(−rT)·N(d2)
Put  = K·e^(−rT)·N(−d2) − S·N(−d1)
```

N is the standard normal CDF and n is its density.

| Greek | Call | Put | Units |
|---|---|---|---|
| Delta | N(d1) | N(d1) − 1 | per 1.00 change in spot |
| Gamma | n(d1) / (S·σ·√T) | same as call | per 1.00 change in spot |
| Vega | S·n(d1)·√T | same as call | per 1.00 change in σ (÷100 for per 1%) |
| Theta | −S·n(d1)·σ/(2√T) − r·K·e^(−rT)·N(d2) | −S·n(d1)·σ/(2√T) + r·K·e^(−rT)·N(−d2) | per year (÷365 for per day) |
| Rho | K·T·e^(−rT)·N(d2) | −K·T·e^(−rT)·N(−d2) | per 1.00 change in r (÷100 for per 1%) |

**Monte Carlo.** The spot at expiry is sampled exactly from geometric
Brownian motion,
`S_T = S·exp((r − σ²/2)·T + σ·√T·Z)` with Z ~ N(0, 1), so there is no
time-stepping error for European payoffs. The price is the discounted
average payoff, `e^(−rT)·mean(payoff(S_T))`. With antithetic variates
each draw Z is paired with −Z, which lowers the standard error at
almost no extra cost.

## Project layout

```
include/
  black_scholes.hpp   OptionParameters, call_price, put_price
  greeks.hpp          Delta, Gamma, Vega, Theta, Rho
  payoff.hpp          OptionType and the Payoff hierarchy
  option.hpp          EuropeanOption and the Greeks struct
  monte_carlo.hpp     Monte Carlo settings, result and pricer
src/
  bs_common.hpp       Internal helpers: normal CDF/PDF, d1, d2, validation
  *.cpp               Implementations, built as the `quant` static library
  main.cpp            Demo executable (options_pricer)
tests/
  test_helpers.hpp    Minimal assertion helpers
  test_*.cpp          One test executable per module, run by CTest
```

## Testing

There are five test suites, one per module:

- **BlackScholesTests**: reference prices, put-call parity, price
  bounds and rejection of invalid or non-finite inputs.
- **GreeksTests**: reference values, call/put relationships, and each
  Greek against a finite-difference derivative of the price for
  in-, at- and out-of-the-money options.
- **PayoffTests**: payoff values, edge cases at the strike, cloning
  and the factory.
- **OptionTests**: `EuropeanOption` matches the free functions exactly.
- **MonteCarloTests**: convergence to the exact call, put and digital
  prices within 4 standard errors, parity, variance reduction,
  1/√N error decay and reproducibility.

## Roadmap

- Implied volatility solver (Newton-Raphson with a bisection fallback)
- American options with a binomial tree
- Path-dependent payoffs (Asian, barrier) with multi-step Monte Carlo
- Continuous dividend yield
- Monte Carlo Greeks

## License

MIT. See [LICENSE](LICENSE).
