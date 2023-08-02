
#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <memory>
#include <unordered_set>
#include <condition_variable>

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

    std::vector<std::unique_ptr<CandleStick>> processBuffer();

    bool checkBufferFull(void);
    void setNewBufferCapacity(int value);
    void addToBuffer(std::unique_ptr<CandleStick> candle);
    bool checkSet(int value);
    void addToSet(int value);

private:
    // bufferReqs will ensure we have all reqIds from the request list before emptying the buffer
    std::unordered_set<int> bufferReqs;
    std::vector<std::unique_ptr<CandleStick>> buffer;
    size_t capacity_;

    std::mutex bufferMutex;
};

class MockWrapper {
public:
    MockWrapper();  

    // Candle to be used repeatedly for realtime bars
    bool notDone(void);
    long getCurrentTime(void);
    double getSPXPrice(void);
    std::vector<std::unique_ptr<CandleStick>> getHistoricCandles(void);
    std::vector<std::unique_ptr<CandleStick>> getProcessedFiveSecCandles(void);

    bool checkMockBufferFull(void);
    //void waitForFullMockBuffer(void);
    std::mutex& getWrapperMutex(void);
    std::condition_variable& getWrapperConditional(void);
    
    void showHistoricalDataOutput(void);
    void hideHistoricalDataOutput(void);
    void showRealTimeDataOutput(void);
    void hideRealTimeDataOutput(void);
    void setmDone(bool x);
    void setMockUnderlying(double x);
    void setBufferCapacity(int x);

    virtual void currentTime(long time);

    virtual void historicalData(TickerId reqId, const IBString& date
        , double open, double high, double low, double close
        , int volume, int barCount, double WAP, int hasGaps);

    virtual void realtimeBar(TickerId reqId, long time, double open, double high,
        double low, double close, long volume, double wap, int count);

private:
    CandleStickBuffer candleBuffer;

    // Mutex and conditional for buffer use
    std::mutex wrapperMtx;
    std::condition_variable cv;

    double SPXPrice = 4581;

    std::vector<std::unique_ptr<CandleStick>> historicCandles;
    std::vector<std::unique_ptr<CandleStick>> fiveSecCandles;

    // Variables to show data request output
    bool showHistoricalData = false;
    bool showRealTimeData = false;

    bool m_done;
    long time_ = 0;
};

