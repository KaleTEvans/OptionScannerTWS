/*====================================
* EWrapper Class
* This will receive callbacks from the TWS Api and translate to usable data
* Can overwrite virtual functions to define your own output
* The TwsApiC++ library uses the same functions and object names as the actual TWS Api
*=====================================
*/

#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>

///Easier: Just one include statement for all functionality
#include "TwsApiL0.h"

///Faster: Check spelling of parameter at compile time instead of runtime.
#include "TwsApiDefs.h"
using namespace TwsApi; // for TwsApiDefs.h

// **** Use this definition for candlesticks
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
    
    TickerId reqId;
    IBString date = "";
    long time = 0;
    double open;
    double close;
    double high;
    double low;
    int volume;
    int barCount = 0;
    double WAP = 0;
    int hasGaps = 0;
    int count = 0;
};

//=======================================================================
// This is a buffer to contain candlestick data and send to app when full
//=======================================================================
template <typename T> 
class CandleBuffer {
public:
    CandleBuffer(size_t capacity) : capacity(capacity) {}

    void processBuffer(std::vector<T>& wrapperContainer) {
        if (buffer.size() == capacity) {
            std::cout << "Processing buffer with " << buffer.size() << " candles" << std::endl;
            // Append buffer contents to the target vector
            wrapperContainer.clear();
            wrapperContainer.insert(wrapperContainer.end(), buffer.begin(), buffer.end());
            // Clear the buffer
            buffer.clear();
        }
    }

public:
    std::vector<T> buffer;
    size_t capacity;
};

// We can define our own eWrapper to implement only the functionality that we need to use
class tWrapper : public EWrapperL0 {

public:
    bool m_Done, m_ErrorForRequest;
    bool notDone(void) { return !(m_Done || m_ErrorForRequest); }

    // Req will be used to track the request to the client, and used to return the correct information once received
    int Req = 0;
    

    //========================================
    // Containers to be used for received data
    //========================================
    // 22 Candles for each option strikle
    CandleBuffer<Candle> candleBuffer{ 22 };

    std::vector<Candle> underlyingCandles;
    std::vector<Candle> fiveSecCandles;

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
        fprintf(stderr, "Error for id=%d: %d = %s\n"
            , id, errorCode, (const char*)errorString);
        m_ErrorForRequest = (id > 0);    // id == -1 are 'system' messages, not for user requests
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
        , int volume, int barCount, double WAP, int hasGaps) {

        ///Easier: EWrapperL0 provides an extra method to check all data was retrieved
        if (IsEndOfHistoricalData(date)) {
            // m_Done = true;
            // Set Req to the same value as reqId so we can retrieve the data once finished
            Req = reqId;
            std::cout << "Completed historical data request " << Req << std::endl;
            return;
        }

        // Upon receiving the price request, populate candlestick data
        Candle c(reqId, date, open, high, low, close, volume, barCount, WAP, hasGaps);
        underlyingCandles.push_back(c);

      /*  fprintf(stdout, "%10s, %5.3f, %5.3f, %5.3f, %5.3f, %7d\n"
            , (const  char*)date, open, high, low, close, volume);*/
    }

    // Retrieve real time bars, TWS Api currently only returns 5 second candles
    virtual void realtimeBar(TickerId reqId, long time, double open, double high,
        double low, double close, long volume, double wap, int count) {

        // Get the current time, and check with market close (3pm cst) to determine when to end the connection
        std::time_t tmNow = std::time(NULL);
        struct tm t = *localtime(&tmNow);

        if (t.tm_hour == 15 && t.tm_min == 0) {
            std::cout << "Market closed, ending realTimeBar connection" << std::endl;
            m_Done = true;
            return;
        }

        // Upon receiving the price request, populate candlestick data
        Candle c(reqId, time, open, high, low, close, volume, wap, count);
        candleBuffer.buffer.push_back(c);
        candleBuffer.processBuffer(fiveSecCandles);

        std::cout << reqId << " " << time << " " << "high: " << high << " low: " << low << " volume: " << volume << std::endl;
    }

    virtual void cancelRealTimeBars(TickerId tickerId) {
        std::cout << "Cancelled real time bar data for " << tickerId << std::endl;
    }

};

