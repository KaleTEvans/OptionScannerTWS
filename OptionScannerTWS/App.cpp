#include "App.h"

OptionScanner::OptionScanner(const char* host, IBString ticker) : host(host), ticker(ticker), YW(false) {

    cout << "Scanner Initialized" << endl;

    // Initialize connection
    EC = EClientL0::New(&YW);
    // Connect to TWS
    EC->eConnect(host, 7496, 0);

    // NOTE : Some API functions will encounter issues if called immediately after connection
    //          Here we will start with a simple request for the TWS current time and wait
    // ** Also note that this will return time in Unix form for the current time zone of the server hosting tws
    EC->reqCurrentTime();
    Sleep(10);

    getDateTime();

	// Initialize with contract specs
	underlying.symbol = this->ticker;
	
    // Check if the symbol is an index or a security
    if (ticker == "SPX") {
        underlying.secType = *SecType::IND;
        underlying.primaryExchange = *Exchange::CBOE;
    }
    else {
        underlying.secType = *SecType::STK;
    }

	underlying.currency = "USD";
    underlying.exchange = *Exchange::IB_SMART;

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
void OptionScanner::retreiveUnderlyingPrice(string interval, string duration, TickerId reqId) {
    // Use reqId 101 for SPX underlying price
    EC->reqHistoricalData
    (reqId
        , underlying
        , EndDateTime(todayDate[2], todayDate[1], todayDate[0])
        , duration
        , interval
        , *WhatToShow::TRADES
        , UseRTH::OnlyRegularTradingData
        , FormatDate::AsDate
        , false
    );

    // Wait for the response
    while (YW.notDone()) {
        EC->checkMessages();
        if (YW.Req == reqId) break;
    }

    // Clear out the prices vector and repopulate
    if (!prices.empty()) prices.clear();

    for (auto i : YW.candlesticks) prices.push_back(i);

    // Once completed, clear the wrapper vector for next callback
    YW.candlesticks.clear();
}

// The multiple variable is the increments of options strikes
void OptionScanner::populateStrikes(int multiple) {
    // Retrieve the latest SPX price
    retreiveUnderlyingPrice("1 min", "1 D", 101);

    // Clear the strikes vector
    if (!strikes.empty()) strikes.clear();

    // Round the price down to nearest increment
    double currentPrice = prices[prices.size() - 1].close;
    int roundedPrice = currentPrice + (multiple / 2);
    roundedPrice -= roundedPrice % multiple;
    int strikePrice = roundedPrice - (multiple * 5);

    // This will give us 11 strikes in total
    while (strikePrice <= roundedPrice + (multiple * 5)) {
        strikes.push_back(strikePrice);
        strikePrice += multiple;
    }

    for (auto i : strikes) cout << i << " ";
    cout << endl;

}

void OptionScanner::retrieveOptionData() {
    // To get this data, we will need to loop over each option strike and send a request
    // The reqId will be the same as the contract strike value

    // Use a set to store all the reqIds to know when the wrapper has received every request
    unordered_set<int> requests;
    for (auto i : strikes) {
        requests.insert(i);
        requests.insert(i + 1);
    }

    for (auto i : requests) cout << i << " ";
    cout << endl;

    for (auto i : strikes) {

        // Ensure wrapper candlestick vector is empty
        YW.fiveSecCandles.clear();

       // vector<double> closePrice;

        // Create the contract
        SPXchain.symbol = "SPX";
        SPXchain.secType = *SecType::OPT;
        SPXchain.currency = "USD";
        SPXchain.exchange = *Exchange::IB_SMART;
        SPXchain.primaryExchange = *Exchange::CBOE;
        SPXchain.right = *ContractRight::CALL;
        SPXchain.expiry = EndDateTime(todayDate[2], todayDate[1], todayDate[0]);
        SPXchain.strike = i;

        // Create the request
        EC->reqRealTimeBars
        (i
            , SPXchain
            , 5
            , *WhatToShow::TRADES
            , UseRTH::OnlyRegularTradingData
        );

        //for (auto i : YW.candlesticks) closePrice.push_back(i.close);
        //cout << closePrice[closePrice.size() - 1] << endl;

        // cout << "Receieved options data for strike: " << i << endl;
    }

    //for (auto i : strikes) {
    //    // Create the contract
    //    SPXchain.symbol = "SPX";
    //    SPXchain.secType = *SecType::OPT;
    //    SPXchain.currency = "USD";
    //    SPXchain.exchange = *Exchange::IB_SMART;
    //    SPXchain.primaryExchange = *Exchange::CBOE;
    //    SPXchain.right = *ContractRight::PUT;
    //    SPXchain.expiry = EndDateTime(todayDate[2], todayDate[1], todayDate[0]);
    //    SPXchain.strike = i;

    //    // Create the request
    //    EC->reqHistoricalData
    //    (i+1
    //        , SPXchain
    //        , EndDateTime(todayDate[2], todayDate[1], todayDate[0])
    //        , DurationStr(1, *DurationHorizon::Days)
    //        , *BarSizeSetting::_1_min
    //        , *WhatToShow::TRADES
    //        , UseRTH::OnlyRegularTradingData
    //        , FormatDate::AsDate
    //        , false
    //    );
    //}

    // Wait for requst
    while (YW.notDone()) {
        if (requests.empty()) break;
        EC->checkMessages();
        
        // Remove reqId from set once received
        if (requests.find(YW.Req) != requests.end()) requests.erase(YW.Req);
    }

    YW.m_Done = true;
}