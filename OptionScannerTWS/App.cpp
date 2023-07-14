#include "App.h"

App::App(const char* host) : host(host), YW(false) {

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

    // Default contract will be SPX
    createUnderlyingContract("SPX", true);
}

App::~App() {
    EC->eDisconnect();
    delete EC;

}

void App::createUnderlyingContract(IBString ticker, bool isIndex) {
    underlying.symbol = ticker;

    // Check if the symbol is an index or a security
    if (isIndex) {
        underlying.secType = *SecType::IND;
        underlying.primaryExchange = *Exchange::CBOE;
    }
    else {
        underlying.secType = *SecType::STK;
        underlying.primaryExchange = *Exchange::IB_SMART;
    }

    underlying.currency = "USD";
    underlying.exchange = *Exchange::IB_SMART;
}

void App::getDateTime() {
    std::time_t tmNow;
    tmNow = time(NULL);
    struct tm t = *localtime(&tmNow);

    if (todayDate.empty()) {
        todayDate.push_back(t.tm_mday);
        todayDate.push_back(t.tm_mon + 1);
        todayDate.push_back(t.tm_year + 1900);
        todayDate.push_back(t.tm_hour);
        todayDate.push_back(t.tm_min);
        todayDate.push_back(t.tm_sec);
    }
    else {
        todayDate[0] = t.tm_mday;
        todayDate[1] = t.tm_mon + 1;
        todayDate[2] = t.tm_year + 1900;
        todayDate[3] = t.tm_hour;
        todayDate[4] = t.tm_min;
        todayDate[5] = t.tm_sec;
    }
}

// This will cause the wrapper to return a vector full of today's SPX price range in minute intervals
void App::retreiveRecentData(string interval, string duration, TickerId reqId) {

    //================================================
    // For some reason, using gmt time tends to cause
    // less errors when using time windows of one hour
    // or less with historical data, using a temporary
    // solution here in leiu of further api updates
    // just for the purpose of testing
    //================================================

    IBString endTime;

    if (!useTestData) {
        getDateTime();
        endTime = EndDateTime(todayDate[2], todayDate[1], todayDate[0], todayDate[3],
            todayDate[4], todayDate[5]) + " US/Central";
    }
    else
    {
        std::time_t rawtime;
        std::tm* timeinfo;
        char queryTime[80];
        std::time(&rawtime);
        timeinfo = std::gmtime(&rawtime);
        std::strftime(queryTime, 80, "%Y%m%d-%H:%M:%S", timeinfo);

        endTime = queryTime;
    }

    EC->reqHistoricalData
    (reqId
        , underlying
        , endTime
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

    for (auto i : YW.underlyingCandles) prices.push_back(i);

    // Once completed, clear the wrapper vector for next callback
    YW.underlyingCandles.clear();
}

// The multiple variable is the increments of options strikes
void App::populateStrikes(int multiple, int reqId) {
    // Retrieve the latest Underlying price
    retreiveRecentData("1 min", "1 D", reqId);

    // Clear the strikes vector
    if (!strikes.empty()) strikes.clear();

    // Round the price down to nearest increment
    double currentPrice = prices[prices.size() - 1].close;
    int roundedPrice = int(currentPrice + (multiple / 2));
    roundedPrice -= roundedPrice % multiple;
    int strikePrice = roundedPrice - (multiple * 4);

    // This will give us 9 strikes in total
    while (strikePrice <= roundedPrice + (multiple * 4)) {
        strikes.push_back(strikePrice);
        strikePrice += multiple;
    }

    for (auto i : strikes) cout << i << " ";
    cout << endl;

}