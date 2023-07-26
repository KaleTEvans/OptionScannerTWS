#include "MockWrapper.h"

// Getters
int MockWrapper::getReq() { return Req; }

std::vector<Candle> MockWrapper::getUnderlyingCandles() { return underlyingCandles; }
std::vector<Candle> MockWrapper::getFiveSecCandles() { return fiveSecCandles; }

void MockWrapper::connectionOpened() {
	std::cout << "Connected to TWS" << std::endl;
}

void MockWrapper::connectionClosed() {
	std::cout << "Connection has been closed" << std::endl;
}

void MockWrapper::currentTime(long time) {
	std::cout << "Current Unix time received from TWS: " << time << std::endl;
}

void MockWrapper::historicalData(TickerId reqId, const IBString& date
	, double open, double high, double low, double close
	, int volume, int barCount, double WAP, int hasGaps) {

    // Upon receiving the price request, populate candlestick data
    Candle c(reqId, date, open, high, low, close, volume, barCount, WAP, hasGaps);
    underlyingCandles.push_back(c);

    if (showHistoricalData) {
        fprintf(stdout, "%10s, %5.3f, %5.3f, %5.3f, %5.3f, %7d\n"
            , (const  char*)date, open, high, low, close, volume);
    }
}

void MockWrapper::realtimeBar(TickerId reqId, long time, double open, double high,
    double low, double close, long volume, double wap, int count) {

    // Upon receiving the price request, populate candlestick data
    Candle c(reqId, time, open, high, low, close, volume, wap, count);

    // ReqId 1234 will be used for the underlying contract
    if (reqId == 1234 || singleRTBs) {
        underlyingRTBs.reqId = reqId;
        underlyingRTBs.time = time;
        underlyingRTBs.open = open;
        underlyingRTBs.high = high;
        underlyingRTBs.low = low;
        underlyingRTBs.close = close;
        underlyingRTBs.volume = volume;
    }
    // Otherwise use the buffer for the option contracts
    else {
        if (!candleBuffer.checkSet(c.reqId)) {
            candleBuffer.addToBuffer(c);

            candleBuffer.addToSet(c.reqId);
        }
        candleBuffer.processBuffer(fiveSecCandles);
    }

    if (showRealTimeData) {
        std::cout << reqId << " " << time << " " << "high: " << high << " low: " << low << " volume: " << volume << std::endl;
    }
}