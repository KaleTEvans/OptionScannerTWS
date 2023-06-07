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
struct Candle {
    IBString date = "";
    long time = 0;
    double open;
    double close;
    double high;
    double low;
    int volume;
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
    std::vector<Candle> candlesticks;
    std::vector<Candle> fiveSecCandles;

    ///Easier: The EReader calls all methods automatically(optional)
    tWrapper(bool runEReader = true) : EWrapperL0(runEReader) {
        m_Done = false;
        m_ErrorForRequest = false;
    }

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

    virtual void connectionOpened() {
        std::cout << "Connected to TWS" << std::endl;
    }

    virtual void connectionClosed() {
        std::cout << "Connection has been closed" << std::endl;
    }

    ///Safer: uncatched exceptions are catched before they reach the IB library code.
    ///       The Id is tickerId, orderId, or reqId, or -1 when no id known
    virtual void OnCatch(const char* MethodName, const long Id) {
        fprintf(stderr, "*** Catch in EWrapper::%s( Id=%ld, ...) \n", MethodName, Id);
    }

    virtual void currentTime(long time) {
        std::cout << "Current Unix time received from TWS: " << time << std::endl;
    }

    ///Faster: Implement only the method for the task.
    /// => TwsApiC++  provides an empty implementation for each EWrapper method.
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
        Candle c;
        c.date = date;
        c.open = open;
        c.close = close;
        c.high = high;
        c.low = low;
        c.volume = volume;
        candlesticks.push_back(c);

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
        Candle c;
        c.time = time;
        c.open = open;
        c.close = close;
        c.high = high;
        c.low = low;
        c.volume = volume;
        fiveSecCandles.push_back(c);

        std::cout << reqId << " " << time << " " << "high: " << high << " low: " << low << " volume: " << volume << std::endl;
    }

    virtual void cancelRealTimeBars(TickerId tickerId) {
        std::cout << "Cancelled real time bar data for " << tickerId << std::endl;
    }

};

