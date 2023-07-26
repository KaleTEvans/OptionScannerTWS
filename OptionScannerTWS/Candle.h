#define _CRT_SECURE_NO_WARNINGS

//=====================================================================
// Candlestick for stock data that will be used throughout the program
//=====================================================================

#pragma once

#include <unordered_set>

#include "TwsApiL0.h"
#include "TwsApiDefs.h"
using namespace TwsApi; // for TwsApiDefs.h

class CandleStick {
public:
    // Constructor for historical data
    CandleStick(TickerId reqId, const IBString& date, double open, double high, double low, double close
        , int volume, int barCount, double WAP, int hasGaps);

    // Constructor for 5 Second data
    CandleStick(TickerId reqId, long time, double open, double high, double low, double close, long volume, double wap, int count);

    // Constructor for other candles created from 5 sec
    CandleStick(TickerId reqId, long time, double open, double high, double low, double close, long volume);

    TickerId getReqId() const;
    IBString getDate() const;
    long getTime() const;
    double getOpen() const;
    double getClose() const;
    double getHigh() const;
    double getLow() const;
    long getVolume() const;
    int getBarCount() const;
    double getWAP() const;
    int checkGaps() const;
    int getCount() const;

    void convertDateToUnix();
    void convertUnixToDate();
    
private:
    TickerId reqId_;
    IBString date_;
    long time_;
    double open_;
    double close_;
    double high_;
    double low_;
    long volume_;
    int barCount_;
    double WAP_;
    int hasGaps_;
    int count_;
};

//=======================================================================
// This is a buffer to contain candlestick data and send to app when full
//=======================================================================

class CandleStickBuffer {
public:
    CandleStickBuffer(size_t capacity);

    void processBuffer(std::vector<CandleStick>& wrapperContainer);

    size_t checkBufferCapacity();
    size_t getCurrentBufferLoad();
    void setNewBufferCapacity(int value);
    void addToBuffer(CandleStick candle);
    bool checkSet(int value);
    void addToSet(int value);

private:
    // bufferReqs will ensure we have all reqIds from the request list before emptying the buffer
    std::unordered_set<int> bufferReqs;
    std::vector<CandleStick> buffer;
    size_t capacity_;
};