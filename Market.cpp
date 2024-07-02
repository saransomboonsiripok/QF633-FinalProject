#include "Market.h"
#include <cmath>

using namespace std;

void RateCurve::display() const {
    cout << "rate curve:" << name << endl;
    for (size_t i=0; i<tenors.size(); i++) {
      cout << tenors[i] << ":" << rates[i] << endl;
  }
  cout << endl;
}

void RateCurve::addRate(Date tenor, double rate) {
  //consider to check if tenor already exist
  if (true){
    tenors.push_back(tenor);
    rates.push_back(rate);
  }  
}

double RateCurve::getRate(Date tenor) const {
    // If the tenor is exactly one of the known tenors, return the corresponding rate
    for (size_t i = 0; i < tenors.size(); i++) {
        if (tenors[i] == tenor) {
            return rates[i];
        }
    }

    //If the tenor is outside the known range, return the closest known value
    if (tenor < tenors.front()) {
        return rates.front();
    }
    if (tenor > tenors.back()) {
        return rates.back();
    }

    // Linear interpolation
    for (size_t i = 1; i < tenors.size(); i++) {
        if (tenor < tenors[i]) {
            double t1 = tenors[i - 1] - tenors[0];
            double t2 = tenors[i] - tenors[0];
            double t = tenor - tenors[0];
            double r1 = rates[i - 1];
            double r2 = rates[i];
            return r1 + (r2 - r1) * (t - t1) / (t2 - t1);
        }
    }

    // Fallback (should never reach here)
    return 0.0;
}

double RateCurve::getDf(Date _date) const
{
  double ccr = getRate(_date);
  //double t = _date - _asOf;
  return exp(-ccr * (_date.month / 12));
}

void VolCurve::display() const {
    cout << "vol curve: " << name << endl;
    for (size_t i = 0; i < tenors.size(); i++) {
        cout << tenors[i] << ": " << vols[i] << endl;
    }
    cout << endl;
}

void Market::Print() const {
    cout << "Market as of: " << asOf << endl;
    
    cout << "Stock Prices:" << endl;
    for (const auto& stock : stockPrices) {
        cout << stock.first << ": " << stock.second << endl;
    }
    cout << endl;
    
    cout << "Curves:" << endl;
    for (const auto& curve : curves) {
        curve.second.display();
    }
    cout << endl;
    
    cout << "Volatilities:" << endl;
    for (const auto& vol : vols) {
        vol.second.display();
    }
    cout << endl;
}

void VolCurve::addVol(Date tenor, double vol) {
    tenors.push_back(tenor);
    vols.push_back(vol);
}

double VolCurve::getVol(Date tenor) const {
    // If the tenor is exactly one of the known tenors, return the corresponding vol
    for (size_t i = 0; i < tenors.size(); i++) {
        if (tenors[i] == tenor) {
            return vols[i];
        }
    }

    //If the tenor is outside the known range, return the closest known value
    if (tenor < tenors.front()) {
        return vols.front();
    }
    if (tenor > tenors.back()) {
        return vols.back();
    }

    // Linear interpolation
    for (size_t i = 1; i < tenors.size(); i++) {
        if (tenor < tenors[i]) {
            double t1 = tenors[i - 1] - tenors[0];
            double t2 = tenors[i] - tenors[0];
            double t = tenor - tenors[0];
            double v1 = vols[i - 1];
            double v2 = vols[i];
            return v1 + (v2 - v1) * (t - t1) / (t2 - t1);
        }
    }

    // Fallback (should never reach here)
    return 0.0;
}

void Market::addCurve(const std::string& curveName, const RateCurve& curve){
  curves.emplace(curveName, curve);
}

void Market::addVolCurve(const std::string& curveName, const VolCurve& curve) {
    vols.emplace(curveName, curve);
}

void Market::addBondPrice(const std::string& bondName, double price) {
    bondPrices[bondName] = price;
}

void Market::addStockPrice(const std::string& stockName, double price) {
    stockPrices[stockName] = price;
}

std::ostream& operator<<(std::ostream& os, const Market& mkt)
{
  os << mkt.asOf << std::endl;
  return os;
}

std::istream& operator>>(std::istream& is, Market& mkt)
{
  is >> mkt.asOf;
  return is;
}

