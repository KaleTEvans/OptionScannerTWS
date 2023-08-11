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
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <condition_variable>

//#include "Logger.h"
#include "Candle.h"

///Easier: Just one include statement for all functionality
#include "TwsApiL0.h"

///Faster: Check spelling of parameter at compile time instead of runtime.
#include "TwsApiDefs.h"
using namespace TwsApi; // for TwsApiDefs.h

//================================================
// Use this definition for candlesticks
//================================================

//class Candle {
//public:
//    // Constructor for historical data
//    Candle(TickerId reqId, const IBString& date
//        , double open, double high, double low, double close
//        , int volume, int barCount, double WAP, int hasGaps) :
//        reqId(reqId), date(date), open(open), high(high), low(low),
//        close(close), volume(volume), barCount(barCount), WAP(WAP), hasGaps(hasGaps) {}
//
//    // Constructor for 5 Second data
//    Candle(TickerId reqId, long time, double open, double high,
//        double low, double close, long volume, double wap, int count) :
//        reqId(reqId), time(time), open(open), high(high), low(low),
//        close(close), volume(volume), WAP(wap), count(count) {}
//
//    // Constructor for other candles created from 5 sec
//    Candle(TickerId reqId, long time, double open, double high,
//        double low, double close, long volume) :
//        reqId(reqId), time(time), open(open), high(high), low(low),
//        close(close), volume(volume) {}
//
//    // Default Constructor
//    Candle() {
//        reqId = 0;
//        open = 0;
//        close = 0;
//        high = 0;
//        low = 0;
//        volume = 0;
//    }
//    
//    TickerId reqId;
//    IBString date = "";
//    long time = 0;
//    double open;
//    double close;
//    double high;
//    double low;
//    long volume;
//    int barCount = 0;
//    double WAP = 0;
//    int hasGaps = 0;
//    int count = 0;
//};

class CandleBuffer;

// We can define our own eWrapper to implement only the functionality that we need to use
class tWrapper : public EWrapperL0 {

public:
    ///Easier: The EReader calls all methods automatically(optional)
    tWrapper(bool runEReader = true);

    // Public variables to determine completion of certain wrapper requests
    bool m_Done, m_ErrorForRequest;
    bool notDone(void) { return !(m_Done || m_ErrorForRequest); }

    //========================================
    // Error Handling
    //========================================

    ///Methods winError & error print the errors reported by IB TWS
    virtual void winError(const IBString& str, int lastError);
    virtual void error(const int id, const int errorCode, const IBString errorString);

    ///Safer: uncatched exceptions are catched before they reach the IB library code.
    ///       The Id is tickerId, orderId, or reqId, or -1 when no id known
    virtual void OnCatch(const char* MethodName, const long Id);

    //=======================================
    // Connectivity
    //=======================================

    virtual void connectionOpened();
    virtual void connectionClosed();

    // Upon initial API connection, recieves a comma-separated string with the managed account IDs
    virtual void managedAccounts(const IBString& accountsList);

    //========================================
    // Data Retrieval
    //========================================

    virtual void currentTime(long time);

    virtual void historicalData(TickerId reqId, const IBString& date
        , double open, double high, double low, double close
        , int volume, int barCount, double WAP, int hasGaps);

    // Retrieve real time bars, TWS Api currently only returns 5 second candles
    virtual void realtimeBar(TickerId reqId, long time, double open, double high,
        double low, double close, long volume, double wap, int count);

    virtual void cancelRealTimeBars(TickerId tickerId) {
        std::cout << "Cancelled real time bar data for " << tickerId << std::endl;
    }

    //========================================
    // Mutators
    //========================================

    void showHistoricalDataOutput();
    void hideHistoricalDataOutput();
    void showRealTimeDataOutput();
    void hideRealTimeDataOutput();
    void setBufferCapacity(const int x);

    //=======================================
    // Accessors
    //=======================================

    int getReqId();
    long getCurrentTime();
    int getBufferCapacity();
    bool checBufferFull();
    std::vector<std::unique_ptr<Candle>> getHistoricCandles();
    std::vector<std::unique_ptr<Candle>> getProcessedFiveSecCandles();

    std::mutex& getWrapperMutex();
    std::condition_variable& getWrapperConditional();

private:
    CandleBuffer candleBuffer;

    // Mutex and conditional for buffer use
    std::mutex wrapperMtx;
    std::condition_variable cv;

    //========================================
    // Containers to be used for received data
    //========================================
   
    // Vectors of unique pointers will automatically be cleared when data is passed
    std::vector<std::unique_ptr<Candle>> historicCandles;
    std::vector<std::unique_ptr<Candle>> fiveSecCandles;

    std::unordered_set<int> activeReqs;

    // Variables to show data request output
    bool showHistoricalData = false;
    bool showRealTimeData = false;

    bool m_done;
    long time_ = 0;

    // Req will be used to track the request to the client, and used to return the correct information once received
    int Req = 0;
};

//=======================================================================
// This is a buffer to contain candlestick data and send to app when full
//=======================================================================

class CandleBuffer {
public:
    CandleBuffer(int capacity);

    std::vector<std::unique_ptr<Candle>> processBuffer();

    bool checkBufferFull(void);
    void setNewBufferCapacity(int value);
    void updateBuffer(std::unique_ptr<Candle> candle);
    int getCapacity(void);

    int wrapperActiveReqs = 0; // Will ensure buffer capacity is the same as all wrapper open requests

private:
    void checkBufferStatus();

    // bufferMap will ensure we have all reqIds from the request list before emptying the buffer
    std::unordered_map<int, std::unique_ptr<Candle>> bufferMap;
    int capacity_;
    std::chrono::time_point<std::chrono::steady_clock> bufferTimePassed_;

    bool wasDataProcessed_ = false; // Periodically check to ensure buffer is processing and not in an unfilled state

    std::mutex bufferMutex;
};
