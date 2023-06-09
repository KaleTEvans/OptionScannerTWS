#include "ContractData.h"

// Helper function to create new candles from time increments
Candle createNewBars(int id, int increment, const vector<Candle>& data) {
	// Need to get total volume, high and low, and the open and close prices
	double open = data[data.size() - increment].open;
	double close = data[data.size() - 1].close;
	long time = data[data.size() - 1].time;

	double high = 0;
	double low = INT_MAX;
	long volume = 0;
	
	for (size_t i = data.size() - increment; i <= data.size() - 1; i++) {
		high = max(high, data[i].high);
		low = min(low, data[i].low);
		volume += data[i].volume;
	}

	Candle c1(id, time, open, high, low, close, volume);

	return c1;
}

ContractData::ContractData(TickerId reqId, Candle initData, bool isUnderlying) : contractId(reqId) {
	// Push the first candle only in the 5 sec array
	fiveSecCandles.push_back(initData);
	// Update the standard deviation class for the first 5 sec candle
	sd5Sec.addValue(initData.high - initData.low);
	sdVol5Sec.addValue(initData.volume);
}

// The input data function will be called each time a new candle is received, and will be where we 
// update each time series vector, stdev and mean. This will also be where we send out alerts
// if any anomalies are found
void ContractData::updateData(Candle c) {
	// Always push to the 5 sec array
	fiveSecCandles.push_back(c);

	// Update stdev values for 5 sec array
	sd5Sec.addValue(c.high - c.low);
	sdVol5Sec.addValue(c.volume);

	//==============================================
	// 5 Second Timeframe Alert Options
	//==============================================
	if (sdVol5Sec.checkDeviation(c.volume, 2) && sdVol5Sec.getTotal() > 9 && !isUnderlying) {
		if (alert_) alert_(1001, sd5Sec, sdVol5Sec, c);
	}

	// Using the length of the 5 sec array, we will determine if any new candles should be added to the other arrays
	// 6 increments for the 30 sec
	if (fiveSecCandles.size() % 6 == 0) {
		Candle c6 = createNewBars(contractId, 6, fiveSecCandles);
		thirtySecCandles.push_back(c6);

		// Update stdev values for 30 sec array
		sd30Sec.addValue(c6.high - c6.low);
		sdVol30Sec.addValue(c6.volume);

		//==============================================
		// 30 Second Timeframe Alert Options
		//==============================================
		if (sdVol30Sec.checkDeviation(c6.volume, 1.5) && sdVol30Sec.getTotal() > 9 && !isUnderlying) {
			if (alert_) alert_(1002, sd30Sec, sdVol30Sec, c6);
		}

		// Now we'll reference the 30 sec array for the 1min, so we only need to use increments of 2
		if (thirtySecCandles.size() > 0 && thirtySecCandles.size() % 2 == 0) {
			//std::cout << thirtySecCandles.size() << " " << oneMinCandles.size() << std::endl;
			Candle c1 = createNewBars(contractId, 2, thirtySecCandles);
			oneMinCandles.push_back(c1);

			// Update stdev values for 1 min array
			sd1Min.addValue(c1.high - c1.low);
			sdVol1Min.addValue(c1.volume);

			if (cumulativeVolume.empty()) {
				std::pair<long, long> p = { c1.time, c1.volume };
				cumulativeVolume.push_back(p);
			}
			else {
				int totalVol = c1.volume + cumulativeVolume.back().second;
				std::pair<long, long> p = { c1.time, totalVol };
				cumulativeVolume.push_back(p);
			}

			//==============================================
			// 1 Minute Timeframe Alert Options
			//==============================================
			if (sdVol1Min.checkDeviation(c1.volume, 1) && sdVol1Min.getTotal() > 9 && !isUnderlying) {
				if (alert_) alert_(1003, sd1Min, sdVol1Min, c1);
			}

			// Update cumulative volume for historical records
			/*if (cumulativeVolume.empty()) {
				std::pair<long, long> p = { c1.time, c1.volume };
				cumulativeVolume.push_back(p);
			}
			else {
				int totalVol = c1.volume + cumulativeVolume.back().second;
				std::pair<long, long> p = { c1.time, totalVol };
				cumulativeVolume.push_back(p);
			}*/

			// Update daily high and low values to check relative price
			dailyHigh = max(dailyHigh, c1.high);
			dailyLow = min(dailyLow, c1.low);

			updateUnderlyingComparisons();

			// Referencing the 1 min for the 5min array we can use increments of 5
			if (oneMinCandles.size() > 0 && oneMinCandles.size() % 5 == 0) {
				Candle c5 = createNewBars(contractId, 5, oneMinCandles);
				fiveMinCandles.push_back(c5);

				// Update stdev values for 5 minute array
				sd5Min.addValue(c5.high - c5.low);
				sdVol5Min.addValue(c5.volume);

				//==============================================
				// 5 Minute Timeframe Alert Options
				//==============================================
				if (sdVol5Min.checkDeviation(c5.volume, 1) && sdVol5Min.getTotal() > 4 && !isUnderlying) {
					if (alert_) alert_(1004, sd5Sec, sdVol5Sec, c5);
				}

				// Every 30 minutes, update the local high and low. Temp high and low will serve to track these values in between
				tempHigh = max(tempHigh, c5.high);
				tempLow = max(tempLow, c5.low);

				if (fiveMinCandles.size() % 6 == 0) {
					localHigh = tempHigh;
					localLow = tempLow;
					tempHigh = 0;
					tempLow = 10000;

					std::cout << "6 five min candles created" << std::endl;

					OPTIONSCANNER_DEBUG("Local variables updatd for {} | Local High: {} | Local Low: {} ", contractId, localHigh, localLow);
					OPTIONSCANNER_DEBUG("Daily high: {} | Daily Low: {} | Current Price: {}", dailyHigh, dailyLow, getCurrentPrice());
				}
			}
		}
	}

}

//==============================================
// Helper Functions
//==============================================

void ContractData::updateUnderlyingComparisons() {
	// Update underlying information
	double lastPrice = fiveSecCandles.back().close;
	double percentDiff = 0.1;

	// Check values against the underlying price, will use 0.1% difference
	if (isWithinXPercent(lastPrice, dailyHigh, percentDiff)) nearDailyHigh = true;
	else nearDailyHigh = false;

	if (isWithinXPercent(lastPrice, dailyLow, percentDiff)) nearDailyLow = true;
	else nearDailyLow = false;

	if (isWithinXPercent(lastPrice, localHigh, percentDiff)) nearLocalHigh = true;
	else nearLocalHigh = false;

	if (isWithinXPercent(lastPrice, localLow, percentDiff)) nearLocalLow = true;
	else nearLocalLow = false;
}
