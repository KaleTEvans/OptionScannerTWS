
#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <unordered_set>

///Easier: Just one include statement for all functionality
#include "TwsApiL0.h"

///Faster: Check spelling of parameter at compile time instead of runtime.
#include "TwsApiDefs.h"
using namespace TwsApi; // for TwsApiDefs.h

#include "tWrapper.h"

class MockWrapper {
public:
   // 18 Candles for each option strikle
    CandleBuffer candleBuffer{ 18 };

    // Candle to be used repeatedly for realtime bars
    Candle underlyingRTBs;
    // Boolean if only needing the single realtime bars
    bool singleRTBs = false;
    
    int getReq();
    std::vector<Candle> getUnderlyingCandles();
    std::vector<Candle> getFiveSecCandles();
    

    virtual void connectionOpened();
    virtual void connectionClosed();
    virtual void currentTime(long time);

    virtual void historicalData(TickerId reqId, const IBString& date
        , double open, double high, double low, double close
        , int volume, int barCount, double WAP, int hasGaps);

    virtual void realtimeBar(TickerId reqId, long time, double open, double high,
        double low, double close, long volume, double wap, int count);

private:
    int Req = 0;

    std::vector<Candle> underlyingCandles;
    std::vector<Candle> fiveSecCandles;

    // Variables to show data request output
    bool showHistoricalData = false;
    bool showRealTimeData = false;

};

