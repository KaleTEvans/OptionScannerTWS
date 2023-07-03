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
	vector<Candle> getFiveSecData() const;
	vector<Candle> getThirtySecData() const;
	vector<Candle> getOneMinData() const;
	vector<Candle> getFiveMinData() const;

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