#define _CRT_SECURE_NO_WARNINGS

//=====================================================================
// Candle for stock data that will be used throughout the program
//=====================================================================

#pragma once

#include <iostream>

#include "TwsApiL0.h"
#include "TwsApiDefs.h"
using namespace TwsApi; // for TwsApiDefs.h

#include "Enums.h"

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

    Candle(const Candle& c); // Copy constructor

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

// This will contain all tags available to be obtained upon the creation of a candle
// This allows for the candle itself to remain lightweight while this data is sent to the db
// or sent via callback to the alert handler
class CandleTags {
public:
    CandleTags(std::shared_ptr<Candle> c, TimeFrame tf, Alerts::OptionType optType, Alerts::TimeOfDay tod,
        Alerts::VolumeStDev volStDev, Alerts::VolumeThreshold volThresh, Alerts::PriceDelta optPriceDelta,
        Alerts::DailyHighsAndLows optDHL, Alerts::LocalHighsAndLows optLHL);

    // Constructor if receiving db data
    CandleTags(std::shared_ptr<Candle> c, std::vector<int> tags);

    void setSqlId(int val);
    // Mutator to add underlying tags
    void addUnderlyingTags(Alerts::RelativeToMoney rtm, Alerts::PriceDelta pd, Alerts::DailyHighsAndLows DHL, Alerts::LocalHighsAndLows LHL);

    // Make the candle a public member 
    Candle candle;

    int getSqlId() const;
    TimeFrame getTimeFrame() const;
    Alerts::OptionType getOptType() const;
    Alerts::TimeOfDay getTOD() const;
    Alerts::RelativeToMoney getRTM() const;
    Alerts::VolumeStDev getVolStDev() const;
    Alerts::VolumeThreshold getVolThresh() const;
    Alerts::PriceDelta getOptPriceDelta() const;
    Alerts::DailyHighsAndLows getDHL() const;
    Alerts::LocalHighsAndLows getLHL() const;
    Alerts::PriceDelta getUnderlyingPriceDelta() const;
    Alerts::DailyHighsAndLows getUnderlyingDHL() const;
    Alerts::LocalHighsAndLows getUnderlyingLHL() const;

private:
    std::vector<int> tags_{};
    int sqlId{ 0 };

    TimeFrame tf_{ TimeFrame::FiveSecs };
    Alerts::OptionType optType_{ Alerts::OptionType::Call };
    Alerts::TimeOfDay tod_{ Alerts::TimeOfDay::Hour1 };

    Alerts::VolumeStDev volStDev_{ Alerts::VolumeStDev::LowVol };
    Alerts::VolumeThreshold volThresh_{ Alerts::VolumeThreshold::LowVol };
    Alerts::PriceDelta optPriceDelta_{ Alerts::PriceDelta::Under1 };
    Alerts::DailyHighsAndLows optDHL_{ Alerts::DailyHighsAndLows::Inside };
    Alerts::LocalHighsAndLows optLHL_{ Alerts::LocalHighsAndLows::Inside };

    // Tags that rely on underlying price data that will be updated after construction
    Alerts::RelativeToMoney rtm_{ Alerts::RelativeToMoney::ATM };
    Alerts::PriceDelta underlyingPriceDelta_{ Alerts::PriceDelta::Under1 };
    Alerts::DailyHighsAndLows underlyingDHL_{ Alerts::DailyHighsAndLows::Inside };
    Alerts::LocalHighsAndLows underlyingLHL_{ Alerts::LocalHighsAndLows::Inside };
};