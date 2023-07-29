#include "Candle.h"

#include <chrono>
#include <iomanip>
#include <sstream>

// Constructor for historical data
CandleStick::CandleStick(TickerId reqId, const IBString& date, double open, double high, double low, double close, 
    int volume, int barCount, double WAP, int hasGaps)
    : reqId_(reqId), date_(date), time_(0), open_(open), close_(close), high_(high), 
    low_(low), volume_(volume), barCount_(barCount), WAP_(WAP), hasGaps_(hasGaps), count_(0)
{
    convertDateToUnix();
}

// Constructor for 5 Second data
CandleStick::CandleStick(TickerId reqId, long time, double open, double high, double low,
    double close, long volume, double wap, int count)
    : reqId_(reqId), date_(""), time_(time), open_(open), close_(close), high_(high), 
    low_(low), volume_(volume), barCount_(0), WAP_(wap), hasGaps_(0), count_(count)
{
}

// Constructor for other candles created from 5 sec
CandleStick::CandleStick(TickerId reqId, long time, double open, double high,
    double low, double close, long volume)
    : reqId_(reqId), date_(""), time_(time), open_(open), close_(close), high_(high), 
    low_(low), volume_(volume), barCount_(0), WAP_(0.0), hasGaps_(0), count_(0)
{
}

// Getters
TickerId CandleStick::getReqId() const { return reqId_; }
IBString CandleStick::getDate() const { return date_; }
long CandleStick::getTime() const { return time_; }
double CandleStick::getOpen() const { return open_; }
double CandleStick::getClose() const { return close_; }
double CandleStick::getHigh() const { return high_; }
double CandleStick::getLow() const { return low_; }
long CandleStick::getVolume() const { return volume_; }
int CandleStick::getBarCount() const { return barCount_; }
double CandleStick::getWAP() const { return WAP_; }
int CandleStick::checkGaps() const { return hasGaps_; }
int CandleStick::getCount() const { return count_; }

void CandleStick::convertDateToUnix() {
    // Convert time string to unix values
    std::tm tmStruct = {};
    std::istringstream ss(date_);
    ss >> std::get_time(&tmStruct, "%Y%m%d %H:%M:%S");

    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tmStruct));
    auto unixTime = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();

    time_ = static_cast<long>(unixTime);
}

void CandleStick::convertUnixToDate() {
    struct tm* timeInfo;
    char buffer[80];

    time_t time = static_cast<time_t>(time_);

    timeInfo = localtime(&time);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);

    date_ = buffer;
}

