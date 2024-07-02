#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>
#include <chrono>
#include <string>
#include <algorithm>
#include <iomanip>

#include "Market.h"
#include "Bond.h"
#include "Pricer.h"
#include "RiskEngine.h"
#include "Factory.h"
#include "thread_pool.h"

using namespace std;

Date convertTenorToDate(const string &tenorStr) {
    string cleanedTenorStr = tenorStr.substr(0, tenorStr.size() - 1); // Remove the last character (colon)
    Date tenorDate;
    if (cleanedTenorStr == "ON") {
        tenorDate = Date(0, 0, 0); // Overnight
    } else if (cleanedTenorStr.back() == 'M') {
        int months = stoi(cleanedTenorStr.substr(0, cleanedTenorStr.size() - 1));
        tenorDate = Date(0, months, 0);
    } else if (cleanedTenorStr.back() == 'Y') {
        int months = stoi(cleanedTenorStr.substr(0, cleanedTenorStr.size() - 1)) * 12;
        tenorDate = Date(0, months, 0);
    }
    return tenorDate;
}

void loadStockPrices(Market &mkt, const string &date, const string &prefix) {
    // Load stock prices
    ifstream stockFile(prefix + "stock_price_" + date + ".txt");
    string line;
    while (getline(stockFile, line)) {
        istringstream iss(line);
        string symbol;
        char colon;
        double price;
        if (iss >> symbol >> colon >> price) {
            // Remove the colon at the end of the ticker symbol
            if (!symbol.empty() && symbol.back() == ':') {
                symbol.pop_back();
            }
            mkt.addStockPrice(symbol, price);
        }
    }
    stockFile.close();
}

void loadRateCurve(Market &mkt, const string &date, const string &prefix, const string &curveName) {
    // Load rate curve
    ifstream curveFile(prefix + "usd_sofr_" + date + ".txt");
    string line;
    RateCurve rateCurve(curveName);
    while (getline(curveFile, line)) {
        istringstream iss(line);
        string tenorStr;
        double rate;
        char percent;
        if (iss >> tenorStr >> rate >> percent) {
            Date tenorDate = convertTenorToDate(tenorStr); // Convert tenorStr to Date
            rateCurve.addRate(tenorDate, rate / 100.0); // Convert percentage to decimal
        }
    }
    mkt.addCurve(curveName, rateCurve);
    curveFile.close();
}

void loadVolCurve(Market &mkt, const string &date, const string &prefix, const string &curveName) {
    // Load volatility curve
    ifstream volFile(prefix + "vol_" + date + ".txt");
    string line;
    VolCurve volCurve(curveName);
    while (getline(volFile, line)) {
        istringstream iss(line);
        string tenorStr;
        double vol;
        char percent;
        if (iss >> tenorStr >> vol >> percent) {
            Date tenorDate = convertTenorToDate(tenorStr); // Convert tenorStr to Date
            volCurve.addVol(tenorDate, vol / 100.0); // Convert percentage to decimal
        }
    }
    mkt.addVolCurve(curveName, volCurve);
    volFile.close();
}

int main()
{
  ofstream resultFile("output.txt");
  resultFile << "QF633 - Project 2 - Output File" << "\n" << "\n";
  //task 1, create an market data object, and update the market data from from txt file
  // load 2 days market data
  std::time_t t = std::time(0);
  auto now_ = std::localtime(&t);
  Date valueDate;
  valueDate.year = now_->tm_year + 1900;
  valueDate.month = now_->tm_mon + 1;
  valueDate.day = now_->tm_mday;

Market mkt1(Date(2024, 6, 1));
loadStockPrices(mkt1, "20240601", "");
loadRateCurve(mkt1, "20240601", "", "usd-sofr");
loadVolCurve(mkt1, "20240601", "", "volatility");


// Load data for 20240602
Market mkt2(Date(2024, 6, 2));
loadStockPrices(mkt2, "20240602", "");
loadRateCurve(mkt2, "20240602", "", "usd-sofr");
loadVolCurve(mkt2, "20240602", "", "volatility");

// to print the market data for verification
// cout << "Market data for 20240601:" << endl;
// mkt1.Print();

// cout << "Market data for 20240602:" << endl;
// mkt2.Print();

// To access stock price
// auto stockprices = mkt1.getStockPrice();
// cout << stockprices["APPL"] << endl;

// To access rate curve
// mkt1.getCurve("usd-sofr").display();
// double rates = mkt1.getCurve("usd-sofr").getRate(Date(0,84,0));
// cout << rates << endl;

// To access vol curve
// double vol = mkt1.getVolCurve("volatility").getVol(Date(0,84,0));
// cout << vol << endl;

  //task 2, create a portfolio of bond, swap, european option, american option
  vector<std::shared_ptr<Trade>> myPortfolio;
  
  auto bFactory = std::make_unique<BondFactory>();
  auto sFactory = std::make_unique<SwapFactory>();
  auto eFactory = std::make_unique<EurOptFactory>();
  auto aFactory = std::make_unique<AmericanOptFactory>();
  // add bond
  //std::string underlying, Date start, Date end, double notional, double strike, double freq, OptionType opt
  auto bond1 = bFactory->createTrade("usd-gov",Date(2024, 6, 1), Date(2034, 6, 1),1000000, 0.0250, 0.5, OptionType::None);
  auto bond2 = bFactory->createTrade("usd-gov",Date(2024, 6, 1), Date(2044, 6, 1),-500000, 0.02, 0.5, OptionType::None);

  // add swap
  //std::string underlying, Date start, Date end, double notional, double strike, double freq, OptionType opt
  auto swap = sFactory->createTrade("usd-sofr",Date(2024, 6, 1), Date(2034, 6, 1),-1000000, 0.0375, 1.0, OptionType::None);
  auto swap2 = sFactory->createTrade("usd-sofr",Date(2024, 6, 1), Date(2030, 6, 1),2000000, 0.043, 0.5, OptionType::None);
  auto swap3 = sFactory->createTrade("usd-sofr",Date(2024, 6, 1), Date(2030, 6, 1),5000000, 0.032, 1.0, OptionType::None);
  auto swap4 = sFactory->createTrade("usd-sofr",Date(2024, 6, 1), Date(2032, 6, 1),1000000, 0.04, 0.5, OptionType::None);

  // add european option
  // std::string underlying, Date start, Date end, double notional, double strike, double freq, OptionType opt
  auto eCall1 = eFactory->createTrade("APPL", Date(2026, 6, 1), Date(2026, 6, 1), 200, 655, 0, OptionType::Call);
  auto eCall2 = eFactory->createTrade("SP500", Date(2026, 6, 1), Date(2026, 6, 1), 100, 5020, 0, OptionType::Call);
  auto eCall3 = eFactory->createTrade("STI", Date(2026, 6, 1), Date(2026, 6, 1), 400, 3400, 0, OptionType::Call);

  // add american option
  //std::string underlying, Date start, Date end, double notional, double strike, double freq, OptionType opt
  auto aPut1 = aFactory->createTrade("APPL", Date(2026, 6, 1), Date(2026, 6, 1), 200, 525, 0, OptionType::Put);
  auto aPut2 = aFactory->createTrade("SP500", Date(2027, 6, 1), Date(2027, 6, 1), 100, 525, 0, OptionType::Put);
  auto aPut3 = aFactory->createTrade("STI", Date(2028, 6, 1),Date(2028, 6, 1), 1000, 525, 0, OptionType::Put);

  myPortfolio.push_back(bond1);
  myPortfolio.push_back(bond2);
  myPortfolio.push_back(swap);
  myPortfolio.push_back(swap2);
  myPortfolio.push_back(swap3);
  myPortfolio.push_back(swap4);
  myPortfolio.push_back(eCall1);
  myPortfolio.push_back(eCall2);
  myPortfolio.push_back(eCall3);
  myPortfolio.push_back(aPut1);
  myPortfolio.push_back(aPut2);
  myPortfolio.push_back(aPut3);

  //task 3, creat a pricer and price the portfolio, output the pricing result of each deal 
  //3.1 compute the NPV of deal as of market date 1
  //3.2 compute the NPV of deal as of market date 2, and then compute the daily Pnl for each deal uisng NPV(date2) - NPV (date1), and output the result in file
  double total_PV_1 = 0;
  double total_PV_2 = 0;
  int tradeID_1 = 1;
  resultFile << "NPV and PnL of portfolio" << endl;
  resultFile << "----------------------------------------------------------------------" << "\n";
  auto pricer = new CRRBinomialTreePricer(100);
  
  for (size_t i = 0; i<myPortfolio.size(); i++) {
    auto& trade = myPortfolio[i];
    double pv_1 = pricer->Price(mkt1, trade);
    double pv_2 = pricer->Price(mkt2, trade);
    if (trade->getType() == "BondTrade"){
      Bond* bondptr = dynamic_cast<Bond*>(trade.get());
      resultFile << "Trade number: " << tradeID_1 << " " << "type of asset: " << " " << "Bond" << "\n";
      resultFile << "Issuer: " << bondptr->getUnderlying() << "\n";
      Date start = bondptr->getStarting();
      Date matur = bondptr->getMaturity();
      resultFile << "Start Date of Bond: " << start.day << "/" << start.month << "/" << start.year << "\n";
      resultFile << "Maturity Date of Bond: " << matur.day << "/" << matur.month << "/" << matur.year << "\n";
      resultFile << "Notional: " << std::setprecision(8) << bondptr->getNotional() << "\n";
      resultFile << "Present Value at day 1: " << pv_1 << "\n";
      resultFile << "Present Value at day 2: " << pv_2 << "\n";
      resultFile << "PnL of the asset: " << pv_2 - pv_1 << "\n" << "\n";
      tradeID_1 += 1;
      total_PV_1 += pv_1;
      total_PV_2 += pv_2;
    }
    else if (trade->getType() == "SwapTrade"){
      Swap* swapptr = dynamic_cast<Swap*>(trade.get());
      resultFile << "Trade number: " << tradeID_1 << " " << "type of asset: " << " " << "Swap" << "\n";
      Date start_swap = swapptr->getMaturity();
      Date matur_swap = swapptr->getMaturity();
      resultFile << "Start Date: " << start_swap.day << "/" << start_swap.month << "/" << start_swap.year << "\n";
      resultFile << "Maturity Date: " << matur_swap.day << "/" << matur_swap.month << "/" << matur_swap.year << "\n";
      resultFile << "Notional: " << swapptr->getNotional() << "\n";
      resultFile << "Traded Rate: " << swapptr->getTraderate() << "\n";
      resultFile << "Payment Frequency: " << swapptr->getFrequency() << "\n";
      resultFile << "Present Value at day 1: " << pv_1 << "\n";
      resultFile << "Present Value at day 2: " << pv_2 << "\n";
      resultFile << "PnL of the asset: " << pv_2 - pv_1 << "\n" << "\n";
      tradeID_1 += 1;
      total_PV_1 += pv_1;
      total_PV_2 += pv_2;
    }
    else if (trade->getType() == "TreeProduct"){
      EuropeanOption* eur = dynamic_cast<EuropeanOption*>(trade.get());
      if (eur){
        resultFile << "Trade number: " << tradeID_1 << " " << "type of asset: " << " " << "European Option" << "\n";
        if (eur->getoptiontype() == 0){
          resultFile << "Option Type: Call" << "\n";
        }
        else {
          resultFile << "Option Type: Put" << "\n";
        }
        resultFile << "Expiry Date: " << eur->GetExpiry().day << "/" << eur->GetExpiry().month << "/" << eur->GetExpiry().year << "\n";
        resultFile << "Strike Price: " << eur->getStike() << "\n";
        resultFile << "Underlying: " << eur->getUnderlying() << "\n";
        resultFile << "Present Value at day 1: " << pv_1 << "\n";
        resultFile << "Present Value at day 2: " << pv_2 << "\n";
        resultFile << "PnL of the asset: " << pv_2 - pv_1 << "\n" << "\n";
        tradeID_1 += 1;
        total_PV_1 += pv_1;
        total_PV_2 += pv_2;
      }
        else{
        AmericanOption* amr = dynamic_cast<AmericanOption*>(trade.get());
        resultFile << "Trade number: " << tradeID_1 << " " << "type of asset: " << " " << "American Option" << "\n";
        if (amr->getoptiontype() == 0){
          resultFile << "Option Type: Call" << "\n";
        }
        else {
          resultFile << "Option Type: Put" << "\n";
        }
        resultFile << "Expiry Date: " << amr->GetExpiry().day << "/" << amr->GetExpiry().month << "/" << amr->GetExpiry().year << "\n";
        resultFile << "Strike Price: " << amr->getStike() << "\n";
        resultFile << "Underlying: " << amr->getUnderlying() << "\n";
        resultFile << "Present Value at day 1: " << pv_1 << "\n";
        resultFile << "Present Value at day 2: " << pv_2 << "\n";
        resultFile << "PnL of the asset: " << pv_2 - pv_1 << "\n" << "\n";
        tradeID_1 += 1;
        total_PV_1 += pv_1;
        total_PV_2 += pv_2;
      }  
    }
  }
  resultFile << "NPV of the first day: " << total_PV_1 << "\n";
  resultFile << "NPV of the second day: " << total_PV_2 << "\n";
  resultFile << "PnL on day 2: " << total_PV_2 - total_PV_1 << "\n";

  //task 4, compute the Greeks of DV01, and Vega risk as of market date 1
  // 4.1 compute risk using risk engine
  // 4.2 use idea of multi-threading
  // analyzing the risk of PnL and risk of your portfolio and come out some simple strategy of a) square off the risk, b) maximize the PnL

  // sample code for risk computation
  //double curve_shock = 0.0001;// 1 bp of zero rate
  //double vol_shock = 0.01; //1% of log normal vol
  //double price_shock = 1.0; // shock in abs price of stock
  
  
  // example 1, simple example of computing one point dv01 for one swap
//   string risk_id = "usd-sofr:on";
//   double shockUp = 0.0001;
//   double shockDown = -0.0001;
//   auto testShockUp = MarketShock();
//   testShockUp.market_id ="usd-sofr";
//   testShockUp.shock = make_pair(Date(), shockUp);
//   auto testShockDown = MarketShock();
//   testShockDown.market_id ="usd-sofr";
//   testShockDown.shock = make_pair(Date(), shockDown);

//   auto shockedUpCurveUp = CurveDecorator(mkt1, testShockUp);
//   auto shockedUpCurveDown = CurveDecorator(mkt1, testShockDown);
  
//   unordered_map<string, double> thisDealDv01;
//   double pv_up, pv_down;
//   auto m_up = shockedUpCurveUp.getMarketUp();
//   pv_up = swap->Pv(m_up);
//   auto m_down = shockedUpCurveDown.getMarketDown();
//   pv_down = swap->Pv(m_down);
//   double dv01 = (pv_up - pv_down)/2.0;
//   thisDealDv01.emplace(risk_id, dv01);

  //example2, using risk engine to compute full set of dv01 for a swap
//   RiskEngine re(mkt1, curve_shock, vol_shock, price_shock);
//   re.computeRisk("dv01", swap, true);
//   auto dv01_of_swap = re.getResult();

  //example 3, demo using thread pool
//   ThreadPool pool(4); 
//   map<string, double> swapDv01;
//   auto pv_job = [&swapDv01, risk_id, &swap, &m_up, &m_down]() {
//     cout << "Task is running on thread: " << this_thread::get_id() << endl; 
//     auto pricer = std::make_unique<CRRBinomialTreePricer>(100);
//     double pv_u = pricer->Price(m_up, swap);
//     double pv_d = pricer->Price(m_down, swap);
//     double dv01 = (pv_u - pv_d) / 2.;
//     swapDv01.emplace(std::make_pair(risk_id, dv01));
//     this_thread::sleep_for(chrono::milliseconds(100)); 
//   }; 

//   for (int i = 0; i < 5; ++i) { 
//       pool.enqueue(pv_job); 
//   } 
  
  //final
  cout << "Project build successfully!" << endl;
  return 0;
}
