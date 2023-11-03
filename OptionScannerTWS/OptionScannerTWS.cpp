#define _CRT_SECURE_NO_WARNINGS

#ifndef TEST_CONFIG

#include <iostream>
#include <memory>
#include <ctime>

#define DEBUG_LOGS

#include "Candle.h"
#include "tWrapper.h"
#include "Logger.h"
#include "SQLSchema.h"
#include "CandleRoutes.h"
#include "AlertRoutes.h"
#include "App.h"
#include "OptionScanner.h"
#include "ContractData.h"
#include "AlertHandler.h"

using std::cout;
using std::endl;

//=======================================================================
// Informal testing functions
//=======================================================================

// Turn on tests or main program here
constexpr bool runTests = true;

// Main launcher for imformal tests
void informalTests();
// Tests
void basicHistoricalDataRequest(std::unique_ptr<App>& test);
void testMktDataRequest(std::unique_ptr<App>& test);
void testRealTimeBars(std::unique_ptr<App>& test);
void runAlertDBTests();
//void candleFunctionality(std::unique_ptr<TestConnections>& test);
//void testStreamingAlerts(std::unique_ptr<TestConnections>& test);
//void testLowAndHighAccuracy(std::unique_ptr<TestConnections>& test);
// Test Helper Functions
//bool compareCandles(Candle c1, Candle c2);

//========================================================================
// main entry
//========================================================================

int main(void) {

    Logger::Initialize();
    
    if (runTests) {
        informalTests();
    }

    else {
        const char* host = "127.0.0.1";
        IBString ticker = "SPX";

        //std::unique_ptr<OptionScanner> opt = std::make_unique<OptionScanner>(host, ticker);
        OptionScanner* opt = new OptionScanner(host, ticker);
        opt->streamOptionData();

       /* std::thread t([&] {
            opt->streamOptionData();
        });

        std::this_thread::sleep_for(std::chrono::seconds(30));
        t.join();*/

        /*while (true) {
            std::unique_lock<std::mutex> lock(opt->optScanMtx());
            opt->optScanCV().wait(lock, [&] { return opt->strikesUpdated(); });

            opt->outputChainData();
            opt->changeStrikesUpdated();

            lock.unlock();
        }*/
    }

    Logger::Shutdown();

    return 0;
}


//===================================================================
// Testing functionality
//===================================================================

void informalTests() {
    // Can enable and disable tests here

    // Show welcome will be the initial header for testing options
    constexpr bool showWelcome = true;

    // Test functionality of a basic historical data request
    constexpr bool showBasicRequest = false;
    // Test different outputs of a market data request
    constexpr bool showMktDataRequest = false;
    // Test functionality of 5 second real time bars for 30 seconds
    constexpr bool showRealTimeBarsTest = false;

    // DB Tests
    constexpr bool testAlertTables = true;

    // Test ability for ContractData to create functional candles in different time frames
    // constexpr bool showCandleFunctionality = false;
    // Test ability to receive alerts from ContractData as callbacks
    // constexpr bool showAlertFunctionality = false;
    // Test the accuracy of daily high/low attributes from ContractData
    // constexpr bool showDailyLowHighAccuracy = false;

    // Run tests here
    if (showWelcome) {
        cout << "========================================================================" << endl;
        cout << "Beginning tests ..." << endl;
    }

    // Create a connection to TWS for use in all functions
    std::unique_ptr<App> test = std::make_unique<App>("127.0.0.1");
    cout << "Current Time: " << test->YW.getCurrentTime() << endl;

    if (showBasicRequest) basicHistoricalDataRequest(test);
    if (showMktDataRequest) testMktDataRequest(test);
    if (showRealTimeBarsTest) testRealTimeBars(test);
    if (testAlertTables) runAlertDBTests();
    //if (showCandleFunctionality) candleFunctionality(test);
    //if (showAlertFunctionality) testStreamingAlerts(test);
    //if (showDailyLowHighAccuracy) testLowAndHighAccuracy(test);

    // delete test;
}
//
////=================================================================
//// Test functionality of a basic historical data request
////=================================================================

void basicHistoricalDataRequest(std::unique_ptr<App>& test) {
    
    cout << "========================================================================" << endl;
    cout << "Testing Historical Data Functionality" << endl;
    cout << "========================================================================" << endl;

    test->YW.showHistoricalDataOutput();

    Contract C;
    C.symbol = "SPX";
    C.secType = "OPT";
    C.currency = "USD";
    C.exchange = *Exchange::IB_SMART;
    C.primaryExchange = *Exchange::CBOE;
    C.lastTradeDateOrContractMonth = "20230915";
    C.right = "PUT";
    C.includeExpired = true;
    C.strike = 4450;

    test->EC->reqHistoricalData
    (10
        , C
        , "20230915 13:00:00 US/Central"
        , "3600 S"
        , "1 min"
        , *WhatToShow::TRADES
        , UseRTH::OnlyRegularTradingData
        , FormatDate::AsDate
        , false
    );

    while (test->YW.notDone()) {
        test->EC->checkMessages();
        if (test->YW.getReqId() == 10) break;
    }
    
    cout << "Num test candles received: " << test->YW.historicCandles().size() << endl;
}

//=================================================================
// Test functionality of a market data request
//=================================================================

void testMktDataRequest(std::unique_ptr<App>& test) {

    cout << "========================================================================" << endl;
    cout << "Testing Market Data Output" << endl;
    cout << "========================================================================" << endl;

    Contract C;
    C.symbol = "SPX";
    C.secType = "IND";
    C.currency = "USD";
    C.exchange = "SMART";
    C.primaryExchange = *Exchange::CBOE;

    test->EC->reqMktData(11, C, *GenericTicks::MiscellaneousStats, true);

    while (test->YW.getReqId() != 11) {
        test->EC->checkMessages();
    }

    cout << test->YW.lastTickPrice() << endl;
}

////=================================================================
//// Test functionality of 5 second real time bars for 30 seconds
////=================================================================

void testRealTimeBars(std::unique_ptr<App>& test) {

    cout << "========================================================================" << endl;
    cout << "Testing Real Time Bar Output" << endl;
    cout << "========================================================================" << endl;

    test->YW.showRealTimeDataOutput();

    Contract C;
    C.symbol = "SPX";
    C.secType = "IND";
    C.currency = "USD";
    //C.exchange = "SMART";
    C.primaryExchange = "CBOE";

    test->EC->reqRealTimeBars
    (20
        , C
        , 5
        , *WhatToShow::TRADES
        , UseRTH::OnlyRegularTradingData
    );

    // Let run for 15 secs
    int timer = 15;

    while (test->YW.notDone()) {
        if (timer <= 0) break;
        test->EC->checkMessages();

        std::this_thread::sleep_for(std::chrono::seconds(1));
        timer -= 1;
    }
}

void runAlertDBTests() {
    OptionDB::DatabaseManager dbm;
    dbm.setAlertTables();
}

////======================================================================================
//// Test ability for ContractData to create functional candles in different time frames
////======================================================================================
//
//void candleFunctionality(std::unique_ptr<App>& test) {
//
//    cout << "========================================================================" << endl;
//    cout << "Testing Candle Functionality" << endl;
//    cout << "========================================================================" << endl;
//    cout << "Testing to ensure that as 5 second candles are recieved, they will be   " << endl;
//    cout << "correctly organized into the 30sec, 1min, and 5min candles. This test   " << endl;
//    cout << "will use 5 second intervals from spy price action and compare to each   " << endl;
//    cout << "other interval." << endl;
//    cout << endl;
//    cout << "Retrieving data ..." << endl;
//
//    test->useTestData = true;
//    test->createUnderlyingContract("SPY", false);
//
//    test->retreiveRecentData("5 secs", "3600 S", 21);
//
//    // Vectors to store data for each candle size
//    vector<Candle> fiveSec;
//    vector<Candle> thirtySec;
//    vector<Candle> oneMin;
//    vector<Candle> fiveMin;
//
//    int histTotalVol = 0;
//
//    cout << "========================================================================" << endl;
//
//    for (auto i : test->prices) {
//        fiveSec.push_back(i);
//        histTotalVol += i.volume;
//    }
//    cout << "5 second data for SPY received, size: " << fiveSec.size() << " bars" << endl;
//
//    // Repeat for other intervals
//    test->retreiveRecentData("30 secs", "3600 S", 22);
//    for (auto i : test->prices) thirtySec.push_back(i);
//    cout << "30 second data for SPY received, size: " << thirtySec.size() << " bars" << endl;
//
//    test->retreiveRecentData("1 min", "3600 S", 23);
//    for (auto i : test->prices) oneMin.push_back(i);
//    cout << "1 min data for SPY received, size: " << oneMin.size() << " bars" << endl;
//
//    test->retreiveRecentData("5 mins", "3600 S", 24);
//    for (auto i : test->prices) fiveMin.push_back(i);
//    cout << "5 min data for SPY received, size: " << fiveMin.size() << " bars" << endl;
//
//    // Now create the Contract Data instance and compare candles
//    ContractData testCon(111, fiveSec[0]);
//
//    cout << "Initialized Data" << endl;
//
//    // Use update data to fill the arrays
//    for (size_t i = 1; i < fiveSec.size(); i++) testCon.updateData(fiveSec[i]);
//
//    // Now compare the data
//    cout << endl;
//    cout << "--- Comparing Data ---" << endl;
//    vector<Candle> test5Sec = testCon.getFiveSecData();
//    vector<Candle> test30Sec = testCon.getThirtySecData();
//    vector<Candle> test1Min = testCon.getOneMinData();
//    vector<Candle> test5Min = testCon.getFiveMinData();
//
//    for (size_t i = 0; i < test30Sec.size(); i++) {
//        if (!compareCandles(thirtySec[i], test30Sec[i])) {
//            OPTIONSCANNER_ERROR("Error: 30 Second candle comparison incorrect");
//            return;
//        }
//    }
//    cout << "30 Second candles matched correctly" << endl;
//    for (size_t i = 0; i < test1Min.size(); i++) {
//        if (!compareCandles(oneMin[i], test1Min[i])) {
//            OPTIONSCANNER_ERROR("Error: 1 Minute candle comparison incorrect");
//            return;
//        }
//    }
//    cout << "One Minute candles matched correctly" << endl;
//    for (size_t i = 0; i < test5Min.size(); i++) {
//        if (!compareCandles(fiveMin[i], test5Min[i])) {
//            OPTIONSCANNER_ERROR("Error: 5 Minute candle comparison incorrect");
//            return;
//        }
//    }
//    cout << "Five minute candles matched correctly" << endl;
//
//    cout << "Historical Cumulative Vol: " << histTotalVol << " Test Cumulative Vol: " << testCon.getCumulativeVol() << endl;
//}
//
////==================================================================
//// Test candle functionality when inputting real time bar data
////==================================================================
//
//void testStreamingAlerts(std::unique_ptr<App>& test) {
//
//    cout << "========================================================================" << endl;
//    cout << "Testing Alert Capabilities For Streaming Data" << endl;
//    cout << "========================================================================" << endl;
//    cout << "Testing to ensure that alerts are capable of being sent back to the     " << endl;
//    cout << "parent class, Option Scanner from each individual ContractData class via" << endl;
//    cout << "callback functionality. Will use SPX if within market hours, or an hour " << endl;
//    cout << "worth of SPY data when outside market hours" << endl;
//    cout << endl;
//    cout << "Retreiving Data ...." << endl;
//
//    test->useTestData = true;
//    test->createUnderlyingContract("SPY");
//
//    vector<Candle> fiveSecData;
//    vector<Candle> testFiveSec;
//
//    // Retrieve data from spy
//    test->retreiveRecentData("5 secs", "3600 S", 31);
//    for (auto i : test->prices) fiveSecData.push_back(i);
//
//    // Register the callback
//    ContractData testCon(112, fiveSecData[0]);
//    testCon.registerAlert([&](int data, StandardDeviation sdVol, StandardDeviation sdPrice, Candle c) {
//        //cout << "Callback Received: " << data << " stdev: " << sdVol.getStDev() << " volume: " << c.volume << " close price: " << c.close << endl;
//    });
//
//    // Fill ContractData object with remaining data
//    for (size_t i = 1; i < fiveSecData.size(); i++) testCon.updateData(fiveSecData[i]);
//}
//
////==================================================================
//// Test the accuracy of daily high/low attributes from ContractData
////==================================================================
//
//void testLowAndHighAccuracy(std::unique_ptr<App>& test) {
//
//    cout << "========================================================================" << endl;
//    cout << "Testing the accuracy of the low and high booleans in ContractData" << endl;
//    cout << "========================================================================" << endl;
//    cout << "Currently being within 0.1% of the high or low of the daily or local" << endl;
//    cout << "price range will constitute a true output. This test will determine" << endl;
//    cout << "whether or not this is accurate enough to be a valuable point of data" << endl;
//    cout << endl;
//    cout << "Retreiving data ...." << endl;
//
//    test->useTestData = true;
//    test->createUnderlyingContract("SPY");
//
//    test->retreiveRecentData("5 secs", "3600 S", 31);
//    // Store the data
//    vector<Candle> fiveSec;
//    for (auto i : test->prices) fiveSec.push_back(i);
//
//    // Now create the Contract Data instance and compare candles
//    ContractData testCon(131, fiveSec[0]);
//
//    cout << "Initialized Data" << endl;
//
//    // While filling ContractData, check every 60 iterations, which would be 1 minute, and
//    // when the comparisons are updated
//    for (size_t i = 1; i < fiveSec.size(); i++) {
//        if (i % 60 == 0) {
//            vector<bool> v = testCon.getHighLowComparisons();
//            cout << "==================================================================" << endl;
//            cout << "CURRENT PRICE: " << testCon.getCurrentPrice() << endl;
//            cout << "Current vector state: " << v[0] << " " << v[1] << " " << v[2] << " " << v[3] << endl;
//            cout << "Current conditions: ";
//            (v[0]) ? cout << "near DL | " : cout << "not near DL | ";
//            (v[1]) ? cout << "near DH | " : cout << "not near DH | ";
//            (v[2]) ? cout << "near LL | " : cout << "not near LL | ";
//            (v[3]) ? cout << "near LH | " : cout << "not near LH | ";
//            cout << "Daily Low: " << testCon.getDailyLow() << " | Daily High: " << testCon.getDailyHigh() << endl;
//            cout << "Local Low: " << testCon.getLocalLow() << " | Local High: " << testCon.getLocalHigh() << endl;
//            cout << "===================================================================" << endl;
//        }
//
//        testCon.updateData(fiveSec[i]);
//    }
//}
//
//bool compareCandles(Candle c1, Candle c2) {
//    // Allow for a 1% error in comparison
//    if (abs(c1.open - c2.open) / c1.open > 0.01) return false;
//    if (abs(c1.close - c2.close) / c1.close > 0.01) return false;
//    if (abs(c1.high - c2.high) / c1.high > 0.01) return false;
//    if (abs(c1.low - c2.low) / c1.low > 0.01) return false;
//    if (abs(c1.volume - c2.volume) / c1.volume > 0.01) { // Volume will not be accurate when testing different historical timeframes
//        //cout << "Volume: " << c1.volume << " | " << c2.volume << endl;
//        //return false;
//    }
//
//    return true;
//}

#else

int main(void) { return 0; }

#endif // !TEST_CONFIG

