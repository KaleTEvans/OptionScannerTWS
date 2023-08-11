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
#include <memory>

//#include "tWrapper.h"
#include "Candle.h"
#include "Formulas.h"

using std::vector;
using std::string;
using std::max;
using std::min;
using std::pair;

// Time frame enums
enum class TimeFrame {
	FiveSecs,
	ThirtySecs,
	OneMin,
	FiveMin
};

class ContractData {
public:
	ContractData(TickerId reqId, std::unique_ptr<Candle> initData);

	// With each incoming candle, we will need to update the vectors for each time frame
	// This will also update stDevs for each time series
	void updateData(std::unique_ptr<Candle> c);

	// Accessors
	TickerId contractId() const;

	// Time series accessors
	vector<std::shared_ptr<Candle>> fiveSecData() const;
	vector<std::shared_ptr<Candle>> thirtySecData() const;
	vector<std::shared_ptr<Candle>> oneMinData() const;
	vector<std::shared_ptr<Candle>> fiveMinData() const;

	// Other data acessors
	double currentPrice() const;
	double dailyHigh() const;
	double dailyLow() const;
	double localHigh() const;
	double localLow() const;
	long cumulativeVol() const;

	pair<StandardDeviation, StandardDeviation> get5SecStDev() const;
	pair<StandardDeviation, StandardDeviation> get30SecStDev() const;
	pair<StandardDeviation, StandardDeviation> get1MinStDev() const;
	pair<StandardDeviation, StandardDeviation> get5MinStDev() const;

	vector<bool> highLowComparisons() const;

private:
	TickerId contractId_ = 0;

	vector<std::shared_ptr<Candle>> fiveSecCandles;
	vector<std::shared_ptr<Candle>> thirtySecCandles;
	vector<std::shared_ptr<Candle>> oneMinCandles;
	vector<std::shared_ptr<Candle>> fiveMinCandles;

	// Statistic Variables
	StandardDeviation sd5Sec;
	StandardDeviation sd30Sec;
	StandardDeviation sd1Min;
	StandardDeviation sd5Min;

	StandardDeviation sdVol5Sec;
	StandardDeviation sdVol30Sec;
	StandardDeviation sdVol1Min;
	StandardDeviation sdVol5Min;

	double dailyHigh_ = 0;
	double dailyLow_= 10000;

	// To be used for local high and low in 30 minute frames
	double localHigh_ = 0;
	double localLow_ = 10000;
	double tempHigh_ = 0;
	double tempLow_= 10000;

	// Data retrieval to compare and add to alerts
	bool nearDailyHigh = false;
	bool nearDailyLow = false;
	bool nearLocalHigh = false;
	bool nearLocalLow = false;

	// Function to update the comparison values
	void updateComparisons();

	// For data keeping purposes
	vector<std::pair<long, long>> cumulativeVolume_;

	// We will also need to keep a connection open for the underlying price
	// Will cancel all alerts when an underlying security is being passed through
	bool isUnderlying_ = false;

//=========================================
// Callback Functionality for Alerts
//=========================================
public:
	using AlertFunction = std::function<void(TimeFrame tf, StandardDeviation, StandardDeviation, std::shared_ptr<Candle> candle)>;
	void registerAlert(AlertFunction alert) { alert_ = std::move(alert); }

private:
	AlertFunction alert_;
};