#define _CRT_SECURE_NO_WARNINGS

//=====================================================================
// Candle for stock data that will be used throughout the program
//=====================================================================

#pragma once

#include <iostream>

#include "TwsApiL0.h"
#include "TwsApiDefs.h"
using namespace TwsApi; // for TwsApiDefs.h

class Candle {
public:
    // Constructor for historical data
    Candle(TickerId reqId, const IBString& date, double open, double high, double low, double close
        , int volume, int barCount, double WAP, int hasGaps);

    // Constructor for 5 Second data
    Candle(TickerId reqId, long time, double open, double high, double low, double close, long volume, double wap, int count);

    // Constructor for other candles created from 5 sec
    Candle(TickerId reqId, long time, double open, double high, double low, double close, long volume);

    Candle();

    TickerId reqId() const;
    IBString date() const;
    long time() const;
    double open() const;
    double close() const;
    double high() const;
    double low() const;
    long volume() const;
    int barCount() const;
    double WAP() const;
    int hasGaps() const;
    int count() const;

    void convertDateToUnix();
    void convertUnixToDate() const; // Lazy conversion only upon request
    
private:
    TickerId reqId_;
    mutable IBString date_;
    mutable bool dateConverted_{ false }; // Marked false if a unix time is received in the constructor
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