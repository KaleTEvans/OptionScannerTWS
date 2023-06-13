#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <ctime>

#include "tWrapper.h"
#include "App.h"
#include "OptionScanner.h"
#include "ContractData.h"

//=======================================================================
// Informal testing functions
//=======================================================================

// Main launcher for imformal tests
void informalTests();
// Tests
void candleFunctionality();
void candlesWithRealTimeBars();
// Test Helper Functions
bool compareCandles(Candle c1, Candle c2);

//========================================================================
// main entry
//========================================================================
int main(void) {

    //informalTests();


    ///Easier: just allocate your wrapper and instantiate the EClientL0 with it.
    const char* host = "127.0.0.1";
    IBString ticker = "SPX";

    OptionScanner* opt = new OptionScanner(host, ticker);

    opt->populateStrikes();
    //opt->streamOptionData();

    //Contract SPXchain;
    //SPXchain.symbol = "SPX";
    //SPXchain.secType = *SecType::OPT;
    //SPXchain.currency = "USD";
    //SPXchain.exchange = *Exchange::IB_SMART;
    //SPXchain.primaryExchange = *Exchange::CBOE;
    //SPXchain.right = *ContractRight::CALL;
    //SPXchain.expiry = EndDateTime(2023, 06, 9);
    //SPXchain.strike = 4230;

    //opt->EC->reqRealTimeBars
    //(4230
    //    , SPXchain
    //    , 5
    //    , *WhatToShow::TRADES
    //    , UseRTH::OnlyRegularTradingData
    //);

    //Easier: Call checkMessages() in a loop. No need to wait between two calls.
    //while (opt->YW.notDone()) {
    //    opt->EC->checkMessages();

    //    /*std::time_t tmNow = std::time(NULL);
    //    struct tm t = *localtime(&tmNow);
    //    if (t.tm_hour == 13 && t.tm_min == 39) opt->EC->cancelRealTimeBars(4275);*/

    //    if (opt->YW.Req == 101) opt->YW.m_Done = true;

    //    // Set m_Done to true when you no longer wish to receive messages from the wrapper
    //    //======================
    //    // opt.YW->m_Done = true 
    //    //======================
    //}

   /* for (auto i : opt->prices) {
        cout << i.close << endl;
    }*/

    //delete opt;

    return 0;
}


//===================================================================
// Testing functionality
//===================================================================

void informalTests() {
    // Can enable and disable tests here

    // Show welcome will be the initial header for testing options
    constexpr bool showWelcome = true;

    constexpr bool showCandleFunctionality = true;

    // Run tests here
    if (showWelcome) {
        cout << "========================================================================" << endl;
        cout << "Beginning tests ..." << endl;
    }

    if (showCandleFunctionality) candleFunctionality();
}

void candleFunctionality() {

    App* test = new App("127.0.0.1", "SPY");

    cout << "========================================================================" << endl;
    cout << "Testing Candle Functionality" << endl;
    cout << "========================================================================" << endl;
    cout << "Testing to ensure that as 5 second candles are recieved, they will be   " << endl;
    cout << "correctly organized into the 30sec, 1min, and 5min candles. This test   " << endl;
    cout << "will use 5 second intervals from spy price action and compare to each   " << endl;
    cout << "other interval." << endl;
    cout << endl;
    cout << "Retrieving data ..." << endl;

    test->retreiveUnderlyingPrice("5 secs", "3600 S", 201);

    // Vectors to store data for each candle size
    vector<Candle> fiveSec;
    vector<Candle> thirtySec;
    vector<Candle> oneMin;
    vector<Candle> fiveMin;

    cout << "========================================================================" << endl;

    for (auto i : test->prices) fiveSec.push_back(i);
    cout << "5 second data for SPY received, size: " << fiveSec.size() << " bars" << endl;

    // Repeat for other intervals
    test->retreiveUnderlyingPrice("30 secs", "3600 S", 202);
    for (auto i : test->prices) thirtySec.push_back(i);
    cout << "30 second data for SPY received, size: " << thirtySec.size() << " bars" << endl;

    test->retreiveUnderlyingPrice("1 min", "3600 S", 203);
    for (auto i : test->prices) oneMin.push_back(i);
    cout << "1 min data for SPY received, size: " << oneMin.size() << " bars" << endl;

    test->retreiveUnderlyingPrice("5 mins", "3600 S", 204);
    for (auto i : test->prices) fiveMin.push_back(i);
    cout << "5 min data for SPY received, size: " << fiveMin.size() << " bars" << endl;

    // Now create the Contract Data instance and compare candles
    ContractData testCon(111, fiveSec[0]);
    // Use update data to fill the arrays
    for (int i = 1; i < fiveSec.size(); i++) testCon.updateData(fiveSec[i]);
    // Now compare the data
    cout << endl;
    cout << "--- Comparing Data ---" << endl;
    vector<Candle> test5Sec = testCon.getFiveSecData();
    vector<Candle> test30Sec = testCon.getThirtySecData();
    vector<Candle> test1Min = testCon.getOneMinData();
    vector<Candle> test5Min = testCon.getFiveMinData();
    
    // Make sure sizes are the same
    if (test5Sec.size() != fiveSec.size() || test30Sec.size() != thirtySec.size()
        || test1Min.size() != oneMin.size() || test5Min.size() != fiveMin.size()) throw std::runtime_error("Error: Vector sizes do not match");

    for (int i = 0; i < thirtySec.size(); i++) {
        if (!compareCandles(thirtySec[i], test30Sec[i])) throw std::runtime_error("Error: 30 Second candle comparison incorrect");
    }
    cout << "30 Second candles matched correctly" << endl;
    for (int i = 0; i < oneMin.size(); i++) {
        if (!compareCandles(oneMin[i], test1Min[i])) throw std::runtime_error("Error: 1 Minute candle comparison incorrect");
        /*cout << "TWS open: " << thirtySec[i].open << " close: " << thirtySec[i].close << " high: " << thirtySec[i].high << " low: " << thirtySec[i].low << " volume: " << thirtySec[i].volume
            << " CD open: " << test30Sec[i].open << " close: " << test30Sec[i].close << " high: " << test30Sec[i].high << " low: " << test30Sec[i].low << " volume: " << test30Sec[i].volume << endl;*/
    }
    cout << "One Minute candles matched correctly" << endl;
    for (int i = 0; i < fiveMin.size(); i++) {
        if (!compareCandles(fiveMin[i], test5Min[i])) throw std::runtime_error("Error: 5 Minute candle comparison incorrect");
    }
    cout << "Five minute candles matched correctly" << endl;

    delete test;
}

// Test candle functionality when inputting real time bar data
void candlesWithRealTimeBars() {

}

bool compareCandles(Candle c1, Candle c2) {
    // Allow for a 1% error in comparison
    if (abs(c1.open - c2.open)/c1.open > 0.01) return false;
    if (abs(c1.close - c2.close) / c1.close > 0.01) return false;
    if (abs(c1.high - c2.high) / c1.high > 0.01) return false;
    if (abs(c1.low - c2.low) / c1.low > 0.01) return false;
    if (abs(c1.volume - c2.volume) / c1.volume > 0.01) return false;

    return true;
}