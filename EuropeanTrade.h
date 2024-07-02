#ifndef _EUROPEAN_TRADE
#define _EUROPEAN_TRADE

#include <cassert> 

#include "TreeProduct.h"
#include "Payoff.h"
#include "Types.h"

class EuropeanOption : public TreeProduct {
public:
    EuropeanOption(){};
    EuropeanOption(OptionType _optType, double _strike, const Date& _expiry, const std::string& _underlying) :optType(_optType), strike(_strike), expiryDate(_expiry), underlying(_underlying) {};
    inline double Payoffswap(double market_rate, const Market& mkt) const {return 0.0;};
    virtual double Payoff(double S) const { return PAYOFF::VanillaOption(optType, strike, S); }
    virtual const Date& GetExpiry() const { return expiryDate; }
    virtual const string& getUnderlying() const { return underlying; }
    virtual double ValueAtNode(double S, double t, double continuation) const { return continuation; }
    string getUnderlying() {return underlying;};
    OptionType getoptiontype() {return optType;};
    double getStike() {return strike;};

protected:
    OptionType optType;
    double strike;
    Date expiryDate;
    std::string underlying;
};

class EuroCallSpread : public EuropeanOption {
public:
    EuroCallSpread(double _k1, double _k2, const Date& _expiry): strike1(_k1), strike2(_k2) { 
        expiryDate = _expiry;
        assert(_k1 < _k2); 
      };
    virtual double Payoff(double S) const { return PAYOFF::CallSpread(strike1, strike2, S); };
    virtual const Date& GetExpiry() const { return expiryDate; };

private:
    double strike1;
    double strike2;
};

#endif
