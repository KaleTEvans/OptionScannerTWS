#include "ContractData.h"
//#include "Logger.h"

// Helper function to create new candles from time increments
std::shared_ptr<Candle> createNewBars(int id, int increment, const vector<std::shared_ptr<Candle>> data) {
	// Need to get total volume, high and low, and the open and close prices
	double open = data[data.size() - increment]->open();
	double close = data[data.size() - 1]->close();
	long time = data[data.size() - 1]->time();

	double high = 0;
	double low = INT_MAX;
	long volume = 0;
	
	for (size_t i = data.size() - increment; i <= data.size() - 1; i++) {
		high = max(high, data[i]->high());
		low = min(low, data[i]->low());
		volume += data[i]->volume();
	}

	std::shared_ptr<Candle> candle = std::make_shared<Candle>(id, time, open, high, low, close, volume);

	return candle;
}

ContractData::ContractData(TickerId reqId, std::unique_ptr<Candle> initData) : 
	contractId_{reqId}, dailyHigh_{ 0 }, dailyLow_{ 10000 }, localHigh_{ 0 }, localLow_{ 10000 }, tempHigh_{ 0 }, tempLow_{ 10000 },
	nearDailyHigh{ false }, nearDailyLow{ false }, nearLocalHigh{ false }, nearLocalLow{ false }
{
	// Push the first candle only in the 5 sec array
	std::shared_ptr<Candle> initCandle{ std::move(initData) };
	fiveSecCandles_.push_back(initCandle);

	// Update the standard deviation class for the first 5 sec candle
	sdPrice5Sec_.addValue(initCandle->high() - initCandle->low());
	sdVol5Sec_.addValue(initCandle->volume());

	// If reqId is 1234, mark as underlying
	if (initCandle->reqId() == 1234) isUnderlying_ = true;
}

// Initiate the SQL connection variable to add db insertion after each candle created
void ContractData::setupDatabaseManager(std::shared_ptr<OptionDB::DatabaseManager> dbm) {
	dbm_ = dbm;
	dbConnect = true;
}

// The input data function will be called each time a new candle is received, and will be where we 
// update each time series vector, stdev and mean. The chaining of if statements ensures that
// each vector has enough values to fill the next timeframe
void ContractData::updateData(std::unique_ptr<Candle> c) {
	//============================================================
	// 5 Second Candle Options
	// ===========================================================
	// Switch to shared pointer
	std::shared_ptr<Candle> fiveSec{ std::move(c) };
	updateContainers(fiveSec, TimeFrame::FiveSecs);

	// Post to db
	if (dbConnect) dbm_->addToInsertionQueue(fiveSec, TimeFrame::FiveSecs);

	// Update daily high and low values to check relative price
	dailyHigh_ = max(dailyHigh_, fiveSec->high());
	dailyLow_ = min(dailyLow_, fiveSec->low());

	updateComparisons();

	///////////////////////// 5 Second Alert Options ///////////////////////////////
	if (sdVol5Sec_.checkDeviation(fiveSec->volume(), 2) && sdVol5Sec_.sum() > 9 && !isUnderlying_) {
		if (alert_) alert_(TimeFrame::FiveSecs, fiveSec);
	}

	//=============================================================
	// 30 Second Candle Options
	//=============================================================

	if (fiveSecCandles_.size() % 6 == 0 && fiveSecCandles_.size() > 0) {
		std::shared_ptr<Candle> thirtySec{ createNewBars(contractId_, 6, fiveSecCandles_) };
		updateContainers(thirtySec, TimeFrame::ThirtySecs);

		// Post to db
		if (dbConnect) dbm_->addToInsertionQueue(thirtySec, TimeFrame::ThirtySecs);

		//OPTIONSCANNER_DEBUG("30 Second candle created for {}, Open: {}, Close: {}, volume{}",
		//	contractId_, thirtySecCandles_.back()->open(), thirtySecCandles_.back()->close(), thirtySecCandles_.back()->volume());

		///////////////////////// 30 Second Alert Options ///////////////////////////////
		if (sdVol30Sec_.checkDeviation(thirtySec->volume(), 1.5) && sdVol30Sec_.sum() > 9 && !isUnderlying_) {
			if (alert_) alert_(TimeFrame::ThirtySecs, thirtySec);
		}

		//=================================================================
		// 1 Minute Candle Options
		//=================================================================

		if (thirtySecCandles_.size() > 0 && thirtySecCandles_.size() % 2 == 0) {
			std::shared_ptr<Candle> oneMin{ createNewBars(contractId_, 2, thirtySecCandles_) };
			updateCumulativeVolume(oneMin);
			updateContainers(oneMin, TimeFrame::OneMin);

			// Post to db
			if (dbConnect) dbm_->addToInsertionQueue(oneMin, TimeFrame::OneMin);

			///////////////////////// 1 minute Alert Options ///////////////////////////////
			if (sdVol1Min_.checkDeviation(oneMin->volume(), 1) && sdVol1Min_.sum() > 9 && !isUnderlying_) {
				if (alert_) alert_(TimeFrame::OneMin, oneMin);
			}

			//====================================================================
			// 5 Minute Candle Options
			//====================================================================

			if (oneMinCandles_.size() > 0 && oneMinCandles_.size() % 5 == 0) {
				std::shared_ptr<Candle> fiveMin{ createNewBars(contractId_, 5, oneMinCandles_) };
				// Every 30 minutes, update the local high and low.
				updateLocalMinMax(fiveMin);
				updateContainers(fiveMin, TimeFrame::FiveMin);

				// Post to db
				if (dbConnect) dbm_->addToInsertionQueue(fiveMin, TimeFrame::FiveMin);

				///////////////////////// 5 Minute Alert Options ///////////////////////////////
				if (sdVol5Min_.checkDeviation(fiveMin->volume(), 1) && sdVol5Min_.sum() > 4 && !isUnderlying_) {
					if (alert_) alert_(TimeFrame::FiveMin, fiveMin);
				}
			}
		}
	}

}

//===============================================
// Access Functions
//===============================================

// Accessors
TickerId ContractData::contractId() const { return contractId_; }

// Time series accessors
vector<std::shared_ptr<Candle>> ContractData::fiveSecData() const { return fiveSecCandles_; }
vector<std::shared_ptr<Candle>> ContractData::thirtySecData() const { return thirtySecCandles_; }
vector<std::shared_ptr<Candle>> ContractData::oneMinData() const { return oneMinCandles_; }
vector<std::shared_ptr<Candle>> ContractData::fiveMinData() const { return fiveMinCandles_; }

// Other data acessors
double ContractData::currentPrice() const { return fiveSecCandles_.back()->close(); }
double ContractData::dailyHigh() const { return dailyHigh_; }
double ContractData::dailyLow() const { return dailyLow_; }
double ContractData::localHigh() const { return localHigh_; }
double ContractData::localLow() const { return localLow_; }
long long ContractData::totalVol() const { return cumulativeVolume_.back().second; }

vector<std::pair<long, long long>> ContractData::volOverTime() const { return cumulativeVolume_; }
vector<bool> ContractData::highLowComparisons() const { return { nearDailyLow, nearDailyHigh, nearLocalLow, nearLocalHigh }; }

std::shared_ptr<Candle> ContractData::latestCandle(TimeFrame tf) {
	switch (tf)
	{
	case TimeFrame::FiveSecs:
		return fiveSecCandles_.back();
	case TimeFrame::ThirtySecs:
		return thirtySecCandles_.back();
	case TimeFrame::OneMin:
		return oneMinCandles_.back();
	case TimeFrame::FiveMin:
		return fiveMinCandles_.back();
	default:
		//OPTIONSCANNER_ERROR("Failed to return most recent candle");
		return {};
	}
}

std::vector<std::shared_ptr<Candle>> ContractData::candlesLast30Minutes() {
	vector<std::shared_ptr<Candle>> res;

	if (fiveSecCandles_.size() >= 360) {
		for (size_t i = fiveSecCandles_.size() - 360; i < fiveSecCandles_.size(); i++) res.push_back(fiveSecCandles_[i]);
	}
	else {
		res = {};
		//OPTIONSCANNER_ERROR("Not enough five sec candles to return");
	}

	return res;
}

StandardDeviation ContractData::priceStDev(TimeFrame tf) {
	switch (tf)
	{
	case TimeFrame::FiveSecs:
		return sdPrice5Sec_;
	case TimeFrame::ThirtySecs:
		return sdPrice30Sec_;
	case TimeFrame::OneMin:
		return sdPrice1Min_;
	case TimeFrame::FiveMin:
		return sdPrice5Min_;
	default:
		//OPTIONSCANNER_ERROR("Failed to return Price Standard Deviation Object");
		return {};
	}
}

StandardDeviation ContractData::volStDev(TimeFrame tf) {
	switch (tf)
	{
	case TimeFrame::FiveSecs:
		return sdVol5Sec_;
	case TimeFrame::ThirtySecs:
		return sdVol30Sec_;
	case TimeFrame::OneMin:
		return sdVol1Min_;
	case TimeFrame::FiveMin:
		return sdVol5Min_;
	default:
		//OPTIONSCANNER_ERROR("Failed to return Volume Standard Deviation Object");
		return {};
	}
}

//==============================================
// Helper Functions
//==============================================

void ContractData::updateContainers(std::shared_ptr<Candle> c, TimeFrame tf) {
	switch (tf)
	{
	case TimeFrame::FiveSecs:
		fiveSecCandles_.push_back(c);
		sdPrice5Sec_.addValue(c->high() - c->low());
		sdVol5Sec_.addValue(c->volume());
		break;
	case TimeFrame::ThirtySecs:
		thirtySecCandles_.push_back(c);
		sdPrice30Sec_.addValue(c->high() - c->low());
		sdVol30Sec_.addValue(c->volume());
		break;
	case TimeFrame::OneMin:
		oneMinCandles_.push_back(c);
		sdPrice1Min_.addValue(c->high() - c->low());
		sdVol1Min_.addValue(c->volume());
		break;
	case TimeFrame::FiveMin:
		fiveMinCandles_.push_back(c);
		sdPrice5Min_.addValue(c->high() - c->low());
		sdVol5Min_.addValue(c->volume());
		break;
	}
}

void ContractData::updateCumulativeVolume(std::shared_ptr<Candle> c) {
	long long vol = static_cast<long long>(c->volume());
	long time = c->time();

	if (cumulativeVolume_.empty()) {
		std::pair<long, long long> p = { time, vol };
		cumulativeVolume_.push_back(p);
	}
	else {
		long long totalVol = vol + cumulativeVolume_.back().second;
		std::pair<long, long long> p{ time, totalVol };
		cumulativeVolume_.push_back(p);
	}
}

void ContractData::updateComparisons() {
	// Update underlying information
	double lastPrice = fiveSecCandles_.back()->close();
	double percentDiff = 0.1;

	// Check values against the underlying price, will use 0.1% difference
	if (isWithinXPercent(lastPrice, dailyHigh_, percentDiff)) nearDailyHigh = true;
	else nearDailyHigh = false;

	if (isWithinXPercent(lastPrice, dailyLow_, percentDiff)) nearDailyLow = true;
	else nearDailyLow = false;

	if (isWithinXPercent(lastPrice, localHigh_, percentDiff) || lastPrice > localHigh_) nearLocalHigh = true;
	else nearLocalHigh = false;

	if (isWithinXPercent(lastPrice, localLow_, percentDiff) || lastPrice < localLow_) nearLocalLow = true;
	else nearLocalLow = false;
}

void ContractData::updateLocalMinMax(std::shared_ptr<Candle> c) {
	// Every 30 minutes, update the local high and low. Temp high and low will serve to track these values in between
	tempHigh_ = max(tempHigh_, c->high());
	tempLow_ = min(tempLow_, c->low());

	if (fiveMinCandles_.size() % 6 == 0 && fiveMinCandles_.size() > 0) {
		localHigh_ = tempHigh_;
		localLow_ = tempLow_;
		tempHigh_ = 0;
		tempLow_ = 10000;
	}
}
