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

#include "tWrapper.h"
#include "Enums.h"
#include "Candle.h"
#include "Formulas.h"
#include "DatabaseManager.h"

using std::vector;
using std::string;
using std::max;
using std::min;
using std::pair;

// Helper function for combining candles
std::shared_ptr<Candle> createNewBars(int id, int increment, const vector<std::shared_ptr<Candle>> data);

class ContractData {
public:
	ContractData(TickerId reqId, std::unique_ptr<Candle> initData);

	// Set the sql connection variable if pushing to db
	void setupDatabaseManager(std::shared_ptr<OptionDB::DatabaseManager> dbm);

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

	std::shared_ptr<Candle> latestCandle(TimeFrame tf);
	vector<std::shared_ptr<Candle>> candlesLast30Minutes();

	// Other data acessors
	double currentPrice() const;
	double dailyHigh() const;
	double dailyLow() const;
	double localHigh() const;
	double localLow() const;
	long long totalVol() const;

	vector<std::pair<long, long long>> volOverTime() const;
	vector<bool> highLowComparisons() const;

	StandardDeviation priceStDev(TimeFrame tf);
	StandardDeviation volStDev(TimeFrame tf);

private:
	const TickerId contractId_;
	std::shared_ptr<OptionDB::DatabaseManager> dbm_{ nullptr };
	bool dbConnect{ false };

	vector<std::shared_ptr<Candle>> fiveSecCandles_;
	vector<std::shared_ptr<Candle>> thirtySecCandles_;
	vector<std::shared_ptr<Candle>> oneMinCandles_;
	vector<std::shared_ptr<Candle>> fiveMinCandles_;

	// Statistic Variables
	StandardDeviation sdPrice5Sec_;
	StandardDeviation sdPrice30Sec_;
	StandardDeviation sdPrice1Min_;
	StandardDeviation sdPrice5Min_;

	StandardDeviation sdVol5Sec_;
	StandardDeviation sdVol30Sec_;
	StandardDeviation sdVol1Min_;
	StandardDeviation sdVol5Min_;

	double dailyHigh_;
	double dailyLow_;

	// To be used for local high and low in 30 minute frames
	double localHigh_;
	double localLow_;
	double tempHigh_;
	double tempLow_;

	// Data retrieval to compare and add to alerts
	bool nearDailyHigh;
	bool nearDailyLow;
	bool nearLocalHigh;
	bool nearLocalLow;

	// Update various trackers
	// Update respective containers with new candles and stdev values
	void updateContainers(std::shared_ptr<Candle> c, TimeFrame tf);
	void updateCumulativeVolume(std::shared_ptr<Candle> c);
	void updateComparisons();
	void updateLocalMinMax(std::shared_ptr<Candle> c);

	// For data keeping purposes
	vector<std::pair<long, long long>> cumulativeVolume_;

	// We will also need to keep a connection open for the underlying price
	// Will cancel all alerts when an underlying security is being passed through
	bool isUnderlying_{ false };

//=========================================
// Callback Functionality for Alerts
//=========================================
public:
	using AlertFunction = std::function<void(TimeFrame tf, std::shared_ptr<Candle> candle)>;
	void registerAlert(AlertFunction alert) { alert_ = std::move(alert); }

private:
	AlertFunction alert_;
};