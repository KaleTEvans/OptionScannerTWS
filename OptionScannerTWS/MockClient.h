#define _CRT_SECURE_NO_WARNINGS

#pragma once

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
		const IBString& whatToShow, bool useRTH, const TagValueListSPtr& realTimeBarsOptions);

private:
	MockWrapper& wrapper_;
};

// Helper functions
MiniCandle generateRandomCandle(double referencePrice, long referenceVolume, bool opt);
std::vector<std::string> generateDateTimeSeries(const std::string& endDateTime, const std::string& durationString, const std::string& barSizeSetting);
double calculateOptionPrice(TickerId id, double underlyingPrice);