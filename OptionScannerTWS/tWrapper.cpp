#include "tWrapper.h"


//=======================================================================
// This is a buffer to contain candlestick data and send to app when full
//=======================================================================

CandleBuffer::CandleBuffer(size_t capacity) : capacity_(capacity) {}

void CandleBuffer::processBuffer(std::vector<Candle>& wrapperContainer) {
    if (buffer.size() == capacity_ && bufferReqs.size() == capacity_) {
        // std::cout << "Processing buffer with " << buffer.size() << " candles" << std::endl;
        // Append buffer contents to the target vector
        wrapperContainer.clear();
        wrapperContainer.insert(wrapperContainer.end(), buffer.begin(), buffer.end());
        // Clear the buffer
        buffer.clear();
        // Clear the set
        bufferReqs.clear();
    }
}

size_t CandleBuffer::checkBufferCapacity() { 
    return capacity_; 
}

size_t CandleBuffer::getCurrentBufferLoad() {
    return buffer.size();
}

void CandleBuffer::setNewBufferCapacity(int value) {
    capacity_ = value;
}

void CandleBuffer::addToBuffer(Candle candle) { 
    buffer.push_back(candle); 
}

bool CandleBuffer::checkSet(int value) {
    if (bufferReqs.find(value) == bufferReqs.end()) return false;
    else return true;
}

void CandleBuffer::addToSet(int value) {
    bufferReqs.insert(value);
}

//============================================================
// tWrapper callback functions
//============================================================

void tWrapper::historicalData(TickerId reqId, const IBString& date
    , double open, double high, double low, double close
    , int volume, int barCount, double WAP, int hasGaps) {

    ///Easier: EWrapperL0 provides an extra method to check all data was retrieved
    if (IsEndOfHistoricalData(date)) {
        // m_Done = true;
        // Set Req to the same value as reqId so we can retrieve the data once finished
        Req = reqId;
        std::cout << "Completed historical data request " << Req << std::endl;
        return;
    }

    // Upon receiving the price request, populate candlestick data
    Candle c(reqId, date, open, high, low, close, volume, barCount, WAP, hasGaps);
    underlyingCandles.push_back(c);

    if (showHistoricalData) {
        fprintf(stdout, "%10s, %5.3f, %5.3f, %5.3f, %5.3f, %7d\n"
            , (const  char*)date, open, high, low, close, volume);

        m_Done = true;
    }
}