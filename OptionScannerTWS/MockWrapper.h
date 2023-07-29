
#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <memory>
#include <unordered_set>

///Easier: Just one include statement for all functionality
#include "TwsApiL0.h"

///Faster: Check spelling of parameter at compile time instead of runtime.
#include "TwsApiDefs.h"
using namespace TwsApi; // for TwsApiDefs.h

#include "Candle.h"

//=======================================================================
// This is a buffer to contain candlestick data and send to app when full
//=======================================================================

class CandleStickBuffer {
public:
    CandleStickBuffer(size_t capacity);

    void processBuffer(std::vector<std::unique_ptr<CandleStick>>& wrapperContainer);

    size_t checkBufferCapacity();
    size_t getCurrentBufferLoad();
    void setNewBufferCapacity(int value);
    void addToBuffer(std::unique_ptr<CandleStick> candle);
    bool checkSet(int value);
    void addToSet(int value);

private:
    // bufferReqs will ensure we have all reqIds from the request list before emptying the buffer
    std::unordered_set<int> bufferReqs;
    std::vector<std::unique_ptr<CandleStick>> buffer;
    size_t capacity_;
};

class MockWrapper {
public:
    MockWrapper();  

    long time_ = 0;

    // Candle to be used repeatedly for realtime bars
    //CandleStick underlyingRTBs;
    
    int getReq();
    bool notDone(void);
    double getSPXPrice(void);
    std::vector<std::unique_ptr<CandleStick>> getHistoricCandles();
    std::vector<std::unique_ptr<CandleStick>> getFiveSecCandles();
    
    void setHistoricalDataVisibility(bool x);
    void setRealTimeDataVisibility(bool x);
    void setmDone(bool x);

    virtual void currentTime(long time);

    virtual void historicalData(TickerId reqId, const IBString& date
        , double open, double high, double low, double close
        , int volume, int barCount, double WAP, int hasGaps);

    virtual void realtimeBar(TickerId reqId, long time, double open, double high,
        double low, double close, long volume, double wap, int count);

private:
    CandleStickBuffer candleBuffer;

    int Req = 0;
    double SPXPrice = 0;

    std::vector<std::unique_ptr<CandleStick>> historicCandles;
    std::vector<std::unique_ptr<CandleStick>> fiveSecCandles;

    // Variables to show data request output
    bool showHistoricalData = false;
    bool showRealTimeData = false;

    bool m_done;
};

