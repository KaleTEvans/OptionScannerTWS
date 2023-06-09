#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <ctime>
#include <bitset>

#include "tWrapper.h"
#include "App.h"
#include "OptionScanner.h"
#include "ContractData.h"
#include "AlertHandler.h"

//=======================================================================
// Informal testing functions
//=======================================================================

// Turn on tests or main program here
constexpr bool runTests = true;

// Main launcher for imformal tests
void informalTests();
// Tests
void basicHistoricalDataRequest(App * test);
void testRealTimeBars(App* test);
void candleFunctionality(App* test);
void testStreamingAlerts(App* test);
void testLowAndHighAccuracy(App* test);
void testContractVolumes(App* test);
// Test Helper Functions
bool compareCandles(Candle c1, Candle c2);

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

        OptionScanner* opt = new OptionScanner(host, ticker);

        //opt->streamOptionData();
        opt->populateStrikes();

        //delete opt;
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
    // Test functionality of 5 second real time bars for 30 seconds
    constexpr bool showRealTimeBarsTest = false;
    // Test ability for ContractData to create functional candles in different time frames
    constexpr bool showCandleFunctionality = true;
    // Test ability to receive alerts from ContractData as callbacks
    constexpr bool showAlertFunctionality = false;
    // Test the accuracy of daily high/low attributes from ContractData
    constexpr bool showDailyLowHighAccuracy = false;
    // Test the accuracy of the volume in a few contracts
    constexpr bool testVolumeAccuracy = false;

    // Run tests here
    if (showWelcome) {
        cout << "========================================================================" << endl;
        cout << "Beginning tests ..." << endl;
    }

    // Create a connection to TWS for use in all functions
    App* test = new App("127.0.0.1", "SPY");

    if (showBasicRequest) basicHistoricalDataRequest(test);
    if (showRealTimeBarsTest) testRealTimeBars(test);
    if (showCandleFunctionality) candleFunctionality(test);
    if (showAlertFunctionality) testStreamingAlerts(test);
    if (showDailyLowHighAccuracy) testLowAndHighAccuracy(test);
    if(testVolumeAccuracy) testContractVolumes(test);

    delete test;
}

//=================================================================
// Test functionality of a basic historical data request
//=================================================================

void basicHistoricalDataRequest(App* test) {
    
    cout << "========================================================================" << endl;
    cout << "Testing Historical Data Functionality" << endl;
    cout << "========================================================================" << endl;

    test->YW.showHistoricalData = true;
    test->useTestData = true;

    Contract C;
    C.symbol = "AAPL";
    C.secType = "STK";
    C.currency = "USD";
    C.exchange = "SMART";
    // C.primaryExchange = *Exchange::AMEX;

    test->retreiveRecentData("5 secs", "3600 S", 10);
    
    cout << "Num test candles received: " << test->prices.size() << endl;

    /*for (auto i : test->prices) {
        fprintf(stdout, "%10s, %5.3f, %5.3f, %5.3f, %5.3f, %7d\n"
            , (const  char*)i.date, i.open, i.high, i.low, i.close, i.volume);
    }*/

}

//=================================================================
// Test functionality of 5 second real time bars for 30 seconds
//=================================================================

void testRealTimeBars(App* test) {

    cout << "========================================================================" << endl;
    cout << "Testing Real Time Bar Output" << endl;
    cout << "========================================================================" << endl;

    test->YW.showRealTimeData = true;

    Contract C;
    C.symbol = "SPX";
    C.secType = "IND";
    C.currency = "USD";
    C.exchange = "SMART";
    C.primaryExchange = "CBOE";

    test->EC->reqRealTimeBars
    (20
        , C
        , 5
        , *WhatToShow::MIDPOINT
        , UseRTH::AllTradingData
    );

    // Let run for 15 secs
    int timer = 15;

    while (test->YW.notDone()) {
        if (timer <= 0) break;
        test->EC->checkMessages();

        Sleep(1000);
        timer -= 1;
    }
}

//======================================================================================
// Test ability for ContractData to create functional candles in different time frames
//======================================================================================

void candleFunctionality(App* test) {

    cout << "========================================================================" << endl;
    cout << "Testing Candle Functionality" << endl;
    cout << "========================================================================" << endl;
    cout << "Testing to ensure that as 5 second candles are recieved, they will be   " << endl;
    cout << "correctly organized into the 30sec, 1min, and 5min candles. This test   " << endl;
    cout << "will use 5 second intervals from spy price action and compare to each   " << endl;
    cout << "other interval." << endl;
    cout << endl;
    cout << "Retrieving data ..." << endl;

    test->useTestData = true;

    test->retreiveRecentData("5 secs", "3600 S", 21);

    // Vectors to store data for each candle size
    vector<Candle> fiveSec;
    vector<Candle> thirtySec;
    vector<Candle> oneMin;
    vector<Candle> fiveMin;

    int histTotalVol = 0;

    cout << "========================================================================" << endl;

    for (auto i : test->prices) {
        fiveSec.push_back(i);
        histTotalVol += i.volume;
    }
    cout << "5 second data for SPY received, size: " << fiveSec.size() << " bars" << endl;

    // Repeat for other intervals
    test->retreiveRecentData("30 secs", "3600 S", 22);
    for (auto i : test->prices) thirtySec.push_back(i);
    cout << "30 second data for SPY received, size: " << thirtySec.size() << " bars" << endl;

    test->retreiveRecentData("1 min", "3600 S", 23);
    for (auto i : test->prices) oneMin.push_back(i);
    cout << "1 min data for SPY received, size: " << oneMin.size() << " bars" << endl;

    test->retreiveRecentData("5 mins", "3600 S", 24);
    for (auto i : test->prices) fiveMin.push_back(i);
    cout << "5 min data for SPY received, size: " << fiveMin.size() << " bars" << endl;

    // Now create the Contract Data instance and compare candles
    ContractData testCon(111, fiveSec[0]);

    cout << "Initialized Data" << endl;

    // Use update data to fill the arrays
    for (size_t i = 1; i < fiveSec.size(); i++) testCon.updateData(fiveSec[i]);

    // Now compare the data
    cout << endl;
    cout << "--- Comparing Data ---" << endl;
    vector<Candle> test5Sec = testCon.getFiveSecData();
    vector<Candle> test30Sec = testCon.getThirtySecData();
    vector<Candle> test1Min = testCon.getOneMinData();
    vector<Candle> test5Min = testCon.getFiveMinData();

    for (size_t i = 0; i < test30Sec.size(); i++) {
        if (!compareCandles(thirtySec[i], test30Sec[i])) {
            OPTIONSCANNER_ERROR("Error: 30 Second candle comparison incorrect");
            return;
        }
    }
    cout << "30 Second candles matched correctly" << endl;
    for (size_t i = 0; i < test1Min.size(); i++) {
        if (!compareCandles(oneMin[i], test1Min[i])) {
            OPTIONSCANNER_ERROR("Error: 1 Minute candle comparison incorrect");
            return;
        }
    }
    cout << "One Minute candles matched correctly" << endl;
    for (size_t i = 0; i < test5Min.size(); i++) {
        if (!compareCandles(fiveMin[i], test5Min[i])) {
            return;
        }
    }
    cout << "Five minute candles matched correctly" << endl;

    cout << "Historical Cumulative Vol: " << histTotalVol << " Test Cumulative Vol: " << testCon.getCumulativeVol() << endl;
}

//==================================================================
// Test candle functionality when inputting real time bar data
//==================================================================

void testStreamingAlerts(App* test) {

    cout << "========================================================================" << endl;
    cout << "Testing Alert Capabilities For Streaming Data" << endl;
    cout << "========================================================================" << endl;
    cout << "Testing to ensure that alerts are capable of being sent back to the     " << endl;
    cout << "parent class, Option Scanner from each individual ContractData class via" << endl;
    cout << "callback functionality. Will use SPX if within market hours, or an hour " << endl;
    cout << "worth of SPY data when outside market hours" << endl;
    cout << endl;
    cout << "Retreiving Data ...." << endl;

    test->useTestData = true;

    vector<Candle> fiveSecData;
    vector<Candle> testFiveSec;

    // Retrieve data from spy
    test->retreiveRecentData("5 secs", "3600 S", 31);
    for (auto i : test->prices) fiveSecData.push_back(i);

    // Register the callback
    ContractData testCon(112, fiveSecData[0]);
    testCon.registerAlert([&](int data, StandardDeviation sdVol, StandardDeviation sdPrice, Candle c) {
        //cout << "Callback Received: " << data << " stdev: " << sdVol.getStDev() << " volume: " << c.volume << " close price: " << c.close << endl;
    });

    // Fill ContractData object with remaining data
    for (size_t i = 1; i < fiveSecData.size(); i++) testCon.updateData(fiveSecData[i]);
}

//==================================================================
// Test the accuracy of daily high/low attributes from ContractData
//==================================================================

void testLowAndHighAccuracy(App* test) {

    cout << "========================================================================" << endl;
    cout << "Testing the accuracy of the low and high booleans in ContractData" << endl;
    cout << "========================================================================" << endl;
    cout << "Currently being within 0.1% of the high or low of the daily or local" << endl;
    cout << "price range will constitute a true output. This test will determine" << endl;
    cout << "whether or not this is accurate enough to be a valuable point of data" << endl;
    cout << endl;
    cout << "Retreiving data ...." << endl;

    test->useTestData = true;

    test->retreiveRecentData("5 secs", "3600 S", 31);
    // Store the data
    vector<Candle> fiveSec;
    for (auto i : test->prices) fiveSec.push_back(i);

    // Now create the Contract Data instance and compare candles
    ContractData testCon(131, fiveSec[0]);

    cout << "Initialized Data" << endl;

    // While filling ContractData, check every 60 iterations, which would be 1 minute, and
    // when the comparisons are updated
    for (size_t i = 1; i < fiveSec.size(); i++) {
        if (i % 60 == 0) {
            vector<bool> v = testCon.getHighLowComparisons();
            cout << "==================================================================" << endl;
            cout << "CURRENT PRICE: " << testCon.getCurrentPrice() << endl;
            cout << "Current vector state: " << v[0] << " " << v[1] << " " << v[2] << " " << v[3] << endl;
            cout << "Current conditions: ";
            (v[0]) ? cout << "near DL | " : cout << "not near DL | ";
            (v[1]) ? cout << "near DH | " : cout << "not near DH | ";
            (v[2]) ? cout << "near LL | " : cout << "not near LL | ";
            (v[3]) ? cout << "near LH | " : cout << "not near LH | ";
            cout << "Daily Low: " << testCon.getDailyLow() << " | Daily High: " << testCon.getDailyHigh() << endl;
            cout << "Local Low: " << testCon.getLocalLow() << " | Local Low: " << testCon.getLocalLow() << endl;
            cout << "===================================================================" << endl;
        }

        testCon.updateData(fiveSec[i]);
    }
}

//============================================================
//Test the accuracy of the volume in a few contracts
//===========================================================

void testContractVolumes(App* test) {
    cout << "========================================================================" << endl;
    cout << "Testing the accuracy of volume data in a contract " << endl;
    cout << "========================================================================" << endl;
    cout << "This test will gather data for a certain contract for a few minutes, " << endl;
    cout << "and then make a historical request to check that the values match up" << endl;
    cout << endl;
    cout << "Retreiving data ...." << endl;

    Contract C;
    C.symbol = "SPX";
    C.secType = *SecType::OPT;
    C.currency = "USD";
    C.exchange = *Exchange::IB_SMART;
    C.primaryExchange = *Exchange::CBOE;
    C.right = *ContractRight::CALL;

    // Retrieve Date
    test->getDateTime();
    vector<int> date = test->getDateVector();

    C.lastTradeDateOrContractMonth = EndDateTime(date[2], date[1], date[0]);

    // Get the strike
    test->populateStrikes(41);
    int strike = test->strikes[4];

    C.strike = strike;

    test->EC->reqRealTimeBars
    (42
        , C
        , 5
        , *WhatToShow::MIDPOINT
        , UseRTH::OnlyRegularTradingData
    );

    test->YW.singleRTBs = true;
    // Initialize contract data
    ContractData* testCon = nullptr;

    bool firstCallback = true;

    while (test->YW.notDone()) {
        // Make sure only one instance of the ContractData is created
        if (test->YW.underlyingRTBs.reqId == 42 && firstCallback) {
            testCon = new ContractData(test->YW.underlyingRTBs.reqId, test->YW.underlyingRTBs, true);
            firstCallback = false;
        }
        
        // After 5 minutes, break the loop
        if (testCon->getOneMinData().size() >= 5) break;

        testCon->updateData(test->YW.underlyingRTBs);
    }

    // Now, we'll make a historical request of the last 5 mintues
    std::time_t rawtime;
    std::tm* timeinfo;
    char queryTime[80];
    std::time(&rawtime);
    timeinfo = std::gmtime(&rawtime);
    std::strftime(queryTime, 80, "%Y%m%d-%H:%M:%S", timeinfo);

    test->EC->reqHistoricalData
    (43
        , C
        , queryTime  
        , "3600 S"
        , "1 mins"
        , *WhatToShow::TRADES
        , UseRTH::OnlyRegularTradingData
        , FormatDate::AsDate
        , false
        , TagValueListSPtr()
    );

    vector<Candle> histData = test->prices;
    int volSum = 0;
    // Get the last 5 candles
    int j = 0; // For the contract vector
    for (size_t i = histData.size() - 6; i < histData.size(); i++) {
        cout << "1 Min volume from RTBs: " << testCon->getOneMinData()[j].volume << " at time: " << testCon->getOneMinData()[i].time <<
            " 1 Min volume from historical request: " << histData[i].volume << " at time: " << histData[i].date << endl;

        volSum += histData[i].volume;
        j++;
    }

    // Now output the cumulative volume
    cout << "Total vol for RTB data: " << testCon->getCumulativeVol() << " | Total vol for hist data: " << volSum << endl;
}

bool compareCandles(Candle c1, Candle c2) {
    // Allow for a 1% error in comparison
    if (abs(c1.open - c2.open) / c1.open > 0.01) return false;
    if (abs(c1.close - c2.close) / c1.close > 0.01) return false;
    if (abs(c1.high - c2.high) / c1.high > 0.01) return false;
    if (abs(c1.low - c2.low) / c1.low > 0.01) return false;
    if (abs(c1.volume - c2.volume) / c1.volume > 0.01) { // Volume will not be accurate when testing different historical timeframes
        //cout << "Volume: " << c1.volume << " | " << c2.volume << endl;
        //return false;
    }

    return true;
}

