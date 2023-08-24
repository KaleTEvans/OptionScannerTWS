
#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <condition_variable>

///Easier: Just one include statement for all functionality
#include "TwsApiL0.h"

///Faster: Check spelling of parameter at compile time instead of runtime.
#include "TwsApiDefs.h"
using namespace TwsApi; // for TwsApiDefs.h

#include "Candle.h"
//#include "tWrapper.h"



//=======================================================================
// This is a buffer to contain candlestick data and send to app when full
//=======================================================================

class MockCandleBuffer {
public:
    MockCandleBuffer(int capacity);

    std::vector<std::unique_ptr<Candle>> processBuffer();

    bool checkBufferFull(void);
    void setNewBufferCapacity(int value);
    void updateBuffer(std::unique_ptr<Candle> candle);
    int getCapacity(void);

    int wrapperActiveReqs = 0; // Will ensure buffer capacity is the same as all wrapper open requests

private:
    void checkBufferStatus(void);

    // bufferMap will ensure we have all reqIds from the request list before emptying the buffer
    std::unordered_map<int, std::unique_ptr<Candle>> bufferMap;
    int capacity_;
    std::chrono::time_point<std::chrono::steady_clock> bufferTimePassed_;

    bool wasDataProcessed_ = false; // Periodically check to ensure buffer is processing and not in an unfilled state

    std::mutex bufferMutex;
};

class MockWrapper {
public:
    MockWrapper();

    // Candle to be used repeatedly for realtime bars
    bool notDone();
    long getCurrentTime();
    double getSPXPrice(void);
    int getBufferCapacity(void);
    int checkActiveReqs();
    std::unordered_set<int> getActiveReqs();
    std::vector<std::unique_ptr<Candle>> getHistoricCandles();
    std::vector<std::unique_ptr<Candle>> getProcessedFiveSecCandles();

    bool checkMockBufferFull();
    std::mutex& getWrapperMutex();
    std::condition_variable& getWrapperConditional();

    void showHistoricalDataOutput();
    void hideHistoricalDataOutput();
    void showRealTimeDataOutput();
    void hideRealTimeDataOutput();
    void setmDone(bool x);
    void setMockUnderlying(double x);
    void setBufferCapacity(const int x);

    virtual void currentTime(long time);

    virtual void historicalData(TickerId reqId, const IBString& date
        , double open, double high, double low, double close
        , int volume, int barCount, double WAP, int hasGaps);

    virtual void realtimeBar(TickerId reqId, long time, double open, double high,
        double low, double close, long volume, double wap, int count);

    // Variables for other tests
    int candleBenchmarkSwitch = 1; // 1 - smtptr vector, 2 - smtptr queue, 3 - copied vector, 4 - copied queue

    std::vector<Candle> getCopiedCandleVector();
    std::queue<Candle> getCopiedCandleQueue();
    std::queue<std::unique_ptr<Candle>> getMovedCandleQueue();
    void clearTestContainers(void);

private:
    MockCandleBuffer mcb;

    // Mutex and conditional for buffer use
    std::mutex wrapperMtx;
    std::condition_variable cv;

    double SPXPrice = 4581;

    std::vector<std::unique_ptr<Candle>> historicCandles;
    std::vector<std::unique_ptr<Candle>> fiveSecCandles;

    std::unordered_set<int> activeReqs;

    // Variables to show data request output
    bool showHistoricalData = false;
    bool showRealTimeData = false;

    bool m_done;
    long time_ = 0;

    // Variables for other tests
    std::vector<Candle> copiedCandleVector;
    std::queue<Candle> copiedCandleQueue;
    std::queue<std::unique_ptr<Candle>> movedCandleQueue;
};
