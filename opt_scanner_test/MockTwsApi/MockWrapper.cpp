//#include "pch.h"
#include "MockWrapper.h"
#include "Candle.h"

//=======================================================================
// This is a buffer to contain candlestick data and send to app when full
//=======================================================================

CandleBuffer::CandleBuffer(size_t capacity) : capacity_(capacity) {
    bufferTimePassed_ = std::chrono::steady_clock::now();
}

std::vector<std::unique_ptr<Candle>> CandleBuffer::processBuffer() {
    std::lock_guard<std::mutex> lock(bufferMutex);
    std::vector<std::unique_ptr<Candle>> processedData;

    for (auto& c : bufferMap) processedData.push_back(std::move(c.second));
    bufferMap.clear();

    wasDataProcessed_ = true;

    return processedData;
}

bool CandleBuffer::checkBufferFull() {
    std::lock_guard<std::mutex> lock(bufferMutex);
    auto currentTime = std::chrono::steady_clock::now();
    auto timePassed = currentTime - bufferTimePassed_;
    if (timePassed > std::chrono::seconds(3)) checkBufferStatus();

    //return buffer.size() >= capacity_ && bufferReqs.size() >= capacity_;
    return bufferMap.size() >= capacity_;
}

void CandleBuffer::setNewBufferCapacity(int value) {
    capacity_ = value;
    wasDataProcessed_ = false;
}

void CandleBuffer::updateBuffer(std::unique_ptr<Candle> candle) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    bufferMap[candle->getReqId()] = std::move(candle);
}

int CandleBuffer::getCapacity() { return capacity_; }

void CandleBuffer::checkBufferStatus() {
    if (!wasDataProcessed_) {
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
}

//==================================================================
// Mock Wrapper
//=================================================================

MockWrapper::MockWrapper() : candleBuffer{ 19 } { m_done = false; } // Size 19 for 8 calls, 8 puts, and one underlying

// Getters
bool MockWrapper::notDone() { return !m_done; }
long MockWrapper::getCurrentTime() { return time_; }
double MockWrapper::getSPXPrice() { return SPXPrice; }
int MockWrapper::getBufferCapacity() { return candleBuffer.getCapacity(); }

// Moving unique pointers will automatically clear the vectors
std::vector<std::unique_ptr<Candle>> MockWrapper::getHistoricCandles() {
    return std::move(historicCandles);
}


std::vector<std::unique_ptr<Candle>> MockWrapper::getProcessedFiveSecCandles() { return candleBuffer.processBuffer(); }

bool MockWrapper::checkMockBufferFull() { return candleBuffer.checkBufferFull(); }
std::mutex& MockWrapper::getWrapperMutex() { return wrapperMtx; }
std::condition_variable& MockWrapper::getWrapperConditional() { return cv; }

// Setters
void MockWrapper::showHistoricalDataOutput() { showHistoricalData = true; }
void MockWrapper::hideHistoricalDataOutput() { showHistoricalData = false; }
void MockWrapper::showRealTimeDataOutput() { showRealTimeData = true; }
void MockWrapper::hideRealTimeDataOutput() { showRealTimeData = false; }
void MockWrapper::setmDone(bool x) { m_done = x; }
void MockWrapper::setMockUnderlying(double x) { SPXPrice = x; }
void MockWrapper::setBufferCapacity(int x) { candleBuffer.setNewBufferCapacity(x); }

void MockWrapper::currentTime(long time) { time_ = time; }

void MockWrapper::historicalData(TickerId reqId, const IBString& date
    , double open, double high, double low, double close
    , int volume, int barCount, double WAP, int hasGaps) {

    long vol = static_cast<long>(volume);

    // Upon receiving the price request, populate Candle data
    std::unique_ptr<Candle> c = std::make_unique<Candle>(
        reqId, date, open, high, low, close, vol, barCount, WAP, hasGaps
        );
    historicCandles.push_back(std::move(c));

    if (showHistoricalData) {
        fprintf(stdout, "%10s, %5.3f, %5.3f, %5.3f, %5.3f, %7d\n"
            , (const  char*)date, open, high, low, close, volume);
    }
}

void MockWrapper::realtimeBar(TickerId reqId, long time, double open, double high,
    double low, double close, long volume, double wap, int count) {

    // Upon receiving the price request, populate Candle data
    std::unique_ptr<Candle> c = std::make_unique<Candle>(
        reqId, time, open, high, low, close, volume, wap, count
        );

    if (showRealTimeData) {
        std::cout << reqId << " " << time << " " << "high: " << high << " low: " << low << " volume: " << volume << std::endl;
    }

    // To imitate option pricing, we will need to update the client with each new 
    // underlying price, ie SPX with reqId 1234
    if (reqId == 1234) SPXPrice = close;

    // ReqId 1234 will be used for the underlying contract
    // Along with the other option strike reqs to fill the buffer
    std::lock_guard<std::mutex> lock(wrapperMtx);
    if (activeReqs.find(reqId) == activeReqs.end()) activeReqs.insert(reqId);

    candleBuffer.wrapperActiveReqs = activeReqs.size();
    candleBuffer.updateBuffer(std::move(c));

    cv.notify_one();
}