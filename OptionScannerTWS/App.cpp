#include "App.h"

App::App(const char* host) : host(host), YW(23, false) {

    // Initialize connection
    EC = EClientL0::New(&YW);
    // Connect to TWS
    EC->eConnect(host, 7496, 0);

    // NOTE : Some API functions will encounter issues if called immediately after connection
    //          Here we will start with a simple request for the TWS current time and wait
    // ** Also note that this will return time in Unix form for the current time zone of the server hosting tws
    EC->reqCurrentTime();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EC->checkMessages();
    
    std::cout << YW.getCurrentTime() << std::endl;
}

App::~App() {
    EC->eDisconnect();
    delete EC;

}

// This will cause the wrapper to return a vector full of today's SPX price range in minute intervals
//void App::retreiveRecentData(string interval, string duration, TickerId reqId) {
//
//    //================================================
//    // For some reason, using gmt time tends to cause
//    // less errors when using time windows of one hour
//    // or less with historical data, using a temporary
//    // solution here in leiu of further api updates
//    // just for the purpose of testing
//    //================================================
//
//    IBString endTime;
//
//    if (!useTestData) {
//        getDateTime();
//        endTime = EndDateTime(todayDate[2], todayDate[1], todayDate[0], todayDate[3],
//            todayDate[4], todayDate[5]) + " US/Central";
//    }
//    else
//    {
//        std::time_t rawtime;
//        std::tm* timeinfo;
//        char queryTime[80];
//        std::time(&rawtime);
//        timeinfo = std::gmtime(&rawtime);
//        std::strftime(queryTime, 80, "%Y%m%d-%H:%M:%S", timeinfo);
//
//        endTime = queryTime;
//    }
//
//    EC->reqHistoricalData
//    (reqId
//        , underlying
//        , endTime
//        , duration
//        , interval
//        , *WhatToShow::TRADES
//        , UseRTH::OnlyRegularTradingData
//        , FormatDate::AsDate
//        , false
//    );
//
//    // Wait for the response
//    while (YW.notDone()) {
//        EC->checkMessages();
//        if (YW.Req == reqId) break;
//    }
//
//    // Clear out the prices vector and repopulate
//    if (!prices.empty()) prices.clear();
//
//    for (auto i : YW.underlyingCandles) prices.push_back(i);
//
//    // Once completed, clear the wrapper vector for next callback
//    YW.underlyingCandles.clear();
//}