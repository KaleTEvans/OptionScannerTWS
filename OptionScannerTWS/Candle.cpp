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

