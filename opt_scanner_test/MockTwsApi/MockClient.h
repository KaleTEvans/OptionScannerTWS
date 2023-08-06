#define _CRT_SECURE_NO_WARNINGS

#pragma once

#include <unordered_map>
#include <thread>
#include <chrono>
#include <atomic>

#include "MockWrapper.h"

// For use in random candle generation
struct MiniCandle {
	double open;
	double high;
	double low;
	double close;
	long volume;
};

enum class OptionType {
	CALL,
	PUT
};

class MockClient {
public:
	MockClient(MockWrapper& moockWrapper);

	virtual void reqCurrentTime();

	virtual void reqHistoricalData(TickerId id, const Contract& contract,
		const IBString& endDateTime, const IBString& durationStr, const IBString& barSizeSetting,
		const IBString& whatToShow, int useRTH, int formatDate, bool keepUpToDate);

	virtual void reqRealTimeBars(TickerId id, const Contract& contract, int barSize,
		const IBString& whatToShow, bool useRTH);

	virtual void cancelRealTimeBars(void);

	void setCandleInterval(int i); // In miliseconds

	// Mock RTB stream
	void streamRealTimeData(const TickerId reqId, long unixTime, double refPrice, long refVol, bool isOption);

private:
	MockWrapper& wrapper_;

	bool terminateStream = false;

	std::vector<std::thread> threads_;
	std::condition_variable cv_;

	int candleInterval = 25;
};

// Helper functions
MiniCandle generateRandomCandle(double referencePrice, long referenceVolume, bool opt);
std::vector<std::string> generateDateTimeSeries(const std::string& endDateTime, const std::string& durationString, const std::string& barSizeSetting);
std::pair<double, long> calculateOptionRefValues(TickerId id, double underlyingPrice);