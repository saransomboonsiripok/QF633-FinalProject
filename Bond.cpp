#include "Bond.h"
#include "Market.h"

void Bond::generateSwapSchedule() {
    //std::cout << "Generating bond schedule..." << std::endl;
    //std::cout << "Start Date: " << startDate << ", Maturity Date: " << maturityDate << ", Frequency: " << frequency << std::endl;
    if (startDate == maturityDate || frequency <= 0 || frequency > 1) {
        throw std::runtime_error("Error: start date is later than end date, or invalid frequency!");
    }

    std::string tenorStr;
    if (frequency == 0.25) {
        tenorStr = "3M";
    } else if (frequency == 0.5) {
        tenorStr = "6M";
    } else {
        tenorStr = "1Y";
    }

    Date seed = startDate;
    //cout << tenorStr << endl;
    while (seed < maturityDate) {
        //std::cout << "Adding date: " << seed << std::endl;
        bondSchedule.push_back(seed);
        seed = dateAddTenor(seed, tenorStr);
    }
    bondSchedule.push_back(maturityDate);
    if (bondSchedule.size() < 2) {
        throw std::runtime_error("Error: invalid schedule, check input!");
    }
    //std::cout << "Bond schedule generated with size: " << bondSchedule.size() << std::endl;
}

void Bond::printSchedule() const {
    std::cout << "Bond Schedule:" << std::endl;
    for (const auto& date : bondSchedule) {
        std::cout << date << std::endl;
    }
}

double Bond::Payoff(double s) const
{
  double pv = notional * (s - tradePrice);
  return pv;
}

double Bond::Pv(const Market& mkt) const {
    //std::cout << "Calculating PV..." << std::endl;
    const RateCurve& rateCurve = mkt.getCurve("usd-sofr"); // Assuming "usd_sofr" is the curve name
    double pv = 0.0;

    //std::cout << "Bond Schedule Size: " << bondSchedule.size() << std::endl;
    
    for (size_t i = 0; i < bondSchedule.size() - 1; ++i) {
        Date paymentDate = bondSchedule[i];
        //std::cout << "Payment Date: " << paymentDate << std::endl;
        double tenor_raw = ((paymentDate.day - mkt.asOf.day) / 30) + (paymentDate.month - mkt.asOf.month) + ((paymentDate.year - mkt.asOf.year) * 12);
        Date tenor = Date(0, tenor_raw, 0);
        //cout << tenor << endl;
        double discountFactor = rateCurve.getDf(tenor);
        //std::cout << "Discount Factor: " << discountFactor << std::endl;
        double couponPayment = notional * coupon * frequency; // Coupon payment
        //std::cout << "Coupon Payment: " << couponPayment << std::endl;
        pv += couponPayment * discountFactor;
        //std::cout << "Intermediate PV: " << pv << std::endl;
    }
    
    // Add the principal repayment at maturity
    Date maturityDate = bondSchedule.back();
    double discountFactor = rateCurve.getDf(maturityDate);
    pv += notional * discountFactor;
    //std::cout << "Maturity Discount Factor: " << discountFactor << std::endl;
    //std::cout << "Final PV: " << pv << std::endl;

    return pv;
}



