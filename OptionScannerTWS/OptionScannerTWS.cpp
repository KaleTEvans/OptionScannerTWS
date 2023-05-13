#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <ctime>

#include "tWrapper.h"
#include "App.h"

void getSPXPrices(EClientL0 * client) {
    Contract con;
    con.symbol = "SPX";
    con.secType = *SecType::IND;
    con.currency = "USD";
    con.exchange = *Exchange::IB_SMART;
    con.primaryExchange = *Exchange::CBOE;

    client->reqHistoricalData
    // Will use reqId 101 for all close point price data
    (102
        , con
        , EndDateTime(2023, 05, 13)
        , DurationStr(1, *DurationHorizon::Days)
        , *BarSizeSetting::_1_min
        , *WhatToShow::TRADES
        , UseRTH::OnlyRegularTradingData
        , FormatDate::AsDate
    );

    
}


//========================================================================
// main entry
//========================================================================
int main(void) {
    ///Easier: just allocate your wrapper and instantiate the EClientL0 with it.
    // tWrapper  YW(false);                      // false: not using the EReader
    const char* host = "127.0.0.1";

    OptionScanner* opt = new OptionScanner(host);

    //opt->retreiveSPXPrice();

    //EClientL0* EC = EClientL0::New(&YW);

    /////Easier: All structures defined by IB are immediately available.
    /////Faster: Use of TwsApiDefs.h codes check typos at compile time
    //Contract          C;
    //C.symbol = "SPX";
    //C.secType = *SecType::OPT;              // instead of "STK"
    //C.currency = "USD";
    //C.exchange = *Exchange::IB_SMART;        // instead of "SMART"
    //C.primaryExchange = *Exchange::CBOE;            // instead of "AMEX"
    //C.right = *ContractRight::CALL;
    //C.expiry = EndDateTime(2023, 05, 11);
    //C.strike = 4125;

    /////Easier: Use of TwsApiDefs.h makes code self explanatory,
    /////        i.e. UseRTH::OnlyRegularTradingData instead of true or 1.
    //if (opt->EC->eConnect(opt->host, 7496, 0)) {
    //    std::cout << "Connected to TWS Server" << std::endl;
    //    //EC->reqHistoricalData
    //    //(101
    //    //    , C
    //    //    , EndDateTime(2023, 05, 12)                   // 20130220 00:00:00
    //    //    , DurationStr(1, *DurationHorizon::Days)  // instead of "1 M"
    //    //    , *BarSizeSetting::_1_min                 // instead of "1 day"
    //    //    , *WhatToShow::TRADES                       // instead of "TRADES"
    //    //    , UseRTH::OnlyRegularTradingData            // instead of 1
    //    //    , FormatDate::AsDate                        // instead of 1
    //    //);

        opt->retreiveSPXPrice();

    //    ///Easier: Call checkMessages() in a loop. No need to wait between two calls.
        while (opt->YW.notDone()) {
            opt->EC->checkMessages();

            if (opt->YW.Req == 101) {
                for (auto i : opt->YW.closePrices) std::cout << i << std::endl;
            }
        }

    //}

    // opt->EC->eDisconnect();
    //delete EC;
    delete opt;

    return 0;
}