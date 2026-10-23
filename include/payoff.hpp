#pragma once

#include <memory>

// Payoffs of European options at expiry, as a function of the spot price.
//
// Strikes must be positive and finite, and spot prices passed to a payoff
// must be finite and non-negative; otherwise std::invalid_argument is thrown.

namespace quant {

enum class OptionType { Call, Put };

class Payoff {
public:
    virtual ~Payoff() = default;

    virtual double operator()(double spot) const = 0;
    virtual std::unique_ptr<Payoff> clone() const = 0;
};

class StrikePayoff : public Payoff {
public:
    explicit StrikePayoff(double strike);

    double strike() const { return strike_; }

protected:
    double strike_;
};

// max(S - K, 0)
class CallPayoff : public StrikePayoff {
public:
    using StrikePayoff::StrikePayoff;

    double operator()(double spot) const override;
    std::unique_ptr<Payoff> clone() const override;
};

// max(K - S, 0)
class PutPayoff : public StrikePayoff {
public:
    using StrikePayoff::StrikePayoff;

    double operator()(double spot) const override;
    std::unique_ptr<Payoff> clone() const override;
};

// Cash-or-nothing: pays 1 if S > K, otherwise 0.
class DigitalCallPayoff : public StrikePayoff {
public:
    using StrikePayoff::StrikePayoff;

    double operator()(double spot) const override;
    std::unique_ptr<Payoff> clone() const override;
};

// Cash-or-nothing: pays 1 if S < K, otherwise 0.
class DigitalPutPayoff : public StrikePayoff {
public:
    using StrikePayoff::StrikePayoff;

    double operator()(double spot) const override;
    std::unique_ptr<Payoff> clone() const override;
};

// Call or put payoff for the given option type.
std::unique_ptr<Payoff> make_vanilla_payoff(OptionType type, double strike);

} // namespace quant
