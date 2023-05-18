#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <ctime>

#include "tWrapper.h"
#include "App.h"


//========================================================================
// main entry
//========================================================================
int main(void) {
    ///Easier: just allocate your wrapper and instantiate the EClientL0 with it.
    const char* host = "127.0.0.1";
    IBString ticker = "SPX";

    OptionScanner* opt = new OptionScanner(host, ticker);

    opt->populateStrikes();
    opt->retrieveOptionData();

    Contract SPXchain;
    SPXchain.symbol = "SPX";
    SPXchain.secType = *SecType::OPT;
    SPXchain.currency = "USD";
    SPXchain.exchange = *Exchange::IB_SMART;
    SPXchain.primaryExchange = *Exchange::CBOE;
    SPXchain.right = *ContractRight::CALL;
    SPXchain.expiry = EndDateTime(2023, 05, 17);
    SPXchain.strike = 4160;

   /* opt->EC->reqHistoricalData
    (4150
        , SPXchain
        , EndDateTime(2023, 05, 18)
        , DurationStr(1, *DurationHorizon::Days)
        , *BarSizeSetting::_1_min
        , *WhatToShow::TRADES
        , UseRTH::OnlyRegularTradingData
        , FormatDate::AsDate
    );*/

    //Easier: Call checkMessages() in a loop. No need to wait between two calls.
    while (opt->YW.notDone()) {
        opt->EC->checkMessages();

        if (opt->YW.Req == 4150) break;
        
        // Set m_Done to true when you no longer wish to receive messages from the wrapper
        //======================
        // opt.YW->m_Done = true 
        //======================
    }

    delete opt;

    return 0;
}