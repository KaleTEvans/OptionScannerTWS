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

#ifndef TEST_CONFIG
#include "../Logger.h"
#endif // !TEST_CONFIG

#include "Candle.h"
#include "TwsApiL0.h"
#include "TwsApiDefs.h"
using namespace TwsApi; // for TwsApiDefs.h

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

    int wrapperActiveReqs; // Will ensure buffer capacity is the same as all wrapper open requests

private:
    void checkBufferStatus();

    // bufferMap will ensure we have all reqIds from the request list before emptying the buffer
    std::unordered_map<int, std::unique_ptr<Candle>> bufferMap;
    int capacity_;
    std::chrono::time_point<std::chrono::steady_clock> bufferTimePassed_;

    bool wasDataProcessed_{ false }; // Periodically check to ensure buffer is processing and not in an unfilled state

    std::mutex bufferMutex;
};

// We can define our own eWrapper to implement only the functionality that we need to use
class tWrapper : public EWrapperL0 {

public:
    ///Easier: The EReader calls all methods automatically(optional)
    tWrapper(int initBufferSize, bool runEReader = true);

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

    /////// Tick Options /////////
    virtual void tickPrice(TickerId tickerId, TickType field, double price, int canAutoExecute);
    virtual void tickGeneric(TickerId tickerId, TickType tickType, double value);
    virtual void tickSize(TickerId tickerId, TickType field, int size);
    virtual void marketDataType(TickerId reqId, int marketDataType);
    virtual void tickString(TickerId tickerId, TickType tickType, const IBString& value);
    virtual void tickSnapshotEnd(int reqId);

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
    double lastTickPrice();
    int bufferCapacity();
    bool checkBufferFull();
    std::vector<std::unique_ptr<Candle>> historicCandles();
    std::vector<std::unique_ptr<Candle>> processedFiveSecCandles();

    std::mutex& wrapperMutex();
    std::condition_variable& wrapperConditional();

private:
    CandleBuffer candleBuffer_;

    // Mutex and conditional for buffer use
    std::mutex wrapperMtx_;
    std::condition_variable cv_;

    //========================================
    // Containers to be used for received data
    //========================================
   
    // Vectors of unique pointers will automatically be cleared when data is passed
    std::vector<std::unique_ptr<Candle>> historicCandles_;
    std::vector<std::unique_ptr<Candle>> fiveSecCandles_;

    std::unordered_set<int> activeReqs_;

    // Variables to show data request output
    bool showHistoricalData_{ false };
    bool showRealTimeData_{ false };

    long time_{ 0 };
    // Last tick price for requested contract
    double tickPriceLast_{ 0 };

    // Req will be used to track the request to the client, and used to return the correct information once received
    int Req_{ 0 };
};


