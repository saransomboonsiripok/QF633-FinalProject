#include <cmath>
#include "Pricer.h"
#include "Bond.h"
#include "Swap.h"
#include "Market.h"
#include <ctime>
#include <iomanip>

double Pricer::Price(const Market& mkt, std::shared_ptr<Trade> trade) 
{
  double pv;
  if (trade->getType() == "TreeProduct") {
    auto treePtr = dynamic_cast<TreeProduct*>(trade.get());
    if (treePtr) { //check if cast is sucessful
      pv = PriceTree(mkt, *treePtr);
    }
  }
  else{
    pv = trade->Pv(mkt);
  }
  return pv;
}

double BinomialTreePricer::PriceTree(const Market& mkt, const TreeProduct& trade)
{
  // model setup
  double T = trade.GetExpiry() - mkt.asOf;
  double dt = T / nTimeSteps;
  double stockPrice, vol, rate;
  string ticker = trade.getUnderlying();
  stockPrice = mkt.getStockPrice()[ticker];
  int year_diff = trade.GetExpiry().year - mkt.asOf.year;
  int month_diff = trade.GetExpiry().month - mkt.asOf.month;
  int day_diff = trade.GetExpiry().day - mkt.asOf.day;
  double period = (12 * year_diff) + (month_diff) + (day_diff / 30);
  vol = mkt.getVolCurve("volatility").getVol(Date(0, period, 0));
  rate = mkt.getCurve("usd-sofr").getRate(Date(0,period,0));

  ModelSetup(stockPrice, vol, rate, dt);
  
  // initialize
  for (int i = 0; i <= nTimeSteps; i++) {
    states[i] = trade.Payoff( GetSpot(nTimeSteps, i) );
  }    
  
  // price by backward induction
  for (int k = nTimeSteps-1; k >= 0; k--)
    for (int i = 0; i <= k; i++) {
    // calculate continuation value
      double df = exp(-rate *dt);	  
      double continuation = df * (states[i]*GetProbUp() + states[i+1]*GetProbDown());
      // calculate the option value at node(k, i)
      states[i] = trade.ValueAtNode( GetSpot(k, i), dt*k, continuation);
    }

  return states[0];

}

void CRRBinomialTreePricer::ModelSetup(double S0, double sigma, double rate, double dt)
{
  double b = std::exp((2*rate+sigma*sigma)*dt)+1;
  u = (b + std::sqrt(b*b - 4*std::exp(2*rate*dt))) / 2 / std::exp(rate*dt);
  p = (std::exp(rate*dt) -  1/u) / (u - 1/u);
  currentSpot = S0;
}

void JRRNBinomialTreePricer::ModelSetup(double S0, double sigma, double rate, double dt)
{
    u = std::exp( (rate - sigma*sigma/2) * dt + sigma * std::sqrt(dt) );
    d = std::exp( (rate - sigma*sigma/2) * dt - sigma * std::sqrt(dt) );
    p = (std::exp(rate*dt) -  d) / (u - d);
    currentSpot = S0;
}
