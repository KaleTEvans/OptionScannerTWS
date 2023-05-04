#define _CRT_SECURE_NO_WARNINGS

#include <iostream>

#include "tWrapper.h"


//========================================================================
// main entry
//========================================================================
int main(void) {
    ///Easier: just allocate your wrapper and instantiate the EClientL0 with it.
    tWrapper  YW(false);                      // false: not using the EReader
    EClientL0* EC = EClientL0::New(&YW);

    ///Easier: All structures defined by IB are immediately available.
    ///Faster: Use of TwsApiDefs.h codes check typos at compile time
    Contract          C;
    C.symbol = "MSFT";
    C.secType = *SecType::STK;              // instead of "STK"
    C.currency = "USD";
    C.exchange = *Exchange::IB_SMART;        // instead of "SMART"
    //C.primaryExchange = *Exchange::AMEX;            // instead of "AMEX"

    ///Easier: Use of TwsApiDefs.h makes code self explanatory,
    ///        i.e. UseRTH::OnlyRegularTradingData instead of true or 1.
    if (EC->eConnect("127.0.0.1", 7496, 0)) {
        std::cout << "Connected to TWS Server" << std::endl;
        EC->reqHistoricalData
        (20
            , C
            , EndDateTime(2023, 05, 05)                   // 20130220 00:00:00
            , DurationStr(1, *DurationHorizon::Days)  // instead of "1 M"
            , *BarSizeSetting::_10_mins                  // instead of "1 day"
            , *WhatToShow::TRADES                       // instead of "TRADES"
            , UseRTH::OnlyRegularTradingData            // instead of 1
            , FormatDate::AsDate                        // instead of 1
        );

        ///Easier: Call checkMessages() in a loop. No need to wait between two calls.
        while (YW.notDone()) {
            EC->checkMessages();
        }

    }

    EC->eDisconnect();
    delete EC;

    return 0;
}