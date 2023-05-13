#include "App.h"

OptionScanner::OptionScanner(const char* host) : host(host), YW(false) {

    cout << "Scanner Initialized" << endl;

    // Initialize connection
    EC = EClientL0::New(&YW);
    // Connect to TWS
    EC->eConnect(host, 7496, 0);

    // NOTE : Some API functions will encounter issues if called immediately after connection
    //          Here we will start with a simple request for the TWS current time and wait
    EC->reqCurrentTime();
    Sleep(10);

    getDateTime();

	// Initialize with contract specs
	SPXunderlying.symbol = "SPX";
	SPXunderlying.secType = *SecType::IND;
	SPXunderlying.currency = "USD";
	SPXunderlying.exchange = *Exchange::IB_SMART;
	SPXunderlying.primaryExchange = *Exchange::CBOE;

    SPXchain.symbol = "SPX";
    SPXchain.secType = *SecType::OPT;             
    SPXchain.currency = "USD";
    SPXchain.exchange = *Exchange::IB_SMART;       
    SPXchain.primaryExchange = *Exchange::CBOE;          
    SPXchain.right = *ContractRight::CALL;
    SPXchain.expiry = EndDateTime(todayDate[2], todayDate[1], todayDate[0]);
    SPXchain.strike = strike;
}

OptionScanner::~OptionScanner() {
    EC->eDisconnect();
    delete EC;

}

void OptionScanner::getDateTime() {
    std::time_t tmNow;
    tmNow = time(NULL);
    struct tm t = *localtime(&tmNow);

    if (todayDate.empty()) {
        todayDate.push_back(t.tm_mday + 1); // To get current data, you have to request one day ahead for IBKR
        todayDate.push_back(t.tm_mon + 1);
        todayDate.push_back(t.tm_year + 1900);
    }
    else {
        todayDate[0] = t.tm_mday + 1;
        todayDate[1] = t.tm_mon + 1;
        todayDate[2] = t.tm_year + 1900;
    }
}

// This will cause the wrapper to return a vector full of today's SPX price range in minute intervals
void OptionScanner::retreiveSPXPrice() {
    // Use reqId 101 for SPX underlying price
    EC->reqHistoricalData
    (101
        , SPXunderlying
        , EndDateTime(todayDate[2], todayDate[1], todayDate[0])
        , DurationStr(1, *DurationHorizon::Days)
        , *BarSizeSetting::_1_min
        , *WhatToShow::TRADES
        , UseRTH::OnlyRegularTradingData
        , FormatDate::AsDate
    );
}