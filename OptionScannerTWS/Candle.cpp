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
}

// Constructor for 5 Second data
Candle::Candle(TickerId reqId, long time, double open, double high, double low,
    double close, long volume, double wap, int count)
    : reqId_(reqId), date_(""), time_(time), open_(open), close_(close), high_(high), 
    low_(low), volume_(volume), barCount_(0), WAP_(wap), hasGaps_(0), count_(count)
{
    convertUnixToDate();
}

// Constructor for other candles created from 5 sec
Candle::Candle(TickerId reqId, long time, double open, double high,
    double low, double close, long volume)
    : reqId_(reqId), date_(""), time_(time), open_(open), close_(close), high_(high), 
    low_(low), volume_(volume), barCount_(0), WAP_(0.0), hasGaps_(0), count_(0)
{
    convertUnixToDate();
}

// Getters
TickerId Candle::getReqId() const { return reqId_; }
IBString Candle::getDate() const { return date_; }
long Candle::getTime() const { return time_; }
double Candle::getOpen() const { return open_; }
double Candle::getClose() const { return close_; }
double Candle::getHigh() const { return high_; }
double Candle::getLow() const { return low_; }
long Candle::getVolume() const { return volume_; }
int Candle::getBarCount() const { return barCount_; }
double Candle::getWAP() const { return WAP_; }
int Candle::checkGaps() const { return hasGaps_; }
int Candle::getCount() const { return count_; }

void Candle::convertDateToUnix() {
    // Convert time string to unix values
    std::tm tmStruct = {};
    std::istringstream ss(date_);
    ss >> std::get_time(&tmStruct, "%Y%m%d %H:%M:%S");

    time_t stime = std::mktime(&tmStruct);
    time_ = static_cast<long>(stime);
}

void Candle::convertUnixToDate() {
    struct tm* timeInfo;
    char buffer[80];

    time_t time = static_cast<time_t>(time_);

    std::unique_lock<std::mutex> lock(candleMutex);
    timeInfo = localtime(&time);
    lock.unlock();

    strftime(buffer, sizeof(buffer), "%Y%m%d %H:%M:%S", timeInfo);
    IBString date = buffer;
    date_ = date;
}

