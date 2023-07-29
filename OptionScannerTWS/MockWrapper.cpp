#include "MockWrapper.h"
#include "Candle.h"

//=======================================================================
// This is a buffer to contain candlestick data and send to app when full
//=======================================================================

CandleStickBuffer::CandleStickBuffer(size_t capacity) : capacity_(capacity) {}

void CandleStickBuffer::processBuffer(std::vector<std::unique_ptr<CandleStick>>& wrapperContainer) {
    if (buffer.size() == capacity_ && bufferReqs.size() == capacity_) {
        // std::cout << "Processing buffer with " << buffer.size() << " candles" << std::endl;
        // Append buffer contents to the target vector
        wrapperContainer.clear();
        for (auto& c : buffer) wrapperContainer.push_back(std::move(c));
        // Clear the buffer
        buffer.clear();
        // Clear the set
        bufferReqs.clear();
    }
}

size_t CandleStickBuffer::checkBufferCapacity() {
    return capacity_;
}

size_t CandleStickBuffer::getCurrentBufferLoad() {
    return buffer.size();
}

void CandleStickBuffer::setNewBufferCapacity(int value) {
    capacity_ = value;
}

void CandleStickBuffer::addToBuffer(std::unique_ptr<CandleStick> candle) {
    buffer.push_back(std::move(candle));
}

bool CandleStickBuffer::checkSet(int value) {
    if (bufferReqs.find(value) == bufferReqs.end()) return false;
    else return true;
}

void CandleStickBuffer::addToSet(int value) {
    bufferReqs.insert(value);
}

//==================================================================
// Mock Wrapper
//=================================================================

MockWrapper::MockWrapper() : candleBuffer{ 19 } { m_done = false; } // Size 19 for 8 calls, 8 puts, and one underlying

// Getters
int MockWrapper::getReq() { return Req; }
bool MockWrapper::notDone() { return !m_done; }
double MockWrapper::getSPXPrice() { return SPXPrice; }

std::vector<std::unique_ptr<CandleStick>> MockWrapper::getHistoricCandles() { return std::move(historicCandles); }
std::vector<std::unique_ptr<CandleStick>> MockWrapper::getFiveSecCandles() { return std::move(fiveSecCandles); }

// Setters
void MockWrapper::setHistoricalDataVisibility(bool x) { showHistoricalData = x; }
void MockWrapper::setRealTimeDataVisibility(bool x) { showRealTimeData = x; }
void MockWrapper::setmDone(bool x) { m_done = x; }

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

    // To imitate option pricing, we will need to update the client with each new 
    // underlying price, ie SPX with reqId 1234
    if (reqId == 1234) SPXPrice = close;

    // ReqId 1234 will be used for the underlying contract
    // Along with the other option strike reqs to fill the buffer
    if (!candleBuffer.checkSet(c->getReqId())) {
        candleBuffer.addToSet(c->getReqId());

        candleBuffer.addToBuffer(std::move(c));
    }
    candleBuffer.processBuffer(fiveSecCandles);

    if (showRealTimeData) {
        std::cout << reqId << " " << time << " " << "high: " << high << " low: " << low << " volume: " << volume << std::endl;
    }
}