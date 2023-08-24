#include "MockClient.h"
#include "TwsApiDefs.h"

#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>

MockClient::MockClient(MockWrapper& mockWrapper) : wrapper_(mockWrapper) {}

void MockClient::reqCurrentTime() {
    // Get the current system time in milliseconds since the epoch (Unix timestamp)
    auto currentTime = std::chrono::system_clock::now().time_since_epoch();

    // Convert milliseconds to seconds (Unix timestamp is in seconds)
    std::time_t unixTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
    long time = static_cast<long>(unixTime);

    wrapper_.currentTime(time);
}

void MockClient::reqHistoricalData(TickerId id, const Contract& contract,
    const IBString& endDateTime, const IBString& durationStr, const IBString& barSizeSetting,
    const IBString& whatToShow, int useRTH, int formatDate, bool keepUpToDate) {

    std::vector<std::string> dateTimes = generateDateTimeSeries(endDateTime, durationStr, barSizeSetting);

    bool isOption;
    (contract.secType == "OPT") ? isOption = true : isOption = false;

    // Current general spx price and vol
    double spxRefPrice = 4580.00;
    long spxRefVol = 50000000;

    for (const auto& i : dateTimes) {
        if (!isOption) {
            MiniCandle c = generateRandomCandle(spxRefPrice, spxRefVol, isOption);
            wrapper_.historicalData(id, i, c.open, c.high, c.low, c.close, c.volume, 0, 0, 0);
        }
        else {
            std::pair<double, long> optRefVals = calculateOptionRefValues(id, wrapper_.getSPXPrice());

            MiniCandle c = generateRandomCandle(optRefVals.first, optRefVals.second, isOption);
            wrapper_.historicalData(id, i, c.open, c.high, c.low, c.close, c.volume, 0, 0, 0);
        }
    }

    wrapper_.setmDone(true);
}

void MockClient::reqRealTimeBars(TickerId id, const Contract& contract, int barSize,
    const IBString& whatToShow, bool useRTH) {

    bool isOption;
    (contract.secType == "OPT") ? isOption = true : isOption = false;

    // Get current unix time
    reqCurrentTime();
    long unixTime = wrapper_.getCurrentTime();

    double refPrice = wrapper_.getSPXPrice();
    long refVol = 50000000;

    if (isOption) {
        std::pair<double, long> optVals = calculateOptionRefValues(id, wrapper_.getSPXPrice());
        refPrice = optVals.first;
        refVol = optVals.second;
    }

    // Start the thread to stream data
    threads_.emplace_back([&, id, unixTime, refPrice, refVol, isOption]() {
        streamRealTimeData(id, unixTime, refPrice, refVol, isOption);
        });
}

void MockClient::cancelRealTimeBars() {
    terminateStream = true;

    // Join all the threads before clearing the map
    for (auto& thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    threads_.clear();

    //std::cout << "Terminating Stream" << std::endl;
}

void MockClient::setCandleInterval(int i) { candleInterval = i; }

void MockClient::streamRealTimeData(const TickerId reqId, long unixTime, double refPrice, long refVol, bool isOption) {
    //std::unique_lock<std::mutex> lock(wrapper_.getBufferMutex());

    while (!terminateStream) {

        std::this_thread::sleep_for(std::chrono::milliseconds(candleInterval));

        MiniCandle mc = generateRandomCandle(refPrice, refVol, isOption);
        wrapper_.realtimeBar(reqId, unixTime, mc.open, mc.high, mc.low, mc.close, mc.volume, 0, 0);

        unixTime += 5;
    }
    //std::cout << "Stream thread " << reqId << " is terminating..." << std::endl;
}

//======================================================
// Helper Functions
//======================================================

MiniCandle generateRandomCandle(double referencePrice, long referenceVolume, bool opt) {

    std::random_device rd;
    std::mt19937 gen(rd());

    double priceRange = 0;
    long volumeRange = 0;

    // Define ranges around the reference price and volume
    switch (opt)
    {
    case (true):
        priceRange = 0.5;
        volumeRange = 250;
        break;
    case (false):
        priceRange = 20.0;
        volumeRange = 1000000;
        break;
    }

    // Generate randomized values for open, high, low, and close
    std::uniform_real_distribution<double> priceDist(referencePrice - priceRange, referencePrice + priceRange);
    double open = priceDist(gen);
    double high = priceDist(gen);
    double low = priceDist(gen);
    double close = priceDist(gen);

    // Generate randomized value for volume
    std::uniform_int_distribution<long> volumeDist(referenceVolume - volumeRange, referenceVolume + volumeRange);
    long volume = volumeDist(gen);

    // Make sure the high and low values are within the open and close range
    high = std::max(open, std::max(high, close));
    low = std::min(open, std::min(low, close));

    return { open, high, low, close, volume };
}

// Helper function to generate datetime intervals courtesy of chatGPT
std::vector<std::string> generateDateTimeSeries(const std::string& endDateTime, const std::string& durationString, const std::string& barSizeSetting) {
    // Convert endDateTime string to std::chrono::time_point
    std::tm tm;
    std::istringstream ss(endDateTime);
    ss >> std::get_time(&tm, "%Y%m%d %H:%M:%S");
    auto end = std::chrono::system_clock::from_time_t(std::mktime(&tm));

    // Parse durationString to get the duration in seconds
    int durationSeconds;
    char durationUnit;
    std::istringstream durationStream(durationString);
    durationStream >> durationSeconds >> durationUnit;
    if (durationUnit == 'M')
        durationSeconds *= 60;
    else if (durationUnit == 'H')
        durationSeconds *= 3600;

    // Parse barSizeSetting to get the bar size in seconds
    int barSizeSeconds;
    std::istringstream barSizeStream(barSizeSetting);
    barSizeStream >> barSizeSeconds >> durationUnit;
    if (durationUnit == 'M')
        barSizeSeconds *= 60;
    else if (durationUnit == 'H')
        barSizeSeconds *= 3600;

    // Generate date-time series at specified intervals
    std::vector<std::string> dateTimes;
    auto current = end;
    while (current > end - std::chrono::seconds(durationSeconds)) {
        std::time_t current_time_t = std::chrono::system_clock::to_time_t(current);
        std::ostringstream os;
        os << std::put_time(std::localtime(&current_time_t), "%Y%m%d %H:%M:%S");
        dateTimes.push_back(os.str());
        current -= std::chrono::seconds(barSizeSeconds);
    }

    return dateTimes;
}

std::pair<double, long> calculateOptionRefValues(TickerId id, double underlyingPrice) {
    OptionType optionType;
    double optionStrike = id;

    if (id % 5 != 0) {
        optionStrike -= 1;
        optionType = OptionType::PUT;
    }
    else {
        optionType = OptionType::CALL;
    }

    double priceDifference = std::abs(underlyingPrice - optionStrike);
    // SPX options come in increments of 5
    double strikeIncrement = 5.0;
    // Calculate the number of strikes out of the money
    int strikesOutOfTheMoney = 0;
    if (optionStrike != underlyingPrice) strikesOutOfTheMoney = static_cast<int>(std::ceil(priceDifference / strikeIncrement));

    bool ITM = false;
    if (optionType == OptionType::CALL) {
        ITM = (underlyingPrice >= optionStrike);
    }
    else {
        ITM = (underlyingPrice < optionStrike);
    }

    if (!ITM) strikesOutOfTheMoney *= -1;

    long optionVol = 0;
    double optionPrice = 0;
    if (strikesOutOfTheMoney > 0) {
        // ITM options start at 600
        optionVol = 500 + (100 * strikesOutOfTheMoney);
        optionPrice = optionVol / 100;
    }
    else if (strikesOutOfTheMoney < 0) {
        // OTM options start at 400
        optionVol = 500 - (100 * std::abs(strikesOutOfTheMoney));
        optionPrice = optionVol / 100;
        if (optionPrice < 0) {
            optionPrice = 0;
            optionVol = 0;
        }
    }
    else
    {
        optionPrice = 5.0;
        optionVol = 500;
    }

    return { optionPrice, optionVol };
}