#include "MockWrapper.h"
#include "Candle.h"

//=======================================================================
// This is a buffer to contain candlestick data and send to app when full
//=======================================================================

CandleStickBuffer::CandleStickBuffer(size_t capacity) : capacity_(capacity) {}

std::vector<std::unique_ptr<CandleStick>> CandleStickBuffer::processBuffer() {
    std::lock_guard<std::mutex> lock(bufferMutex);
    std::cout << "Processing buffer with " << buffer.size() << " candles" << std::endl;

    std::vector<std::unique_ptr<CandleStick>> processedData;

    for (auto& c : buffer) processedData.push_back(std::move(c));
    // Clear the buffer
    buffer.clear();
    // Clear the set
    bufferReqs.clear();

    return processedData;
}

bool CandleStickBuffer::checkBufferFull() { return buffer.size() >= capacity_ && bufferReqs.size() >= capacity_; }

void CandleStickBuffer::setNewBufferCapacity(int value) {
    capacity_ = value;
}

void CandleStickBuffer::addToBuffer(std::unique_ptr<CandleStick> candle) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    buffer.push_back(std::move(candle));
}

bool CandleStickBuffer::checkSet(int value) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    if (bufferReqs.find(value) != bufferReqs.end()) return true;
    else return false;
}

void CandleStickBuffer::addToSet(int value) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    bufferReqs.insert(value);
    //std::cout << "Buffer Reqs: " << bufferReqs.size() << std::endl;
}

//==================================================================
// Mock Wrapper
//=================================================================

MockWrapper::MockWrapper() : candleBuffer{ 2 } { m_done = false; } // Size 19 for 8 calls, 8 puts, and one underlying

// Getters
bool MockWrapper::notDone() { return !m_done; }
long MockWrapper::getCurrentTime() { return time_; }
double MockWrapper::getSPXPrice() { return SPXPrice; }

// Moving unique pointers will automatically clear the vectors
std::vector<std::unique_ptr<CandleStick>> MockWrapper::getHistoricCandles() { return std::move(historicCandles); } 
std::vector<std::unique_ptr<CandleStick>> MockWrapper::getProcessedFiveSecCandles() { return candleBuffer.processBuffer(); }

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

    // Upon receiving the price request, populate candlestick data
    std::unique_ptr<CandleStick> c = std::make_unique<CandleStick>(
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

    // Upon receiving the price request, populate candlestick data
    std::unique_ptr<CandleStick> c = std::make_unique<CandleStick>(
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
    if (!candleBuffer.checkSet(c->getReqId())) {
        candleBuffer.addToSet(c->getReqId());

        candleBuffer.addToBuffer(std::move(c));
        
    }
    cv.notify_one();

    //if (candleBuffer.checkBufferFull()) std::vector<std::unique_ptr<CandleStick>> temp = candleBuffer.processBuffer();
}