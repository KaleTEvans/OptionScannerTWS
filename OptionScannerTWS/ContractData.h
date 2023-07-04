//==============================================================================
// Contract Data will perform a variety of functions for each contract under the 
// current scope of strikes. These will inlcude:
//	*Receiving 5 second interval data from the wrapper and organizing into 
//		5 sec, 30 sec, 1 min and 5 min candles
//	*Updating the standard deviation of difference in high and low of price
//		and of the volume for each candle
//	*Outliers in the stdev, which will be defined incrementally, will send out
//		alerts, via logging or other methods
//	*All data will be stored for historical analysis
//==============================================================================
#define _CRT_SECURE_NO_WARNINGS

#pragma once


#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <functional>

#include "tWrapper.h"
#include "Formulas.h"

using std::vector;
using std::string;
using std::max;
using std::min;

class ContractData {
public:
	ContractData(TickerId reqId, Candle initData, bool isUnderlying = false);

	// With each incoming candle, we will need to update the vectors for each time frame
	// This will also update stDevs for each time series
	void updateData(Candle c);

	// Time series accessors
	vector<Candle> getFiveSecData() const { return fiveSecCandles; }
	vector<Candle> getThirtySecData() const { return thirtySecCandles; }
	vector<Candle> getOneMinData() const { return oneMinCandles; }
	vector<Candle> getFiveMinData() const { return fiveMinCandles; }

	// Other data acessors
	double getCurrentPrice() const { return fiveSecCandles[fiveSecCandles.size() - 1].close; }
	double getDailyHigh() const { return dailyHigh; }
	double getDailyLow() const { return dailyLow; }
	double getLocalHigh() const { return localHigh; }
	double getLocalLow() const { return localLow; }
	long getCumulativeVol() const { return cumulativeVolume[cumulativeVolume.size() - 1].second; }

	vector<bool> getHighLowComparisons() const {
		vector<bool> comparisons;
		comparisons.push_back(nearDailyLow);
		comparisons.push_back(nearDailyHigh);
		comparisons.push_back(nearLocalLow);
		comparisons.push_back(nearLocalHigh);

		return comparisons;
	}

	TickerId contractId = 0; // Different from contractStrike for unique key put
	int contractStrike = 0;
	string optionType = "";

private:
	vector<Candle> fiveSecCandles;
	vector<Candle> thirtySecCandles;
	vector<Candle> oneMinCandles;
	vector<Candle> fiveMinCandles;

	// Statistic Variables
	StandardDeviation sd5Sec;
	StandardDeviation sd30Sec;
	StandardDeviation sd1Min;
	StandardDeviation sd5Min;

	StandardDeviation sdVol5Sec;
	StandardDeviation sdVol30Sec;
	StandardDeviation sdVol1Min;
	StandardDeviation sdVol5Min;

	double dailyHigh = 0;
	double dailyLow = 10000;

	// To be used for local high and low in 30 minute frames
	double localHigh = 0;
	double localLow = 10000;
	double tempHigh = 0;
	double tempLow = 10000;

	// Data retrieval to compare and add to alerts
	bool nearDailyHigh = false;
	bool nearDailyLow = false;
	bool nearLocalHigh = false;
	bool nearLocalLow = false;

	// Function to update the comparison values
	void updateUnderlyingComparisons();

	// For data keeping purposes
	vector<std::pair<long, long>> cumulativeVolume;

	// We will also need to keep a connection open for the underlying price
	// Will cancel all alerts when an underlying security is being passed through
	bool isUnderlying = false;

//=========================================
// Callback Functionality for Alerts
//=========================================
public:
	using AlertFunction = std::function<void(int, double, double, Candle)>;
	void registerAlert(AlertFunction alert);

private:
	AlertFunction alert_;
};