/*====================================
* EWrapper Class
* This will receive callbacks from the TWS Api and translate to usable data
* Can overwrite virtual functions to define your own output
* The TwsApiC++ library uses the same functions and object names as the actual TWS Api
*=====================================
*/
#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <unordered_set>

#include <odb/core.hxx> // ORM library for SQL

#include "Logger.h"

///Easier: Just one include statement for all functionality
#include "TwsApiL0.h"

///Faster: Check spelling of parameter at compile time instead of runtime.
#include "TwsApiDefs.h"
using namespace TwsApi; // for TwsApiDefs.h

//================================================
// Use this definition for candlesticks
//================================================

class Candle {
public:
    // Constructor for historical data
    Candle(TickerId reqId, const IBString& date
        , double open, double high, double low, double close
        , int volume, int barCount, double WAP, int hasGaps) :
        reqId(reqId), date(date), open(open), high(high), low(low),
        close(close), volume(volume), barCount(barCount), WAP(WAP), hasGaps(hasGaps) {}

    // Constructor for 5 Second data
    Candle(TickerId reqId, long time, double open, double high,
        double low, double close, long volume, double wap, int count) :
        reqId(reqId), time(time), open(open), high(high), low(low),
        close(close), volume(volume), WAP(wap), count(count) {}

    // Constructor for other candles created from 5 sec
    Candle(TickerId reqId, long time, double open, double high,
        double low, double close, long volume) :
        reqId(reqId), time(time), open(open), high(high), low(low),
        close(close), volume(volume) {}

    // Default Constructor
    Candle() {
        reqId = 0;
        open = 0;
        close = 0;
        high = 0;
        low = 0;
        volume = 0;
    }
    
    TickerId reqId;
    IBString date = "";
    long time = 0;
    double open;
    double close;
    double high;
    double low;
    long volume;
    int barCount = 0;
    double WAP = 0;
    int hasGaps = 0;
    int count = 0;
};

//=======================================================================
// This is a buffer to contain candlestick data and send to app when full
//=======================================================================

class CandleBuffer {
public:
    CandleBuffer(size_t capacity);

    void processBuffer(std::vector<Candle>& wrapperContainer);

    size_t checkBufferCapacity();
    size_t getCurrentBufferLoad();
    void setNewBufferCapacity(int value);
    void addToBuffer(Candle candle);
    bool checkSet(int value);
    void addToSet(int value);

private:
    // bufferReqs will ensure we have all reqIds from the request list before emptying the buffer
    std::unordered_set<int> bufferReqs;
    std::vector<Candle> buffer;
    size_t capacity_;
};

// We can define our own eWrapper to implement only the functionality that we need to use
class tWrapper : public EWrapperL0 {

public:
    bool m_Done, m_ErrorForRequest;
    bool notDone(void) { return !(m_Done || m_ErrorForRequest); }

    // Req will be used to track the request to the client, and used to return the correct information once received
    int Req = 0;

    // Variables to show data request output
    bool showHistoricalData = false;
    bool showRealTimeData = false;

    //========================================
    // Containers to be used for received data
    //========================================
    // 18 Candles for each option strikle
    CandleBuffer candleBuffer{ 18 };

    std::vector<Candle> underlyingCandles;
    std::vector<Candle> fiveSecCandles;

    // Candle to be used repeatedly for realtime bars
    Candle underlyingRTBs;
    // Boolean if only needing the single realtime bars
    bool singleRTBs = false;

    ///Easier: The EReader calls all methods automatically(optional)
    tWrapper(bool runEReader = true) : EWrapperL0(runEReader) {
        m_Done = false;
        m_ErrorForRequest = false;
    }

    //========================================
    // Error Handling
    //========================================

    ///Methods winError & error print the errors reported by IB TWS
    virtual void winError(const IBString& str, int lastError) {
        fprintf(stderr, "WinError: %d = %s\n", lastError, (const char*)str);
        m_ErrorForRequest = true;
    }

    virtual void error(const int id, const int errorCode, const IBString errorString) {
        if (errorCode != 2176) { // 2176 is a weird api error that claims to not allow use of fractional shares
            fprintf(stderr, "Error for id=%d: %d = %s\n"
                , id, errorCode, (const char*)errorString);
            m_ErrorForRequest = (id > 0);    // id == -1 are 'system' messages, not for user requests
        }
    }

    ///Safer: uncatched exceptions are catched before they reach the IB library code.
    ///       The Id is tickerId, orderId, or reqId, or -1 when no id known
    virtual void OnCatch(const char* MethodName, const long Id) {
        fprintf(stderr, "*** Catch in EWrapper::%s( Id=%ld, ...) \n", MethodName, Id);
    }

    //=======================================
    // Connectivity
    //=======================================

    virtual void connectionOpened() {
        std::cout << "Connected to TWS" << std::endl;
    }

    virtual void connectionClosed() {
        std::cout << "Connection has been closed" << std::endl;
    }

    // Upon initial API connection, recieves a comma-separated string with the managed account IDs
    virtual void managedAccounts(const IBString& accountsList) {
        std::cout << accountsList << std::endl;
    }

    virtual void currentTime(long time) {
        std::cout << "Current Unix time received from TWS: " << time << std::endl;
    }

    //========================================
    // Data Retrieval
    //========================================

    virtual void historicalData(TickerId reqId, const IBString& date
        , double open, double high, double low, double close
        , int volume, int barCount, double WAP, int hasGaps);

    // Retrieve real time bars, TWS Api currently only returns 5 second candles
    virtual void realtimeBar(TickerId reqId, long time, double open, double high,
        double low, double close, long volume, double wap, int count) {

        // Upon receiving the price request, populate candlestick data
        Candle c(reqId, time, open, high, low, close, volume, wap, count);

        // ReqId 1234 will be used for the underlying contract
        if (reqId == 1234 || singleRTBs) {
            underlyingRTBs.reqId = reqId;
            underlyingRTBs.time = time;
            underlyingRTBs.open = open;
            underlyingRTBs.high = high;
            underlyingRTBs.low = low;
            underlyingRTBs.close = close;
            underlyingRTBs.volume = volume;
        }
        // Otherwise use the buffer for the option contracts
        else {
            if (!candleBuffer.checkSet(c.reqId)) {
                candleBuffer.addToBuffer(c);

                candleBuffer.addToSet(c.reqId);
            }
            candleBuffer.processBuffer(fiveSecCandles);
        }


        if (showRealTimeData) {
            std::cout << reqId << " " << time << " " << "high: " << high << " low: " << low << " volume: " << volume << std::endl;
        }
    }

    virtual void cancelRealTimeBars(TickerId tickerId) {
        std::cout << "Cancelled real time bar data for " << tickerId << std::endl;
    }

};

