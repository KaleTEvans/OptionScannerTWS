#include "Candle.h"

#include <chrono>
#include <iomanip>
#include <sstream>

// Constructor for historical data
Candle::Candle(TickerId reqId, const IBString& date, double open, double high, double low, double close, 
    int volume, int barCount, double WAP, int hasGaps)
    : reqId_(reqId), date_(date), time_(0), open_(open), close_(close), high_(high), 
    low_(low), volume_(volume), barCount_(barCount), WAP_(WAP), hasGaps_(hasGaps), count_(0)
{
    convertDateToUnix();
    dateConverted_ = true;
}

// Constructor for 5 Second data
Candle::Candle(TickerId reqId, long time, double open, double high, double low,
    double close, long volume, double wap, int count)
    : reqId_(reqId), date_(""), time_(time), open_(open), close_(close), high_(high), 
    low_(low), volume_(volume), barCount_(0), WAP_(wap), hasGaps_(0), count_(count) {}

// Constructor for other candles created from 5 sec
Candle::Candle(TickerId reqId, long time, double open, double high,
    double low, double close, long volume)
    : reqId_(reqId), date_(""), time_(time), open_(open), close_(close), high_(high), 
    low_(low), volume_(volume), barCount_(0), WAP_(0.0), hasGaps_(0), count_(0) {}

Candle::Candle() : reqId_(0), date_(""), time_(0), open_(0), close_(0), high_(0),
    low_(0), volume_(0), barCount_(0), WAP_(0.0), hasGaps_(0), count_(0) {}

Candle::Candle(const Candle& c) : reqId_(c.reqId_), date_(c.date_), dateConverted_(c.dateConverted_),
time_(c.time_), open_(c.open_), close_(c.close_), high_(c.high_), low_(c.low_), volume_(c.volume_),
barCount_(c.barCount_), WAP_(c.WAP_), hasGaps_(c.hasGaps_), count_(c.count_) {}

// Getters
TickerId Candle::reqId() const { return reqId_; }
long Candle::time() const { return time_; }
double Candle::open() const { return open_; }
double Candle::close() const { return close_; }
double Candle::high() const { return high_; }
double Candle::low() const { return low_; }
long Candle::volume() const { return volume_; }
int Candle::barCount() const { return barCount_; }
double Candle::WAP() const { return WAP_; }
int Candle::hasGaps() const { return hasGaps_; }
int Candle::count() const { return count_; }

IBString Candle::date() const {
    if (!dateConverted_) {
        convertUnixToDate();
        dateConverted_ = true;
    }
    return date_;
}

void Candle::convertDateToUnix() {
    // Convert time string to unix values
    std::tm tmStruct = {};
    std::istringstream ss(date_);
    ss >> std::get_time(&tmStruct, "%Y%m%d %H:%M:%S");

    time_t stime = std::mktime(&tmStruct);
    time_ = static_cast<long>(stime);
}

void Candle::convertUnixToDate() const {
    struct tm* timeInfo;
    char buffer[80];

    time_t time = static_cast<time_t>(time_);
    timeInfo = localtime(&time);

    strftime(buffer, sizeof(buffer), "%Y%m%d %H:%M:%S", timeInfo);
    IBString date = buffer;
    date_ = date;
}

CandleTags::CandleTags(std::shared_ptr<Candle> c, TimeFrame tf, Alerts::OptionType optType, Alerts::TimeOfDay tod,
    Alerts::VolumeStDev volStDev, Alerts::VolumeThreshold volThresh, Alerts::PriceDelta optPriceDelta,
    Alerts::DailyHighsAndLows optDHL, Alerts::LocalHighsAndLows optLHL) :
    candle(*c), tf_(tf), optType_(optType), tod_(tod), optPriceDelta_(optPriceDelta),
    volStDev_(volStDev), volThresh_(volThresh), optDHL_(optDHL), optLHL_(optLHL) {}

CandleTags::CandleTags(std::shared_ptr<Candle> c, std::vector<int> tags) : candle(*c), tags_(tags)
{
    std::string tf = Alerts::TagDBInterface::intToTag[tags[0]].first;
    std::string opttype = Alerts::TagDBInterface::intToTag[tags[1]].first;
    std::string tod = Alerts::TagDBInterface::intToTag[tags[2]].first;
    std::string rtm = Alerts::TagDBInterface::intToTag[tags[3]].first;
    std::string volstdev = Alerts::TagDBInterface::intToTag[tags[4]].first;
    std::string volthresh = Alerts::TagDBInterface::intToTag[tags[5]].first;
    std::string priceDelta = Alerts::TagDBInterface::intToTag[tags[6]].first;
    std::string opt_dhl = Alerts::TagDBInterface::intToTag[tags[7]].first;
    std::string opt_lhl = Alerts::TagDBInterface::intToTag[tags[8]].first;
    std::string underlyingPriceDelta = Alerts::TagDBInterface::intToTag[tags[9]].first;
    std::string u_dhl = Alerts::TagDBInterface::intToTag[tags[10]].first;
    std::string u_lhl = Alerts::TagDBInterface::intToTag[tags[11]].first;

    tf_ = str_to_tf(tf);
    optType_ = Alerts::EnumString::str_to_option_type(opttype);
    tod_ = Alerts::EnumString::str_to_tod(tod);
    rtm_ = Alerts::EnumString::str_to_rtm(rtm);
    volStDev_ = Alerts::EnumString::str_to_vol_stdev(volstdev);
    volThresh_ = Alerts::EnumString::str_to_vol_thresh(volthresh);
    optPriceDelta_ = Alerts::EnumString::str_to_price_delta(priceDelta);
    optDHL_ = Alerts::EnumString::str_to_daily_hl(opt_dhl);
    optLHL_ = Alerts::EnumString::str_to_local_hl(opt_lhl);
    underlyingPriceDelta_ = Alerts::EnumString::str_to_price_delta(underlyingPriceDelta);
    underlyingDHL_ = Alerts::EnumString::str_to_daily_hl(u_dhl);
    underlyingLHL_ = Alerts::EnumString::str_to_local_hl(u_lhl);
}

void CandleTags::setSqlId(int val) { sqlId = val; }

void CandleTags::addUnderlyingTags(Alerts::RelativeToMoney rtm, Alerts::PriceDelta pd, Alerts::DailyHighsAndLows DHL, Alerts::LocalHighsAndLows LHL) {
    rtm_ = rtm;
    underlyingPriceDelta_ = pd;
    underlyingDHL_ = DHL;
    underlyingLHL_ = LHL;
}

// Accessors
int CandleTags::getSqlId() const { return sqlId; }
TimeFrame CandleTags::getTimeFrame() const { return tf_; }
Alerts::OptionType CandleTags::getOptType() const { return optType_; }
Alerts::TimeOfDay CandleTags::getTOD() const { return tod_; }
Alerts::RelativeToMoney CandleTags::getRTM() const { return rtm_; }
Alerts::VolumeStDev CandleTags::getVolStDev() const { return volStDev_; }
Alerts::VolumeThreshold CandleTags::getVolThresh() const { return volThresh_; }
Alerts::PriceDelta CandleTags::getOptPriceDelta() const { return optPriceDelta_; }
Alerts::DailyHighsAndLows CandleTags::getDHL() const { return optDHL_; }
Alerts::LocalHighsAndLows CandleTags::getLHL() const { return optLHL_; }
Alerts::PriceDelta CandleTags::getUnderlyingPriceDelta() const { return underlyingPriceDelta_; }
Alerts::DailyHighsAndLows CandleTags::getUnderlyingDHL() const { return underlyingDHL_; }
Alerts::LocalHighsAndLows CandleTags::getUnderlyingLHL() const { return underlyingLHL_; }