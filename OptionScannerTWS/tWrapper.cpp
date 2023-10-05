#include "tWrapper.h"
#include "Logger.h"

//====================================================
// Wrapper for TWS API
//====================================================

tWrapper::tWrapper(int initBufferSize, bool runEReader) : candleBuffer_{ initBufferSize }, EWrapperL0(runEReader) { // Size 19 for 8 calls, 8 puts, and one underlying
    m_Done = false;
    m_ErrorForRequest = false;
}

//==================== Error Handling ========================

///Methods winError & error print the errors reported by IB TWS
void tWrapper::winError(const IBString& str, int lastError) {
    fprintf(stderr, "WinError: %d = %s\n", lastError, (const char*)str);
    m_ErrorForRequest = true;
}

void tWrapper::error(const int id, const int errorCode, const IBString errorString) {
    if (errorCode != 2176) { // 2176 is a weird api error that claims to not allow use of fractional shares
        fprintf(stderr, "Error for id=%d: %d = %s\n"
            , id, errorCode, (const char*)errorString);
        m_ErrorForRequest = (id > 0);    // id == -1 are 'system' messages, not for user requests
    }
}

///Safer: uncatched exceptions are catched before they reach the IB library code.
///       The Id is tickerId, orderId, or reqId, or -1 when no id known
void tWrapper::OnCatch(const char* MethodName, const long Id) {
    fprintf(stderr, "*** Catch in EWrapper::%s( Id=%ld, ...) \n", MethodName, Id);
}

//======================== Connectivity =============================

void tWrapper::connectionOpened() { std::cout << "Connected to TWS" << std::endl; }
void tWrapper::connectionClosed() { std::cout << "Connection has been closed" << std::endl; }

// Upon initial API connection, recieves a comma-separated string with the managed account IDs
void tWrapper::managedAccounts(const IBString& accountsList) { std::cout << accountsList << std::endl; }


// ================== tWrapper callback functions =======================

void tWrapper::currentTime(long time) { time_ = time; }

void tWrapper::tickPrice(TickerId tickerId, TickType field, double price, int canAutoExecute) {
    if (field == TickType::LAST) tickPriceLast_ = price;
    // std::cout << "Field: " << field << " Price: " << price << std::endl;
}

void tWrapper::tickGeneric(TickerId tickerId, TickType tickType, double value) {
    // std::cout << "Tick Type: " << tickType << " Value: " << value << std::endl;
}

void tWrapper::tickSize(TickerId tickerId, TickType field, int size) {
    //std::cout << "Tick Size: " << size << std::endl;
}

void tWrapper::marketDataType(TickerId reqId, int marketDataType) {
    //std::cout << "Market data type: " << marketDataType << std::endl;
}

void tWrapper::tickString(TickerId tickerId, TickType tickType, const IBString& value) {
    // std::cout << "Value: " << value << std::endl;
}

void tWrapper::tickSnapshotEnd(int reqId) {
    Req_ = reqId;
}

void tWrapper::historicalData(TickerId reqId, const IBString& date
    , double open, double high, double low, double close
    , int volume, int barCount, double WAP, int hasGaps) {

    ///Easier: EWrapperL0 provides an extra method to check all data was retrieved
    if (IsEndOfHistoricalData(date)) {
        // m_Done = true;
        // Set Req to the same value as reqId so we can retrieve the data once finished
        Req_ = reqId;
        std::cout << "Completed historical data request " << Req_ << std::endl;
        return;
    }

    long vol = static_cast<long>(volume);

    // Upon receiving the price request, populate Candle data
    std::unique_ptr<Candle> c = std::make_unique<Candle>(
        reqId, date, open, high, low, close, vol, barCount, WAP, hasGaps
        );
    historicCandles_.push_back(std::move(c));

    if (showHistoricalData_) {
        fprintf(stdout, "%10s, %5.3f, %5.3f, %5.3f, %5.3f, %7d\n"
            , (const  char*)date, open, high, low, close, volume);
    }
}

void tWrapper::realtimeBar(TickerId reqId, long time, double open, double high,
    double low, double close, long volume, double wap, int count) {

    if (showRealTimeData_) {
        std::cout << reqId << " " << time << " " << "high: " << high << " low: " << low << " volume: " << volume << std::endl;
    }

    // ReqId 1234 will be used for the underlying contract
    // Along with the other option strike reqs to fill the buffer
    std::lock_guard<std::mutex> lock(wrapperMtx_);
    // Upon receiving the price request, populate Candle data
    std::unique_ptr<Candle> c = std::make_unique<Candle>(
        reqId, time, open, high, low, close, volume, wap, count
        );

    if (activeReqs_.find(reqId) == activeReqs_.end()) activeReqs_.insert(reqId);
    //std::cout << "Current Active Reqs: " << activeReqs_.size() << std::endl;

    candleBuffer_.wrapperActiveReqs = activeReqs_.size();
    candleBuffer_.updateBuffer(std::move(c));

    cv_.notify_one();
}

// ========================= tWrapper Mutators ===========================

void tWrapper::showHistoricalDataOutput() { showHistoricalData_ = true; }
void tWrapper::hideHistoricalDataOutput() { showHistoricalData_ = false; }
void tWrapper::showRealTimeDataOutput() { showRealTimeData_ = true; }
void tWrapper::hideRealTimeDataOutput() { showRealTimeData_ = false; }
void tWrapper::setBufferCapacity(const int x) { candleBuffer_.setNewBufferCapacity(x); }

// ========================= tWrapper Accsessors ============================

int tWrapper::getReqId() { return Req_; }
long tWrapper::getCurrentTime() { return time_; }
double tWrapper::lastTickPrice() { return tickPriceLast_; }
int tWrapper::bufferCapacity() { return candleBuffer_.getCapacity(); }
bool tWrapper::checkBufferFull() { return candleBuffer_.checkBufferFull(); }

std::vector<std::unique_ptr<Candle>> tWrapper::historicCandles() { return std::move(historicCandles_); }
std::vector<std::unique_ptr<Candle>> tWrapper::processedFiveSecCandles() { return candleBuffer_.processBuffer(); }

std::mutex& tWrapper::wrapperMutex() { return wrapperMtx_; }
std::condition_variable& tWrapper::wrapperConditional() { return cv_; }


//=======================================================================
// This is a buffer to contain candlestick data and send to app when full
//=======================================================================

CandleBuffer::CandleBuffer(int capacity) : capacity_(capacity), wrapperActiveReqs{ 0 } {
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
    if (timePassed > std::chrono::seconds(60)) checkBufferStatus();

    //return buffer.size() >= capacity_ && bufferReqs.size() >= capacity_;
    return static_cast<int>(bufferMap.size()) >= capacity_;
}

void CandleBuffer::setNewBufferCapacity(int value) {
    capacity_ = value;
    wasDataProcessed_ = false;
}

void CandleBuffer::updateBuffer(std::unique_ptr<Candle> candle) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    bufferMap[candle->reqId()] = std::move(candle);
    // OPTIONSCANNER_DEBUG("Candle added to buffer, current size: {}", bufferMap.size());
}

int CandleBuffer::getCapacity() { return capacity_; }

void CandleBuffer::checkBufferStatus() {
    if (!wasDataProcessed_) {

#ifndef TEST_CONFIG
        OPTIONSCANNER_WARN("Buffer not processing data. Current buffer size: {}", bufferMap.size());
#endif // !TEST_CONFIG
        setNewBufferCapacity(bufferMap.size());
    }

    // Check if active wrapper requests outnumber current capacity
    if (wrapperActiveReqs > capacity_) {

#ifndef TEST_CONFIG
        OPTIONSCANNER_WARN("Number of open requests [{}] is greater than current buffer capacity [{}]", wrapperActiveReqs, capacity_);
#endif // !TEST_CONFIG
        setNewBufferCapacity(wrapperActiveReqs);
    }

    bufferTimePassed_ = std::chrono::steady_clock::now();
}