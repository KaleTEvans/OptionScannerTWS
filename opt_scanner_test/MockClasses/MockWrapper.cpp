//#include "pch.h"
#include "MockWrapper.h"
#include "Candle.h"

//=======================================================================
// This is a buffer to contain candlestick data and send to app when full
//=======================================================================

MockCandleBuffer::MockCandleBuffer(int capacity) : capacity_(capacity) {
    bufferTimePassed_ = std::chrono::steady_clock::now();
}

std::vector<std::unique_ptr<Candle>> MockCandleBuffer::processBuffer() {
    std::lock_guard<std::mutex> lock(bufferMutex);
    std::vector<std::unique_ptr<Candle>> processedData;

    for (auto& c : bufferMap) processedData.push_back(std::move(c.second));
    bufferMap.clear();

    wasDataProcessed_ = true;

    return processedData;
}

bool MockCandleBuffer::checkBufferFull() {
    std::lock_guard<std::mutex> lock(bufferMutex);
    auto currentTime = std::chrono::steady_clock::now();
    auto timePassed = currentTime - bufferTimePassed_;
    if (timePassed > std::chrono::seconds(3)) checkBufferStatus();

    //return buffer.size() >= capacity_ && bufferReqs.size() >= capacity_;
    return static_cast<int>(bufferMap.size()) >= capacity_;
}

void MockCandleBuffer::setNewBufferCapacity(int value) {
    capacity_ = value;
    wasDataProcessed_ = false;
}

void MockCandleBuffer::updateBuffer(std::unique_ptr<Candle> candle) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    bufferMap[candle->reqId()] = std::move(candle);
}

int MockCandleBuffer::getCapacity() { return capacity_; }

void MockCandleBuffer::checkBufferStatus() {
    if (!wasDataProcessed_) {
        // ****** In real program, log an error each time this occurs *********
        //std::cout << "Error, buffer not processing data. Current Buffer Size: " << bufferMap.size() << 
        //    " Current Buffer Capacity: " << capacity_ << std::endl;
        //std::cout << "Resetting buffer capacity to current size ..." << std::endl;
        setNewBufferCapacity(bufferMap.size());
    }

    // Check if active wrapper requests outnumber current capacity
    if (wrapperActiveReqs > capacity_) {
        //std::cout << "Warning, number of open requests outnumbers current capacity, updating capacity ..." << std::endl;
        //std::cout << "Active Reqs: " << wrapperActiveReqs << " Capacity: " << capacity_ << std::endl;
        setNewBufferCapacity(wrapperActiveReqs);
    }

    // Update new time passed
    bufferTimePassed_ = std::chrono::steady_clock::now();
}

//==================================================================
// Mock Wrapper
//=================================================================

MockWrapper::MockWrapper() : mcb{ 19 } { m_done = false; } // Size 19 for 8 calls, 8 puts, and one underlying

// Getters
bool MockWrapper::notDone() { return !m_done; }
long MockWrapper::getCurrentTime() { return time_; }
double MockWrapper::getSPXPrice() { return SPXPrice; }
int MockWrapper::getBufferCapacity() { return mcb.getCapacity(); }
int MockWrapper::checkActiveReqs() { return activeReqs.size(); }
std::unordered_set<int> MockWrapper::getActiveReqs() { return activeReqs;
}

// Moving unique pointers will automatically clear the vectors
std::vector<std::unique_ptr<Candle>> MockWrapper::getHistoricCandles() {
    return std::move(historicCandles);
}


std::vector<std::unique_ptr<Candle>> MockWrapper::getProcessedFiveSecCandles() { return mcb.processBuffer(); }

bool MockWrapper::checkMockBufferFull() { return mcb.checkBufferFull(); }
std::mutex& MockWrapper::getWrapperMutex() { return wrapperMtx; }
std::condition_variable& MockWrapper::getWrapperConditional() { return cv; }

// Setters
void MockWrapper::showHistoricalDataOutput() { showHistoricalData = true; }
void MockWrapper::hideHistoricalDataOutput() { showHistoricalData = false; }
void MockWrapper::showRealTimeDataOutput() { showRealTimeData = true; }
void MockWrapper::hideRealTimeDataOutput() { showRealTimeData = false; }
void MockWrapper::setmDone(bool x) { m_done = x; }
void MockWrapper::setMockUnderlying(double x) { SPXPrice = x; }
void MockWrapper::setBufferCapacity(const int x) { mcb.setNewBufferCapacity(x); }

void MockWrapper::currentTime(long time) { time_ = time; }

void MockWrapper::historicalData(TickerId reqId, const IBString& date
    , double open, double high, double low, double close
    , int volume, int barCount, double WAP, int hasGaps) {

    long vol = static_cast<long>(volume);

    // Will test different retrieval methods to measure speed
    switch (candleBenchmarkSwitch)
    {
    case 1:
    {   // Upon receiving the price request, populate Candle data
        std::unique_ptr<Candle> c1 = std::make_unique<Candle>(
            reqId, date, open, high, low, close, vol, barCount, WAP, hasGaps
            );
        historicCandles.push_back(std::move(c1));
        break;
    }
    case 2:
    {   std::unique_ptr<Candle> c2 = std::make_unique<Candle>(
        reqId, date, open, high, low, close, vol, barCount, WAP, hasGaps
        );
        movedCandleQueue.push(std::move(c2));
        break;
    }
    case 3:
    {   Candle c3(reqId, date, open, high, low, close, vol, barCount, WAP, hasGaps);
        copiedCandleVector.push_back(c3);
        break;
    }
    case 4:
        Candle c4(reqId, date, open, high, low, close, vol, barCount, WAP, hasGaps);
        copiedCandleQueue.push(c4);
        break;
    }

    if (showHistoricalData) {
        fprintf(stdout, "%10s, %5.3f, %5.3f, %5.3f, %5.3f, %7d\n"
            , (const  char*)date, open, high, low, close, volume);
    }
}

void MockWrapper::realtimeBar(TickerId reqId, long time, double open, double high,
    double low, double close, long volume, double wap, int count) {

    // To imitate option pricing, we will need to update the client with each new 
    // underlying price, ie SPX with reqId 1234
    if (reqId == 1234) SPXPrice = close;

    if (showRealTimeData) {
        std::cout << reqId << " " << time << " " << "high: " << high << " low: " << low << " volume: " << volume << std::endl;
    }

    // ReqId 1234 will be used for the underlying contract
    // Along with the other option strike reqs to fill the buffer
    std::lock_guard<std::mutex> lock(wrapperMtx);
    // Upon receiving the price request, populate Candle data
    std::unique_ptr<Candle> c = std::make_unique<Candle>(
        reqId, time, open, high, low, close, volume, wap, count
    );

    if (activeReqs.find(reqId) == activeReqs.end()) activeReqs.insert(reqId);

    mcb.wrapperActiveReqs = activeReqs.size();
    mcb.updateBuffer(std::move(c));

    cv.notify_one();
}

std::vector<Candle> MockWrapper::getCopiedCandleVector() { return copiedCandleVector; }
std::queue<Candle> MockWrapper::getCopiedCandleQueue() { return copiedCandleQueue; }
std::queue<std::unique_ptr<Candle>> MockWrapper::getMovedCandleQueue() { return std::move(movedCandleQueue); }

void MockWrapper::clearTestContainers() {
    historicCandles.clear();
    copiedCandleVector.clear();
    copiedCandleQueue = {};
    movedCandleQueue = {};
}