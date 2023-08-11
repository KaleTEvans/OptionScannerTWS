#include "ContractData.h"

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

ContractData::ContractData(TickerId reqId, std::unique_ptr<Candle> initData) : contractId_(reqId) {
	// Push the first candle only in the 5 sec array
	std::shared_ptr<Candle> initCandle{ std::move(initData) };
	fiveSecCandles.push_back(initCandle);
	// Update the standard deviation class for the first 5 sec candle
	sd5Sec.addValue(initCandle->high() - initCandle->low());
	sdVol5Sec.addValue(initCandle->volume());

	// If reqId is 1234, mark as underlying
	if (initCandle->reqId() == 1234) isUnderlying_ = true;
}

// The input data function will be called each time a new candle is received, and will be where we 
// update each time series vector, stdev and mean. This will also be where we send out alerts
// if any anomalies are found
void ContractData::updateData(std::unique_ptr<Candle> c) {
	// Switch to shared pointer
	std::shared_ptr<Candle> fiveSec{ std::move(c) };
	// Always push to the 5 sec array
	fiveSecCandles.push_back(fiveSec);

	// Update stdev values for 5 sec array
	sd5Sec.addValue(fiveSec->high() - fiveSec->low());
	sdVol5Sec.addValue(fiveSec->volume());

	// Update daily high and low values to check relative price
	dailyHigh_ = max(dailyHigh_, fiveSec->high());
	dailyLow_ = min(dailyLow_, fiveSec->low());

	updateComparisons();

	//==============================================
	// 5 Second Timeframe Alert Options
	//==============================================
	if (sdVol5Sec.checkDeviation(fiveSec->volume(), 2) && sdVol5Sec.sum() > 9 && !isUnderlying_) {
		if (alert_) alert_(TimeFrame::FiveSecs, sd5Sec, sdVol5Sec, fiveSec);
	}

	// Using the length of the 5 sec array, we will determine if any new candles should be added to the other arrays
	// 6 increments for the 30 sec
	if (fiveSecCandles.size() % 6 == 0) {
		std::shared_ptr<Candle> thirtySec{ createNewBars(contractId_, 6, fiveSecCandles) };
		thirtySecCandles.push_back(thirtySec);

		// Update stdev values for 30 sec array
		sd30Sec.addValue(thirtySec->high() - thirtySec->low());
		sdVol30Sec.addValue(thirtySec->volume());

		//==============================================
		// 30 Second Timeframe Alert Options
		//==============================================
		if (sdVol30Sec.checkDeviation(thirtySec->volume(), 1.5) && sdVol30Sec.sum() > 9 && !isUnderlying_) {
			if (alert_) alert_(TimeFrame::ThirtySecs, sd30Sec, sdVol30Sec, thirtySec);
		}

		// Now we'll reference the 30 sec array for the 1min, so we only need to use increments of 2
		if (thirtySecCandles.size() > 0 && thirtySecCandles.size() % 2 == 0) {
			//std::cout << thirtySecCandles.size() << " " << oneMinCandles.size() << std::endl;
			std::shared_ptr<Candle> oneMin{ createNewBars(contractId_, 2, thirtySecCandles) };
			oneMinCandles.push_back(oneMin);

			// Update stdev values for 1 min array
			sd1Min.addValue(oneMin->high() - oneMin->low());
			sdVol1Min.addValue(oneMin->volume());

			///////////////// Update cumulative volume for historical records///////////////
			if (cumulativeVolume_.empty()) {
				std::pair<long, long> p = { oneMin->time(), oneMin->volume() };
				cumulativeVolume_.push_back(p);
			}
			else {
				int totalVol = oneMin->volume() + cumulativeVolume_.back().second;
				std::pair<long, long> p{ oneMin->time(), totalVol };
				cumulativeVolume_.push_back(p);
			}

			/*OPTIONSCANNER_DEBUG("{} Contract: {}, Cumulative Volume: {}", cumulativeVolume.back().first, contractId,
				cumulativeVolume.back().second);*/

			/*if (oneMinCandles.size() % 5 == 0) {
				OPTIONSCANNER_DEBUG("Local variables updatd for {} | Local High: {} | Local Low: {} ", contractId, localHigh, localLow);
				OPTIONSCANNER_DEBUG("Daily high: {} | Daily Low: {} | Current Price: {}", dailyHigh, dailyLow, getCurrentPrice());
			}*/

			//==============================================
			// 1 Minute Timeframe Alert Options
			//==============================================
			if (sdVol1Min.checkDeviation(oneMin->volume(), 1) && sdVol1Min.sum() > 9 && !isUnderlying_) {
				if (alert_) alert_(TimeFrame::OneMin, sd1Min, sdVol1Min, oneMin);
			}

			// Referencing the 1 min for the 5min array we can use increments of 5
			if (oneMinCandles.size() > 0 && oneMinCandles.size() % 5 == 0) {
				std::shared_ptr<Candle> fiveMin{ createNewBars(contractId_, 5, oneMinCandles) };
				fiveMinCandles.push_back(fiveMin);

				// Update stdev values for 5 minute array
				sd5Min.addValue(fiveMin->high() - fiveMin->low());
				sdVol5Min.addValue(fiveMin->volume());

				//==============================================
				// 5 Minute Timeframe Alert Options
				//==============================================
				if (sdVol5Min.checkDeviation(fiveMin->volume(), 1) && sdVol5Min.sum() > 4 && !isUnderlying_) {
					if (alert_) alert_(TimeFrame::FiveMin, sd5Sec, sdVol5Sec, fiveMin);
				}

				// Every 30 minutes, update the local high and low. Temp high and low will serve to track these values in between
				tempHigh_ = max(tempHigh_, fiveMin->high());
				tempLow_ = min(tempLow_, fiveMin->low());

				if (fiveMinCandles.size() % 6 == 0) {
					localHigh_ = tempHigh_;
					localLow_ = tempLow_;
					tempHigh_ = 0;
					tempLow_ = 10000;

					std::cout << "6 five min candles created" << std::endl;

					/*OPTIONSCANNER_DEBUG("Local variables updatd for {} | Local High: {} | Local Low: {} ", contractId, localHigh, localLow);
					OPTIONSCANNER_DEBUG("Daily high: {} | Daily Low: {} | Current Price: {}", dailyHigh, dailyLow, getCurrentPrice());*/
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
vector<std::shared_ptr<Candle>> ContractData::fiveSecData() const { return fiveSecCandles; }
vector<std::shared_ptr<Candle>> ContractData::thirtySecData() const { return thirtySecCandles; }
vector<std::shared_ptr<Candle>> ContractData::oneMinData() const { return oneMinCandles; }
vector<std::shared_ptr<Candle>> ContractData::fiveMinData() const { return fiveMinCandles; }

// Other data acessors
double ContractData::currentPrice() const { return fiveSecCandles.back()->close(); }
double ContractData::dailyHigh() const { return dailyHigh_; }
double ContractData::dailyLow() const { return dailyLow_; }
double ContractData::localHigh() const { return localHigh_; }
double ContractData::localLow() const { return localLow_; }
long ContractData::cumulativeVol() const { return cumulativeVolume_.back().second; }

pair<StandardDeviation, StandardDeviation> ContractData::get5SecStDev() const { return { sd5Sec, sdVol5Sec }; }
pair<StandardDeviation, StandardDeviation> ContractData::get30SecStDev() const { return { sd30Sec, sdVol30Sec }; }
pair<StandardDeviation, StandardDeviation> ContractData::get1MinStDev() const { return { sd1Min, sdVol1Min }; }
pair<StandardDeviation, StandardDeviation> ContractData::get5MinStDev() const { return { sd5Min, sdVol5Min }; }

vector<bool> ContractData::highLowComparisons() const { return { nearDailyLow, nearDailyHigh, nearLocalLow, nearLocalHigh }; }

//==============================================
// Helper Functions
//==============================================

void ContractData::updateComparisons() {
	// Update underlying information
	double lastPrice = fiveSecCandles.back()->close();
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
