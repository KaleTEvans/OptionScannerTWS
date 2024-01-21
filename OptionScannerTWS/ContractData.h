#define _CRT_SECURE_NO_WARNINGS

#pragma once


#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <functional>
#include <memory>

#include "tWrapper.h"
#include "Logger.h"
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

// Contains vol and price tags for each timeframe that will be updated with new candles
struct VolAndPriceTags {
	Alerts::VolumeStDev volStDev5Sec{ Alerts::VolumeStDev::LowVol };
	Alerts::VolumeThreshold volThresh5Sec{ Alerts::VolumeThreshold::LowVol };
	Alerts::PriceDelta priceDelta5Sec{ Alerts::PriceDelta::Under1 };

	Alerts::VolumeStDev volStDev30Sec{ Alerts::VolumeStDev::LowVol };
	Alerts::VolumeThreshold volThresh30Sec{ Alerts::VolumeThreshold::LowVol };
	Alerts::PriceDelta priceDelta30Sec{ Alerts::PriceDelta::Under1 };

	Alerts::VolumeStDev volStDev1Min{ Alerts::VolumeStDev::LowVol };
	Alerts::VolumeThreshold volThresh1Min{ Alerts::VolumeThreshold::LowVol };
	Alerts::PriceDelta priceDelta1Min{ Alerts::PriceDelta::Under1 };

	Alerts::VolumeStDev volStDev5Min{ Alerts::VolumeStDev::LowVol };
	Alerts::VolumeThreshold volThresh5Min{ Alerts::VolumeThreshold::LowVol };
	Alerts::PriceDelta priceDelta5Min{ Alerts::PriceDelta::Under1 };

	Alerts::VolumeStDev updateVolStDev(double volStDev);
	Alerts::VolumeThreshold updateVolThreshold(long volume);
	Alerts::PriceDelta updatePriceDelta(double priceStDev);
};

//==============================================================================
// Contract Data will perform a variety of functions for each contract under the 
// current scope of strikes. These will inlcude:
//	*Receiving 5 second interval data from the wrapper and organizing into 
//		5 sec, 30 sec, 1 min and 5 min candles
//	*Updating the standard deviation of difference in high and low of price
//		and of the volume for each candle
//	*All data will be stored for historical analysis
//==============================================================================

class ContractData {
public:
	ContractData(TickerId reqId);
	// Constructor for underlying with dbm connection
	ContractData(TickerId reqId, std::shared_ptr<OptionDB::DatabaseManager> dbm);

	// Set the sql connection variable if pushing to db
	void setupDatabaseManager(std::shared_ptr<OptionDB::DatabaseManager> dbm);

	// With each incoming candle, we will need to update the vectors for each time frame
	// This will also update stDevs for each time series
	void updateData(std::unique_ptr<Candle> c);

	// Accessors
	TickerId contractId() const;
	int strikePrice() const;
	Alerts::OptionType optType() const;

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

	StandardDeviation priceStDev(TimeFrame tf);
	StandardDeviation volStDev(TimeFrame tf);

	// Tag Accessors
	Alerts::PriceDelta priceDelta(TimeFrame tf);
	Alerts::DailyHighsAndLows dailyHLComparison();
	Alerts::LocalHighsAndLows localHLComparison();

private:
	const TickerId contractId_;
	int strikePrice_{ 0 };

	std::shared_ptr<OptionDB::DatabaseManager> dbm_{ nullptr };
	bool dbConnect{ false };

	// ****** Update this if wishing to change the percent difference between min max values and price
	double percentDiff = 0.1;

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

	// Tags that will be tracked and added to new candles
	Alerts::OptionType optType_{ Alerts::OptionType::Call };
	Alerts::TimeOfDay tod_{ Alerts::TimeOfDay::Hour1 };
	Alerts::DailyHighsAndLows DHL_{ Alerts::DailyHighsAndLows::Inside };
	Alerts::LocalHighsAndLows LHL_{ Alerts::LocalHighsAndLows::Inside };

	VolAndPriceTags VPT_;

	// Update various trackers
	// Update respective containers with new candles and stdev values
	void updateContainers(std::shared_ptr<Candle> c, TimeFrame tf);
	void updateCumulativeVolume(std::shared_ptr<Candle> c);
	void updateComparisons();
	void updateLocalMinMax(std::shared_ptr<Candle> c);

	// Update Tag Values
	void updateTimeOfDay(long unixTime);

	// For data keeping purposes
	vector<std::pair<long, long long>> cumulativeVolume_;

	std::mutex cdMtx;

	// We will also need to keep a connection open for the underlying price
	// Will cancel all alerts when an underlying security is being passed through
	bool isUnderlying_{ false };

//=========================================
// Callback Functionality for Alerts
//=========================================
public:
	using AlertFunction = std::function<void(std::shared_ptr<CandleTags> candle)>;
	void registerAlert(AlertFunction alert) { alert_ = std::move(alert); }

private:
	AlertFunction alert_;
};

Alerts::RelativeToMoney distFromPrice(Alerts::OptionType optType, int strike, double spxPrice);