#pragma once
#include<string>
#include "Date.h"

using namespace std;

class Market;

class Trade {
public:
    Trade(){};
    Trade(const string& _type,Date _tradeDate): tradeType(_type), tradeDate(_tradeDate) {};
    Trade (const Trade& other) { 
        tradeType = other.tradeType;
        tradeDate = other.tradeDate;
    };

    inline string getType(){ return tradeType; };
    virtual double Pv(const Market& mkt) const = 0;
    virtual double Payoff(double s) const = 0;
    
    virtual ~Trade()
    {
        cout<< "trade base class destructor is called" <<endl;
    };

protected:   
    string tradeType;
    Date tradeDate;
};